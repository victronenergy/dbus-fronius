#include <cassert>
#include <QDBusConnection>
#include <QDebug>
#include <velib/qt/v_busitem.h>
#include <velib/vecan/products.h>
#include "dbus_inverter_guard.h"
#include "inverter.h"
#include "power_info.h"
#include "version.h"

DBusInverterGuard::DBusInverterGuard(Inverter *inverter) :
	QObject(inverter),
	mInverter(inverter)
{
	assert(inverter != 0);

	QString serviceName = QString("com.victronenergy.pvinverter.fronius_%1_%2").
			arg(fixServiceNameFragment(inverter->hostName())).
			arg(fixServiceNameFragment(inverter->id()));

	QDBusConnection connection = QDBusConnection::connectToBus(
			QDBusConnection::SessionBus, serviceName);

	PowerInfo *mpi = inverter->meanPowerInfo();

	addBusItem(connection, inverter, "/Connected", "isConnected", "");
	addBusItem(connection, mpi, "/Ac/Current", "current", "A");
	addBusItem(connection, mpi, "/Ac/Voltage", "voltage", "V");
	addBusItem(connection, mpi, "/Ac/Power", "power", "W");
	if (inverter->supports3Phases()) {
		addBusItems(connection, inverter->l1PowerInfo(), "/Ac/L1");
		addBusItems(connection, inverter->l2PowerInfo(), "/Ac/L2");
		addBusItems(connection, inverter->l3PowerInfo(), "/Ac/L3");
	}

	QString connectionString = QString("%1 %2").
			arg(tr("Fronius Inverter")).
			arg(inverter->uniqueId());
	QString processName = QCoreApplication::arguments()[0];
	// The values of the items below will not change after creation, so we don't
	// need an update mechanism.
	addConstBusItem(connection, "/Mgmt/ProcessName", processName);
	addConstBusItem(connection, "/Mgmt/ProcessVersion", VERSION);
	addConstBusItem(connection, "/Mgmt/Connection", connectionString);
	addConstBusItem(connection, "/Position", inverter->id());
	addConstBusItem(connection, "/ProductId", VE_PROD_ID_PV_INVERTER_FRONIUS);
	addConstBusItem(connection, "/ProductName", tr("Fronius Inverter"));

	qDebug() << __FUNCTION__ << serviceName;
	if (!connection.registerService(serviceName)) {
		qDebug() << "RegisterService failed";
	}

	connect(inverter, SIGNAL(propertyChanged(QString)),
			this, SLOT(onPropertyChanged(QString)));
	connect(mpi, SIGNAL(propertyChanged(QString)),
			this, SLOT(onPropertyChanged(QString)));
}

void DBusInverterGuard::onPropertyChanged(const QString &property)
{
	foreach (BusItemBridge bib, mBusItems) {
		if (bib.src == sender() && bib.property == property) {
			QVariant value = bib.src->property(property.toAscii().data());
			qDebug()
					<< static_cast<Inverter *>(bib.src->parent())->id()
					<< property << ':' << value;
			bib.item->setValue(value);
			break;
		}
	}
}

void DBusInverterGuard::onVBusItemChanged()
{
	foreach (BusItemBridge bib, mBusItems) {
		if (bib.item == sender())
		{
			updateNodes(bib.path);
			break;
		}
	}
}

void DBusInverterGuard::addBusItems(QDBusConnection &connection, PowerInfo *pi,
									const QString &path)
{
	addBusItem(connection, pi, path + "/Current", "current", "A");
	addBusItem(connection, pi, path + "/Voltage", "voltage", "V");
	connect(pi, SIGNAL(propertyChanged(QString)),
			this, SLOT(onPropertyChanged(QString)));
}

void DBusInverterGuard::addBusItem(QDBusConnection &connection, QObject *src,
								   const QString &path, const QString &property,
								   const QString &unit)
{
	VBusItem *vbi = new VBusItem(this);
	QVariant value = src->property(property.toAscii().data());
	vbi->produce(connection, path, "???", value, unit);
	BusItemBridge bib;
	bib.item = vbi;
	bib.src = src;
	bib.property = property;
	bib.path = path;
	mBusItems.push_back(bib);
	connect(vbi, SIGNAL(valueChanged()), this, SLOT(onVBusItemChanged()));
	addNodes(connection, path);
}

void DBusInverterGuard::addConstBusItem(QDBusConnection &connection,
								   const QString &path, const QVariant &value)
{
	VBusItem *vbi = new VBusItem(this);
	vbi->produce(connection, path, "", value, "");
	BusItemBridge bib;
	bib.item = vbi;
	bib.src = 0;
	bib.path = path;
	mBusItems.push_back(bib);
	addNodes(connection, path);
}

void DBusInverterGuard::addNodes(QDBusConnection &connection,
								 const QString &path)
{
	QString p = path;
	for (;;) {
		p = getParentPath(p);
		if (p.isEmpty())
			break;
		QMap<QString, VBusItem *>::iterator it = mNodes.find(p);
		if (it == mNodes.end()) {
			VBusItem *node = new VBusItem(this);
			node->produce(connection, p, "", createNodeMap(p));
			mNodes[p] = node;
		} else {
			it.value()->setValue(createNodeMap(p));
		}
	}
}

void DBusInverterGuard::updateNodes(const QString &path)
{
	QString p = path;
	for (;;) {
		p = getParentPath(p);
		if (p.isEmpty())
			break;
		QMap<QString, VBusItem *>::iterator it = mNodes.find(p);
		if (it != mNodes.end()) {
			it.value()->setValue(createNodeMap(p));
		}
	}
}

QVariantMap DBusInverterGuard::createNodeMap(const QString &path) const
{
	QVariantMap result;
	foreach (BusItemBridge bib, mBusItems) {
		if (isChildPath(path, bib.path)) {
			result[bib.path.mid(1)] = bib.item->getValue();
		}
	}
	return result;
}

QString DBusInverterGuard::fixServiceNameFragment(const QString &s)
{
	return ((QString)s).remove('.').remove('_');
}

QString DBusInverterGuard::getParentPath(const QString &s)
{
	int i = s.lastIndexOf('/');
	if (i == -1 || s.size() == 1)
		return QString();
	if (i == 0)
		++i;
	return s.left(i);
}

bool DBusInverterGuard::isChildPath(const QString &p0, const QString &p1)
{
	return p0 == "/" || (
		p1.size() > p0.size() &&
		p1.startsWith(p0) &&
		p1[p0.size()] == '/');
}

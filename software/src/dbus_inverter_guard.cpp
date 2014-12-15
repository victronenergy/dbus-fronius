#include <cassert>
#include <QDBusConnection>
#include <QDebug>
#include <velib/qt/v_busitem.h>
#include <velib/vecan/products.h>
#include "dbus_inverter_guard.h"
#include "inverter.h"
#include "power_info.h"

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
	addBusItem(connection, inverter, "/Mgmt/ProcessName", "processName", "");
	addBusItem(connection, inverter, "/Mgmt/ProcessVersion", "processVersion", "");
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

	// The values of items below will not change after creation, so we don't
	// need an update mechanism as created in the addBusItem function.
	new VBusItem(connection, "/Mgmt/Connection", "", connectionString, "", this);
	new VBusItem(connection, "/Position", "", inverter->id(), "", this);
	new VBusItem(connection, "/ProductId", "", VE_PROD_ID_PV_INVERTER_FRONIUS, "", this);
	new VBusItem(connection, "/ProductName", "", tr("Fronius Inverter"), "", this);

	qDebug() << __FUNCTION__ << serviceName;
	if (!connection.registerService(serviceName)) {
		qDebug() << "RegisterService failed";
	}

	connect(inverter, SIGNAL(propertyChanged(QString)), this, SLOT(onPropertyChanged(QString)));
	connect(mpi, SIGNAL(propertyChanged(QString)), this, SLOT(onPropertyChanged(QString)));
}

DBusInverterGuard::~DBusInverterGuard()
{
	foreach (BusItemBridge b, mBusItems) {
		// Delete the VBusItems, because their service is no longer desired.
		delete b.item;
		// Do not delete b.src, because we didn't create it, and it may be
		// present more than once in mBusItems.
	}
}

void DBusInverterGuard::onPropertyChanged(const QString &property)
{
	for (QList<BusItemBridge>::iterator it = mBusItems.begin();
		 it != mBusItems.end();
		 ++it) {
		if (it->src == sender() && it->property == property) {
			QVariant value = it->src->property(property.toAscii().data());
			qDebug()
					<< static_cast<Inverter *>(it->src->parent())->id()
					<< property << ':' << value;
			it->item->setValue(value);
			break;
		}
	}
}

void DBusInverterGuard::addBusItems(QDBusConnection &connection, PowerInfo *pi,
									QString path)
{
	addBusItem(connection, pi, path + "/Current", "current", "A");
	addBusItem(connection, pi, path + "/Voltage", "voltage", "V");
	connect(pi, SIGNAL(propertyChanged(QString)),
			this, SLOT(onPropertyChanged(QString)));
}

void DBusInverterGuard::addBusItem(QDBusConnection &connection, QObject *src,
								   QString path, QString property, QString unit)
{
	VBusItem *vbi = new VBusItem(src);
	QVariant value = src->property(property.toAscii().data());
	vbi->produce(connection, path, "???", value, unit);
	BusItemBridge bib;
	bib.item = vbi;
	bib.src = src;
	bib.property = property;
	mBusItems.push_back(bib);
}

QString DBusInverterGuard::fixServiceNameFragment(const QString &s)
{
	return ((QString)s).remove('.').remove('_');
}

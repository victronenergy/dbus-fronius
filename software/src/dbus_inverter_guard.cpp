#include <QCoreApplication>
#include <QDBusConnection>
#include <QsLog.h>
#include <QStringList>
#include <velib/vecan/products.h>
#include "dbus_inverter_guard.h"
#include "inverter.h"
#include "power_info.h"
#include "version.h"

DBusInverterGuard::DBusInverterGuard(Inverter *inverter) :
	DBusGuard(inverter),
	mInverter(inverter)
{
	Q_ASSERT(inverter != 0);

	QString serviceName = QString("com.victronenergy.pvinverter.fronius_%1_%2").
			arg(fixServiceNameFragment(inverter->hostName())).
			arg(fixServiceNameFragment(inverter->id()));

	QDBusConnection connection = QDBusConnection::connectToBus(
			QDBusConnection::SessionBus, serviceName);

	PowerInfo *mpi = inverter->meanPowerInfo();

	// mServiceRoot = new VBusNode(connection, "/", this);

	produce(connection, inverter, "isConnected",	"/Connected");
	produce(connection, mpi, "current", "/Ac/Current", "A");
	produce(connection, mpi, "voltage", "/Ac/Voltage", "V");
	produce(connection, mpi, "power", "/Ac/Power", "W");
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
	produce(connection, "/Mgmt/ProcessName", processName);
	produce(connection, "/Mgmt/ProcessVersion", VERSION);
	produce(connection, "/Mgmt/Connection", connectionString);
	produce(connection, "/Position", inverter->id());
	produce(connection, "/ProductId", VE_PROD_ID_PV_INVERTER_FRONIUS);
	produce(connection, "/ProductName", tr("Fronius Inverter"));
	produce(connection, "/Serial", inverter->uniqueId());

	QLOG_INFO() << "Registering service" << serviceName;
	if (!connection.registerService(serviceName)) {
		QLOG_FATAL() << "RegisterService failed";
	}
}

//void DBusInverterGuard::onPropertyChanged()
//{
//	QObject *src = sender();
//	int signalIndex = senderSignalIndex();
//	const QMetaObject *mo = src->metaObject();
//	for (int i=0; i<mo->propertyCount(); ++i) {
//		QMetaProperty mp = mo->property(i);
//		if (mp.hasNotifySignal() && mp.notifySignalIndex() == signalIndex) {
//			foreach (BusItemBridge bib, mBusItems) {
//				if (bib.src == src && strcmp(bib.property.name(), mp.name()) == 0) {
//					QVariant value = src->property(mp.name());
//					bib.item->setValue(value);
//					QLOG_TRACE() << __FUNCTION__ << bib.property.name();
//					break;
//				}
//			}
//		}
//	}
//}

//void DBusInverterGuard::onVBusItemChanged()
//{
//	foreach (BusItemBridge bib, mBusItems) {
//		if (bib.item == sender()) {
//			QVariant value = bib.item->getValue();
//			bib.src->setProperty(bib.property.name(), value);
//			break;
//		}
//	}
//}

void DBusInverterGuard::addBusItems(QDBusConnection &connection, PowerInfo *pi,
									const QString &path)
{
	produce(connection, pi, "current", path + "/Current", "A");
	produce(connection, pi, "voltage", path + "/Voltage", "V");
}

//void DBusInverterGuard::addBusItem(QDBusConnection &connection, QObject *src,
//								   const QString &path, const char *property,
//								   const QString &unit)
//{
//	VBusItem *vbi = new VBusItem(this);
//	QVariant value = src->property(property);
//	vbi->produce(connection, path, "?", value, unit);
//	BusItemBridge bib;
//	bib.item = vbi;
//	bib.src = src;
//	bib.path = path;
//	const QMetaObject *mo = src->metaObject();
//	int i = mo->indexOfProperty(property);
//	if (i >= 0) {
//		QMetaProperty mp = mo->property(i);
//		if (mp.hasNotifySignal()) {
//			QMetaMethod signal = mp.notifySignal();
//			QMetaMethod slot = metaObject()->method(
//						metaObject()->indexOfSlot("onPropertyChanged()"));
//			connect(src, signal, this, slot);
//		}
//		bib.property = mp;
//	}
//	mBusItems.push_back(bib);
//	connect(vbi, SIGNAL(valueChanged()), this, SLOT(onVBusItemChanged()));
//	mServiceRoot->addChild(path, vbi);
//}

//void DBusInverterGuard::addConstBusItem(QDBusConnection &connection,
//										const QString &path,
//										const QVariant &value)
//{
//	VBusItem *vbi = new VBusItem(this);
//	vbi->produce(connection, path, "", value, "");
//	BusItemBridge bib;
//	bib.item = vbi;
//	bib.src = 0;
//	bib.path = path;
//	mBusItems.push_back(bib);
//	mServiceRoot->addChild(path, vbi);
//}

QString DBusInverterGuard::fixServiceNameFragment(const QString &s)
{
	return ((QString)s).remove('.').remove('_');
}

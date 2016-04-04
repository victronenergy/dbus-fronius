#include <QDBusVariant>
#include <QsLog.h>
#include <QTimer>
#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>
#include "v_bus_node.h"
#include "dbus_bridge.h"

Q_DECLARE_METATYPE(QList<int>)

DBusBridge::DBusBridge(QObject *parent) :
	QObject(parent),
	mServiceRegistered(false),
	mUpdateBusy(false),
	mUpdateTimer(0)
{
}

DBusBridge::DBusBridge(const QString &serviceName, QObject *parent):
	QObject(parent),
	mServiceName(serviceName),
	mServiceRegistered(false),
	mUpdateBusy(false),
	mUpdateTimer(0)
{
}

DBusBridge::~DBusBridge()
{
	if (!mServiceRegistered)
		return;
	QLOG_INFO() << "Unregistering service" << mServiceName;
	QDBusConnection connection = VBusItems::getConnection(mServiceName);
	if (!connection.unregisterService(mServiceName)) {
		QLOG_FATAL() << "UnregisterService failed";
	}
}

void DBusBridge::setUpdateInterval(int interval)
{
	if (interval <= 0) {
		if (mUpdateTimer != 0) {
			delete mUpdateTimer;
			mUpdateTimer = 0;
		}
		return;
	}
	if (mUpdateTimer == 0) {
		mUpdateTimer = new QTimer(this);
		connect(mUpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimer()));
	}
	mUpdateTimer->setInterval(interval);
	mUpdateTimer->start();
}

void DBusBridge::produce(QObject *src, const char *property,
						 const QString &path, const QString &unit,
						 int precision)
{
	VBusItem *vbi = new VBusItem(this);
	QVariant value = src->property(property);
	toDBus(path, value);
	if (!value.isValid())
		value = QVariant::fromValue(QList<int>());
	connectItem(vbi, src, property, path);
	QDBusConnection connection = VBusItems::getConnection(mServiceName);
	vbi->produce(connection, path, "?", value, unit, precision);
	addVBusNodes(path, vbi);
}

void DBusBridge::produce(const QString &path, const QVariant &value,
						 const QString &unit, int precision)
{
	VBusItem *vbi = new VBusItem(this);
	connectItem(vbi, 0, 0, path);
	QDBusConnection connection = VBusItems::getConnection(mServiceName);
	vbi->produce(connection, path, "", value, unit, precision);
	addVBusNodes(path, vbi);
}

void DBusBridge::consume(const QString &service, QObject *src,
						 const char *property, const QString &path)
{
	QDBusConnection &connection = VBusItems::getConnection();
	VBusItem *vbi = new VBusItem(this);
	connectItem(vbi, src, property, path);
	vbi->consume(connection, service, path);
	vbi->getValue(); // force value retrieval
}

void DBusBridge::consume(const QString &service, QObject *src,
						 const char *property, const QVariant &defaultValue,
						 const QString &path)
{
	addSetting(path, defaultValue, QVariant(0), QVariant(0));
	consume(service, src, property, path);
}

void DBusBridge::consume(const QString &service, QObject *src,
						 const char *property, double defaultValue,
						 double minValue, double maxValue, const QString &path)
{
	addSetting(path, QVariant(defaultValue), QVariant(minValue), QVariant(maxValue));
	consume(service, src, property, path);
}

QString DBusBridge::serviceName() const
{
	return mServiceName;
}

void DBusBridge::setServiceName(const QString &sn)
{
	mServiceName = sn;
}

void DBusBridge::registerService()
{
	if (mServiceRegistered) {
		QLOG_ERROR() << "Service already registered";
		return;
	}
	QLOG_INFO() << "Registering service" << mServiceName;
	QDBusConnection connection = VBusItems::getConnection(mServiceName);
	if (connection.registerService(mServiceName))
		mServiceRegistered = true;
	else
		QLOG_FATAL() << "RegisterService failed";
}

bool DBusBridge::toDBus(const QString &, QVariant &)
{
	return true;
}

bool DBusBridge::fromDBus(const QString &, QVariant &)
{
	return true;
}

bool DBusBridge::addSetting(const QString &path,
							const QVariant &defaultValue,
							const QVariant &minValue,
							const QVariant &maxValue)
{
	if (!path.startsWith("/Settings"))
		return false;
	int groupStart = path.indexOf('/', 1);
	if (groupStart == -1)
		return false;
	int nameStart = path.lastIndexOf('/');
	if (nameStart <= groupStart)
		return false;
	QChar type;
	switch (defaultValue.type()) {
	case QVariant::Int:
		type = 'i';
		break;
	case QVariant::Double:
		type = 'f';
		break;
	case QVariant::String:
		type = 's';
		break;
	default:
		return false;
	}
	QString group = path.mid(groupStart + 1, nameStart - groupStart - 1);
	QString name = path.mid(nameStart + 1);
	QDBusConnection &connection = VBusItems::getConnection();
	QDBusMessage m = QDBusMessage::createMethodCall(
						 "com.victronenergy.settings",
						 "/Settings",
						 "com.victronenergy.Settings",
						 "AddSetting")
					 << group
					 << name
					 << QVariant::fromValue(QDBusVariant(defaultValue))
					 << QString(type)
					 << QVariant::fromValue(QDBusVariant(minValue))
					 << QVariant::fromValue(QDBusVariant(maxValue));
	QDBusMessage reply = connection.call(m);
	return reply.type() == QDBusMessage::ReplyMessage;
}

void DBusBridge::onPropertyChanged()
{
	QObject *src = sender();
	int signalIndex = senderSignalIndex();
	for (QList<BusItemBridge>::iterator it = mBusItems.begin(); it != mBusItems.end(); ++it) {
		if (it->src == src && it->property.isValid() &&
				it->property.notifySignalIndex() == signalIndex) {
			if (mUpdateTimer == 0)
				publishValue(*it);
			else
				it->changed = true;
			break;
		}
	}
}

void DBusBridge::onVBusItemChanged()
{
	if (mUpdateBusy)
		return;
	bool checkInit = false;
	for (QList<BusItemBridge>::iterator it = mBusItems.begin();
		 it != mBusItems.end();
		 ++it) {
		if (it->item == sender()) {
			if (it->src == 0) {
				QLOG_WARN() << "Value changed on D-Bus could not be stored in QT-property";
			} else if (it->property.isValid()) {
				QVariant value = it->item->getValue();
				if (value.canConvert<QList<int> >()) {
					QList<int> l = value.value<QList<int> >();
					if (l.isEmpty())
						value = QVariant();
				}
				if (fromDBus(it->path, value))
					it->src->setProperty(it->property.name(), value);
			}
			if (!it->initialized) {
				it->initialized = true;
				checkInit = true;
			}
			break;
		}
	}
	if (checkInit) {
		foreach (BusItemBridge bib, mBusItems) {
			if (!bib.initialized) {
				checkInit = false;
				break;
			}
		}
		if (checkInit)
			emit initialized();
	}
}

void DBusBridge::onUpdateTimer()
{
	for (QList<BusItemBridge>::iterator it = mBusItems.begin(); it != mBusItems.end(); ++it) {
		if (it->changed) {
			publishValue(*it);
			it->changed = false;
		}
	}
}

void DBusBridge::connectItem(VBusItem *busItem, QObject *src,
							 const char *property, const QString &path)
{
	BusItemBridge bib;
	bib.item = busItem;
	bib.src = src;
	bib.path = path;
	bib.initialized = false;
	bib.changed = false;
	if (src == 0) {
		if (property != 0) {
			QLOG_ERROR() << "Property specified (" << property
						 << "), but source omitted in DBusBridge. Path was"
						 << path;
		}
	} else {
		if (property == 0) {
			QLOG_ERROR() << "Source specified, but property omitted in"
						 << "DBusBridge. Path was"
						 << path;
		} else {
			const QMetaObject *mo = src->metaObject();
			int i = mo->indexOfProperty(property);
			if (i == -1) {
				QLOG_ERROR() << "DBusBridge could not find property" << property
							 << "Path was" << path;
			} else {
				QMetaProperty mp = mo->property(i);
				if (mp.hasNotifySignal()) {
					QMetaMethod signal = mp.notifySignal();
					int index = metaObject()->indexOfSlot("onPropertyChanged()");
					QMetaMethod slot = metaObject()->method(index);
					connect(src, signal, this, slot);
				}
				bib.property = mp;
			}
		}
	}
	mBusItems.push_back(bib);
	connect(busItem, SIGNAL(valueChanged()), this, SLOT(onVBusItemChanged()));
}

void DBusBridge::addVBusNodes(const QString &path, VBusItem *vbi)
{
	if (mServiceRoot.isNull()) {
		QDBusConnection connection = VBusItems::getConnection(serviceName());
		mServiceRoot = new VBusNode(connection, "/", this);
	}
	mServiceRoot->addChild(path, vbi);
}

void DBusBridge::publishValue(DBusBridge::BusItemBridge &item)
{
	QVariant value = item.src->property(item.property.name());
	if (!toDBus(item.path, value))
		return;
	if (!value.isValid())
		value = QVariant::fromValue(QList<int>());
	mUpdateBusy = true;
	item.item->setValue(value);
	mUpdateBusy = false;
}

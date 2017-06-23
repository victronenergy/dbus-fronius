#ifdef QT_DBUS_LIB
#include <QDBusMessage>
#include <QDBusVariant>
#include <velib/qt/ve_qitems_dbus.hpp>
#endif // QT_DBUS_LIB
#include <qnumeric.h>
#include <QsLog.h>
#include <velib/qt/ve_qitem.hpp>
#include "ve_qitem_consumer.h"

VeQItemConsumer::VeQItemConsumer(VeQItem *root, QObject *parent):
	QObject(parent),
	mRoot(root)
{
}

void VeQItemConsumer::setRoot(VeQItem *root)
{
	mRoot = root;
}

void VeQItemConsumer::onValueChanged(VeQItem *item, QVariant value)
{
	QLOG_WARN() << __func__ << item->uniqueId() << value.toString();
}

VeQItem *VeQItemConsumer::connectItem(const QString &path, const char *signal, bool forceSync)
{
	VeQItem *item = 0;
	connectItem(item, path, signal, forceSync);
	return item;
}

VeQItem *VeQItemConsumer::connectItem(const QString &path, double defaultValue,
									  double minValue, double maxValue, const char *signal,
									  bool silent)
{
	addSetting(path, defaultValue, minValue, maxValue, silent);
	return connectItem(path, signal);
}

VeQItem *VeQItemConsumer::connectItem(const QString &path, int defaultValue,
									  const char *signal, bool silent)
{
	addSetting(path, defaultValue, 0, 0, silent);
	return connectItem(path, signal);
}

VeQItem *VeQItemConsumer::connectItem(const QString &path, const QString &defaultValue,
									  const char *signal, bool silent)
{
	addSetting(path, defaultValue, 0, 0, silent);
	return connectItem(path, signal);
}

void VeQItemConsumer::connectItem(VeQItem *&item, const QString &path, const char *signal,
								  bool forceSync)
{
	VeQItem *newItem = mRoot->itemGetOrCreate(path, true);
	if (item == newItem)
		return;
	if (item != 0 && signal != 0) {
		QObject::disconnect(item, SIGNAL(valueChanged(VeQItem*,QVariant)), this, signal);
//		QObject::disconnect(item, SIGNAL(valueChanged(VeQItem*,QVariant)),
//						 this, SLOT(onValueChanged(VeQItem*,QVariant)));
	}
	item = newItem;
	if (signal != 0)
		QObject::connect(item, SIGNAL(valueChanged(VeQItem*,QVariant)), this, signal);
//	QObject::connect(item, SIGNAL(valueChanged(VeQItem*,QVariant)),
//					 this, SLOT(onValueChanged(VeQItem*,QVariant)));
#ifdef QT_DBUS_LIB
	if (forceSync) {
		NoStorageQItem *i = qobject_cast<NoStorageQItem *>(item);
		if (i != 0)
			i->setForceSync(true);
	}
	item->getValue();
#else
	Q_UNUSED(forceSync);
#endif // QT_DBUS_LIB
}

double VeQItemConsumer::getDouble(VeQItem *item)
{
	if (item == 0)
		return qQNaN();
	QVariant v = item->getValue();
	return v.isValid() ? v.toDouble() : qQNaN();
}

bool VeQItemConsumer::addSetting(const QString &path, const QVariant &defaultValue,
								 const QVariant &minValue, const QVariant &maxValue, bool silent)
{
	return addSetting(mRoot, path, defaultValue, minValue, maxValue, silent);
}

bool VeQItemConsumer::addSetting(VeQItem *root, const QString &path, const QVariant &defaultValue,
								 const QVariant &minValue, const QVariant &maxValue, bool silent)
{
#ifdef QT_DBUS_LIB
	/// This will call the AddSetting function on com.victronenergy.settings. It should not be done
	/// here, because this class is supposed to be independent from VeQItem type. But since it is
	/// not implemented as part of the VeQItem framework, so it is better to do it here, than to
	/// shift the burden to the users of this class.
	QString px = root->getRelId(root->producer()->services()) + "/" + path;
	px.replace("/com.victronenergy.settings", "");
	int pos = px.startsWith('/') ? 1 : 0;
	if (px.midRef(pos, 8) != "Settings") {
		QLOG_ERROR() << "Settings path should start with Settings: " << px;
		return false;
	}
	int groupStart = px.indexOf('/', pos);
	if (groupStart == -1) {
		QLOG_ERROR() << "Settings path should contain group name: " << px;
		return false;
	}
	int nameStart = px.lastIndexOf('/');
	if (nameStart <= groupStart) {
		QLOG_ERROR() << "Settings path should contain name: " << px;
		return false;
	}
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
	VeQItemDbusProducer *p = qobject_cast<VeQItemDbusProducer *>(root->producer());
	if (p == 0) {
		QLOG_ERROR() << "No D-Bus producer found";
		return false;
	}
	QString group = px.mid(groupStart + 1, nameStart - groupStart - 1);
	QString name = px.mid(nameStart + 1);
	QDBusConnection &connection = p->dbusConnection();
	QDBusMessage m = QDBusMessage::createMethodCall(
						 "com.victronenergy.settings",
						 "/Settings",
						 "com.victronenergy.Settings",
						 silent ? "AddSilentSetting" : "AddSetting")
					 << group
					 << name
					 << QVariant::fromValue(QDBusVariant(defaultValue))
					 << QString(type)
					 << QVariant::fromValue(QDBusVariant(minValue))
					 << QVariant::fromValue(QDBusVariant(maxValue));
	QDBusMessage reply = connection.call(m);
	if (reply.type() != QDBusMessage::ReplyMessage) {
		QLOG_ERROR() << "Could not create setting" << px << reply.errorMessage();
		return false;
	}
	return true;
#else
	Q_UNUSED(minValue)
	Q_UNUSED(maxValue)
	Q_UNUSED(silent)
	VeQItem *newItem = root->itemGetOrCreate(path, true);
	if (!newItem->getValue().isValid())
		newItem->setValue(defaultValue);
	return true;
#endif // QT_DBUS_LIB
}

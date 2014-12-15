#include <QDebug>
#include <QMetaType>
#include <velib/qt/v_busitem.h>
#include "dbus_settings_guard.h"
#include "settings.h"

Q_DECLARE_METATYPE(QList<QHostAddress>)
Q_DECLARE_METATYPE(QHostAddress)

DBusSettingsGuard::DBusSettingsGuard(Settings *settings, QObject *parent) :
	QObject(parent),
	mSettings(settings)
{
	addBusItem("/Settings/Fronius/AutoDetect", "autoDetect");
	addBusItem("/Settings/Fronius/IPAddresses", "ipAddresses");
	addBusItem("/Settings/Fronius/KnownIPAddresses", "knownIpAddresses");
	connect(settings, SIGNAL(propertyChanged(QString)),
			this, SLOT(onPropertyChanged(QString)));
}

void DBusSettingsGuard::onPropertyChanged(const QString &property)
{
	qDebug() << __FUNCTION__ << property;
	foreach (ItemPropertyBridge b, mVBusItems) {
		if (b.property == property) {
			QVariant value = mSettings->property(property.toAscii().data());
			if (property == "ipAddresses" || property == "knownIpAddresses") {
				// Convert QList<QHostAddress> to QStringList, because DBus
				// support for custom types is sketchy. (How do you supply type
				// information?)
				QStringList addresses;
				foreach (QHostAddress a, value.value<QList<QHostAddress> >()) {
					addresses.append(a.toString());
				}
				value = addresses;
			}
			b.item->setValue(value);
		}
	}
}

void DBusSettingsGuard::onVBusItemChanged()
{
	qDebug() << __FUNCTION__;
	foreach (ItemPropertyBridge b, mVBusItems) {
		if (b.item == sender()) {
			qDebug() << __FUNCTION__ << b.property;
			QVariant value = b.item->getValue();
			if (b.property == "ipAddresses" ||
					b.property == "knownIpAddresses") {
				// Convert from DBus type back to to QList<QHostAddress>.
				// See onPropertyChanged.
				QList<QHostAddress> addresses;
				foreach (QString a, value.toStringList()) {
					addresses.append(QHostAddress(a));
				}
				value = QVariant::fromValue(addresses);
			}
			mSettings->setProperty(b.property.toAscii().data(), value);
			break;
		}
	}
}

void DBusSettingsGuard::addBusItem(const QString &path, const QString &property)
{
	qDebug() << __FUNCTION__ << path << property;
	QDBusConnection connection = QDBusConnection::sessionBus();
	VBusItem *item = new VBusItem(this);
	item->consume(connection, "com.victronenergy.settings", path);
	connect(item, SIGNAL(valueChanged()), this, SLOT(onVBusItemChanged()));
	ItemPropertyBridge b;
	b.item = item;
	b.property = property;
	mVBusItems.append(b);
	// Force DBus request on the bus, so we will receive the current value.
	item->getValue();
}

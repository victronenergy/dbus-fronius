#ifndef DBUS_SETTINGS_BRIDGE_H
#define DBUS_SETTINGS_BRIDGE_H

#include <QObject>
#include <QHostAddress>
#include "dbus_bridge.h"

class InverterGateway;
class InverterSettings;
class QDBusVariant;
class Settings;

Q_DECLARE_METATYPE(QList<QHostAddress>)
Q_DECLARE_METATYPE(QHostAddress)
Q_DECLARE_METATYPE(QList<InverterSettings *>)

/*!
 * @brief Setup synchronization between a `Settings` object and the DBus
 * settings service.
 */
class DBusSettingsBridge : public DBusBridge
{
	Q_OBJECT
public:
	DBusSettingsBridge(Settings *settings, InverterGateway *gateway,
					   QObject *parent = 0);

	/*!
	 * @brief Make sure the DBus objects are present.
	 * This function will call the AddSetting method on the settings dbus
	 * service for each settings object. *This function must be called before
	 * any `DBusSettingsBridge` instance is created.*
	 * @return true if successful.
	 */
	static bool addDBusObjects();

protected:
	virtual void toDBus(const QString &path, QVariant &value);

	virtual void fromDBus(const QString &path, QVariant &v);

private:
	Settings *mSettings;
};

#endif // DBUS_SETTINGS_BRIDGE_H

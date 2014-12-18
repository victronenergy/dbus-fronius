#ifndef DBUS_SETTINGS_GUARD_H
#define DBUS_SETTINGS_GUARD_H

#include <QObject>
#include "dbus_guard.h"

class InverterGateway;
class Settings;

class DBusSettingsGuard : public DBusGuard
{
	Q_OBJECT
public:
	DBusSettingsGuard(Settings *settings, InverterGateway *gateway,
					  QObject *parent = 0);

protected:
	virtual void toDBus(const QString &path, QVariant &value);

	virtual void fromDBus(const QString &path, QVariant &v);
};

#endif // DBUS_SETTINGS_GUARD_H

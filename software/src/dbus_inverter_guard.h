#ifndef DBUS_INVERTER_GUARD_H
#define DBUS_INVERTER_GUARD_H

#include <QString>
#include "dbus_guard.h"

class Inverter;
class InverterGateway;
class PowerInfo;

class DBusInverterGuard : public DBusGuard
{
	Q_OBJECT
public:
	explicit DBusInverterGuard(Inverter *inverter);

private:
	void addBusItems(QDBusConnection &connection, PowerInfo *pi,
					 const QString &path);

	static QString fixServiceNameFragment(const QString &s);

	const Inverter *mInverter;
};

#endif // DBUS_INVERTER_GUARD_H

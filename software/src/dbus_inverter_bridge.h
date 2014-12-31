#ifndef DBUS_INVERTER_BRIDGE_H
#define DBUS_INVERTER_BRIDGE_H

#include <QString>
#include "dbus_bridge.h"

class Inverter;
class InverterGateway;
class PowerInfo;

/*!
 * \brief Connects data from `Inverter` to the DBus.
 * This class creates and fills the com.victronenergy.pvinverter_xxx service.
 */
class DBusInverterBridge : public DBusBridge
{
	Q_OBJECT
public:
	explicit DBusInverterBridge(Inverter *inverter, QObject *parent = 0);

protected:
	virtual void toDBus(const QString &path, QVariant &value);

	virtual void fromDBus(const QString &path, QVariant &value);

private:
	void addBusItems(QDBusConnection &connection, PowerInfo *pi,
					 const QString &path);

	static QString fixServiceNameFragment(const QString &s);
};

#endif // DBUS_INVERTER_BRIDGE_H

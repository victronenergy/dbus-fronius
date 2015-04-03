#ifndef DBUS_INVERTER_BRIDGE_H
#define DBUS_INVERTER_BRIDGE_H

#include <QPointer>
#include <QString>
#include "dbus_bridge.h"

class Inverter;
class InverterSettings;
class PowerInfo;

/*!
 * \brief Connects data from `Inverter` to the DBus.
 * This class creates and fills the com.victronenergy.pvinverter_xxx service.
 */
class DBusInverterBridge : public DBusBridge
{
	Q_OBJECT
public:
	explicit DBusInverterBridge(Inverter *inverter, InverterSettings *settings,
								QObject *parent = 0);

protected:
	virtual bool toDBus(const QString &path, QVariant &value);

	virtual bool fromDBus(const QString &path, QVariant &value);

private:
	void addBusItems(PowerInfo *pi, const QString &path);

	static QString fixServiceNameFragment(const QString &s);

	Inverter *mInverter;
};

#endif // DBUS_INVERTER_BRIDGE_H

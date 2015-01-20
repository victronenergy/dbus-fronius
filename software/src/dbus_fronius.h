#ifndef DBUS_TEST2_H
#define DBUS_TEST2_H

#include <QObject>

class DBusGatewayBridge;
class DBusSettingsBridge;
class InverterGateway;
class InverterUpdater;
class Inverter;
class Settings;

class DBusFronius : public QObject
{
	Q_OBJECT
public:
	DBusFronius(QObject *parent = 0);

private slots:
	void onSettingsInitialized();

	void onInverterFound(Inverter *inverter);

	void onInverterSettingsInitialized();

	void onInverterInitialized();

	void onIsConnectedChanged();

private:
	Inverter *findInverter(int deviceType, const QString &uniqueId);

	Settings *mSettings;
	InverterGateway *mGateway;
	DBusSettingsBridge *mSettingsBridge;
	DBusGatewayBridge *mGatewayBridge;
	QList<Inverter *> mInverters;
};

#endif // DBUS_TEST2_H

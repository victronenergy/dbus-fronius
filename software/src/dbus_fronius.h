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
	InverterUpdater *findUpdater(const QString &hostName,
								 const QString &deviceId);

	Settings *mSettings;
	InverterGateway *mGateway;
	DBusSettingsBridge *mSettingsBridge;
	DBusGatewayBridge *mGatewayBridge;
	QList<InverterUpdater *> mUpdaters;
};

#endif // DBUS_TEST2_H

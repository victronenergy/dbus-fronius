#ifndef DBUS_TEST2_H
#define DBUS_TEST2_H

#include <QObject>

class DBusGatewayBridge;
class DBusSettingsBridge;
class InverterGateway;
class InverterUpdater;
class Settings;

class DBusFronius : public QObject
{
	Q_OBJECT
public:
	DBusFronius(QObject *parent = 0);

private slots:
	void onInverterFound(InverterUpdater *iu);

	void onInverterInitialized();

	void onSettingsInitialized();

private:
	Settings *mSettings;
	InverterGateway *mGateway;
	DBusSettingsBridge *mSettingsBridge;
	DBusGatewayBridge *mGatewayBridge;
};

#endif // DBUS_TEST2_H

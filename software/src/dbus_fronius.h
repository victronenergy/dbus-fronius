#ifndef DBUS_TEST2_H
#define DBUS_TEST2_H

#include <QObject>

class DBusGatewayBridge;
class DBusSettingsBridge;
class Inverter;
class InverterGateway;
class InverterMediator;
class Settings;

class DBusFronius : public QObject
{
	Q_OBJECT
public:
	DBusFronius(QObject *parent = 0);

private slots:
	void onSettingsInitialized();

	void onInverterFound(Inverter *inverter);

private:
	Settings *mSettings;
	InverterGateway *mGateway;
	DBusSettingsBridge *mSettingsBridge;
	DBusGatewayBridge *mGatewayBridge;
	QList<InverterMediator *> mMediators;
};

#endif // DBUS_TEST2_H

#ifndef DBUS_TEST2_H
#define DBUS_TEST2_H

#include <QObject>

class DBusSettingsGuard;
class InverterGateway;
class InverterUpdater;
class Settings;

class DBusTest : public QObject
{
	Q_OBJECT
public:
	DBusTest(QObject *parent = 0);

private slots:
	void onInverterFound(InverterUpdater &iu);

	void onInverterInitialized();

private:
	Settings *mSettings;
	InverterGateway *mGateway;
	DBusSettingsGuard *mSettingsGuard;
};

#endif // DBUS_TEST2_H

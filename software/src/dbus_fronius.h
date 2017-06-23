#ifndef DBUS_TEST2_H
#define DBUS_TEST2_H

#include <QObject>
#include "defines.h"

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

	void onInverterFound(DeviceInfo deviceInfo);

private:
	Settings *mSettings;
	InverterGateway *mGateway;
	QList<InverterMediator *> mMediators;
};

#endif // DBUS_TEST2_H

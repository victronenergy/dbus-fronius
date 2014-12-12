#ifndef DBUS_TEST2_H
#define DBUS_TEST2_H

#include <QObject>
#include <QVariantMap>

class DBusInverterGuard;
class Inverter;
class QTimer;
class QDBusMessage;
class VBusItem;
class InverterGateway;
class InverterUpdater;

class DBusTest : public QObject
{
	Q_OBJECT
public:
	DBusTest(QObject *parent = 0);

private slots:
	void onInverterFound(InverterUpdater &iu);

	void onInverterInitialized();

private:
	InverterGateway *mGateway;
};

#endif // DBUS_TEST2_H

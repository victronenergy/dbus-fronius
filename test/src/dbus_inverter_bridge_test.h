#ifndef DBUS_INVERTER_BRIDGE_TEST_H
#define DBUS_INVERTER_BRIDGE_TEST_H

#include <QObject>

class Inverter;
class PowerInfo;

class DBusInverterBridgeTest : public QObject
{
	Q_OBJECT
public:
	DBusInverterBridgeTest(QObject *parent = 0);

private slots:
	void constructor();

	void constructor3Phased();

	void isConnected();

	void acCurrent();

	void acVoltage();

	void acPower();

private:
	void processEvents(int sleep, int count);

	void checkValue(Inverter *inverter, PowerInfo *pi, const QString &path,
					double v0, double v1, const QString &text,
					const char *property);
};

#endif // DBUS_INVERTER_BRIDGE_TEST_H

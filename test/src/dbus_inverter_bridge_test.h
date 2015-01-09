#ifndef DBUS_INVERTER_BRIDGE_TEST
#define DBUS_INVERTER_BRIDGE_TEST

#include <gtest/gtest.h>
#include <QScopedPointer>

class DBusInverterBridge;
class DBusObserver;
class Inverter;
class InverterSettings;
class PowerInfo;
class QVariant;

class DBusInverterBridgeTest : public testing::Test
{
public:
	DBusInverterBridgeTest();

protected:
	virtual void SetUp();

	virtual void TearDown();

	void SetUpBridge();

	void checkValue(PowerInfo *pi, const QString &path,
					double v0, double v1, const QString &text,
					const char *property);

	void checkValue(const QVariant &expected, const QVariant &actual);

	const QString mServiceName;
	QScopedPointer<Inverter> mInverter;
	QScopedPointer<InverterSettings> mSettings;
	QScopedPointer<DBusInverterBridge> mBridge;
	QScopedPointer<DBusObserver> mDBusClient;
};

#endif // DBUS_INVERTER_BRIDGE_TEST

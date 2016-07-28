#ifndef DBUS_INVERTER_BRIDGE_TEST
#define DBUS_INVERTER_BRIDGE_TEST

#include <gtest/gtest.h>
#include <QScopedPointer>
#include <QString>

class DBusInverterBridge;
class DBusObserver;
class Inverter;
class InverterSettings;
class PowerInfo;
class QVariant;
class VeQItemProducer;

class DBusInverterBridgeTest : public testing::Test
{
public:
	DBusInverterBridgeTest();

protected:
	static void SetUpTestCase();

	virtual void SetUp();

	virtual void TearDown();

	void SetUpBridge();

	void setupPowerInfo(PowerInfo *pi);

	static QVariant getValue(const QString &service, const QString &path);

	static QString getText(const QString &service, const QString &path);

	void checkValue(PowerInfo *pi, const QString &path,
					double v0, double v1, const QString &text,
					const char *property);

	void checkValue(const QVariant &expected, const QVariant &actual);

	const QString mServiceName;
	QScopedPointer<Inverter> mInverter;
	QScopedPointer<InverterSettings> mSettings;
	QScopedPointer<DBusInverterBridge> mBridge;
	QScopedPointer<VeQItemProducer> mItemProducer;
};

#endif // DBUS_INVERTER_BRIDGE_TEST

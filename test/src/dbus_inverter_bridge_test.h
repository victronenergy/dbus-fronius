#ifndef DBUS_INVERTER_BRIDGE_TEST
#define DBUS_INVERTER_BRIDGE_TEST

#include <gtest/gtest.h>
#include <QScopedPointer>
#include <QString>

class Inverter;
class InverterSettings;
class PowerInfo;
class QVariant;
class VeQItem;
class VeQItemProducer;

class DBusInverterBridgeTest : public testing::Test
{
public:
	DBusInverterBridgeTest();

protected:
	static void SetUpTestCase();

	virtual void SetUp();

	virtual void TearDown();

	void setupPowerInfo(PowerInfo *pi);

	static QVariant getValue(const QString &service, const QString &path);

	static VeQItem *getItem(const QString &service, const QString &path);

	static QString getText(const QString &service, const QString &path);

	void checkValue(PowerInfo *pi, const QString &path,
					double v0, double v1, const QString &text,
					const char *property);

	void checkValue(const QVariant &expected, const QVariant &actual);

	void checkValue(VeQItem *item, const QVariant &expected, const QString &text);

	const QString mServiceName;
	QScopedPointer<Inverter> mInverter;
	QScopedPointer<InverterSettings> mSettings;
	QScopedPointer<VeQItemProducer> mItemProducer;
	QScopedPointer<VeQItemProducer> mItemSubscriber;
	VeQItem *mPosition;
	VeQItem *mPhase;
	VeQItem *mSerialNumber;
};

#endif // DBUS_INVERTER_BRIDGE_TEST

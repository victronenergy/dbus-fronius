#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QStringList>
#include <unistd.h>
#include "dbus_observer.h"
#include "dbus_inverter_bridge.h"
#include "dbus_inverter_bridge_test.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "power_info.h"
#include "test_helper.h"
#include "version.h"

TEST_F(DBusInverterBridgeTest, constructor)
{
	mInverter->meanPowerInfo()->setCurrent(1.4);
	mInverter->meanPowerInfo()->setPower(137);
	mInverter->meanPowerInfo()->setVoltage(342);
	mInverter->l2PowerInfo()->setCurrent(1.7);
	mInverter->l2PowerInfo()->setPower(138);
	mInverter->l2PowerInfo()->setVoltage(344);

	SetUpBridge();

	checkValue(QVariant(0), mDBusClient->getValue(mServiceName, "/Connected"));

	checkValue(QCoreApplication::arguments()[0],
			   mDBusClient->getValue(mServiceName, "/Mgmt/ProcessName"));
	checkValue(QString(VERSION),
			   mDBusClient->getValue(mServiceName, "/Mgmt/ProcessVersion"));
	checkValue(QString("10.0.1.4 - 3"),
			   mDBusClient->getValue(mServiceName, "/Mgmt/Connection"));
	checkValue(QVariant(2),
			   mDBusClient->getValue(mServiceName, "/Position"));
	// Note: we don't take the product ID from VE_PROD_ID_PV_INVERTER_FRONIUS,
	// because we want this test to fail if someone changes the define.
	checkValue(QString::number(0xA142),
			   mDBusClient->getValue(mServiceName, "/ProductId").toString());
	checkValue(QString("Fronius PV inverter"),
			   mDBusClient->getValue(mServiceName, "/ProductName"));
	checkValue(QString(""),
			   mDBusClient->getValue(mServiceName, "/CustomName"));
	checkValue(QVariant(22),
			   mDBusClient->getValue(mServiceName, "/DeviceInstance"));

	checkValue(QVariant(1.4), mDBusClient->getValue(mServiceName, "/Ac/Current"));
	checkValue(QVariant(342.0), mDBusClient->getValue(mServiceName, "/Ac/Voltage"));
	checkValue(QVariant(137.0), mDBusClient->getValue(mServiceName, "/Ac/Power"));

	checkValue(QVariant(QStringList()), mDBusClient->getValue(mServiceName, "/Ac/L1/Power"));

	checkValue(QVariant(1.7), mDBusClient->getValue(mServiceName, "/Ac/L2/Current"));
	checkValue(QVariant(344.0), mDBusClient->getValue(mServiceName, "/Ac/L2/Voltage"));
	checkValue(QVariant(138.0), mDBusClient->getValue(mServiceName, "/Ac/L2/Power"));
}

TEST_F(DBusInverterBridgeTest, isConnected)
{
	SetUpBridge();

	EXPECT_FALSE(mInverter->isConnected());
	checkValue(QVariant(0), mDBusClient->getValue(mServiceName, "/Connected"));
	mInverter->setIsConnected(true);
	qWait(100);
	checkValue(QVariant(1), mDBusClient->getValue(mServiceName, "/Connected"));
}

TEST_F(DBusInverterBridgeTest, position)
{
	SetUpBridge();

	checkValue(QVariant(2), mDBusClient->getValue(mServiceName, "/Position"));
	mSettings->setPosition(Output);
	qWait(100);
	checkValue(QVariant(1), mDBusClient->getValue(mServiceName, "/Position"));
}

TEST_F(DBusInverterBridgeTest, deviceInstance)
{
	SetUpBridge();

	checkValue(QVariant(22), mDBusClient->getValue(mServiceName, "/DeviceInstance"));
	mSettings->setPosition(Output);
	qWait(100);
	checkValue(QVariant(21), mDBusClient->getValue(mServiceName, "/DeviceInstance"));
}

TEST_F(DBusInverterBridgeTest, acCurrent)
{
	SetUpBridge();
	checkValue(mInverter->meanPowerInfo(), "/Ac/Current", 3.41, 2.03, "2.0A",
			   "current");
}

TEST_F(DBusInverterBridgeTest, acVoltage)
{
	SetUpBridge();
	checkValue(mInverter->meanPowerInfo(), "/Ac/Voltage", 231, 232.9, "233V",
			   "voltage");
}

TEST_F(DBusInverterBridgeTest, acPower)
{
	SetUpBridge();
	checkValue(mInverter->meanPowerInfo(), "/Ac/Power", 3412, 3417.3, "3417W",
			   "power");
}

DBusInverterBridgeTest::DBusInverterBridgeTest():
	mServiceName("com.victronenergy.pvinverter.fronius_010000001004_3")
{
}

void DBusInverterBridgeTest::SetUpTestCase()
{
	qRegisterMetaType<InverterPosition>("Position");
	qRegisterMetaType<InverterPhase>("Phase");
}

void DBusInverterBridgeTest::SetUp()
{
	mInverter.reset(new Inverter("10.0.1.4", 80, "3", "756", "cn"));
	mSettings.reset(new InverterSettings(mInverter->uniqueId()));
	mSettings->setPosition(Input2);
	mSettings->setPhase(PhaseL2);
}

void DBusInverterBridgeTest::TearDown()
{
	mDBusClient.reset();
	mBridge.reset();
	mInverter.reset();
	mSettings.reset();
}

void DBusInverterBridgeTest::SetUpBridge()
{
	mBridge.reset(new DBusInverterBridge(mInverter.data(), mSettings.data()));
	mDBusClient.reset(new DBusObserver(mServiceName));
	while (!mDBusClient->isInitialized(mServiceName)) {
		QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
		usleep(10000);
	}
}

void DBusInverterBridgeTest::checkValue(PowerInfo *pi, const QString &path,
										double v0, double v1,
										const QString &text,
										const char *property)
{
	pi->setProperty(property, v0);
	qWait(100);
	checkValue(QVariant(v0), mDBusClient->getValue(mServiceName, path));

	pi->setProperty(property, v1);
	qWait(100);
	checkValue(QVariant(v1), mDBusClient->getValue(mServiceName, path));
	EXPECT_EQ(text, mDBusClient->getText(mServiceName, path));
}

void DBusInverterBridgeTest::checkValue(const QVariant &expected,
										const QVariant &actual)
{
	EXPECT_EQ(expected.type(), actual.type());
	EXPECT_EQ(expected, actual);
}

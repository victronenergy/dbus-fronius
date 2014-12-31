#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QStringList>
#include <unistd.h>
#include "dbus_observer.h"
#include "dbus_inverter_bridge.h"
#include "dbus_inverter_bridge_test.h"
#include "inverter.h"
#include "power_info.h"
#include "test_helper.h"
#include "version.h"

TEST_F(DBusInverterBridgeTest, constructor)
{
	SetUpBridge();

	EXPECT_EQ(QVariant(0), mDBusClient->getValue(mServiceName, "/Connected"));

	EXPECT_EQ(QCoreApplication::arguments()[0],
			  mDBusClient->getValue(mServiceName, "/Mgmt/ProcessName").toString());
	EXPECT_EQ(QString(VERSION),
			  mDBusClient->getValue(mServiceName, "/Mgmt/ProcessVersion").toString());
	EXPECT_EQ(QString("10.0.1.4 - 3"),
			  mDBusClient->getValue(mServiceName, "/Mgmt/Connection").toString());
	EXPECT_EQ(QString("3"),
			  mDBusClient->getValue(mServiceName, "/Position").toString());
	// Note: we don't take the product ID from VE_PROD_ID_PV_INVERTER_FRONIUS,
	// because we want this test to fail if someone changes the define.
	EXPECT_EQ(QString::number(0xA142),
			  mDBusClient->getValue(mServiceName, "/ProductId").toString());
	EXPECT_EQ(QString("Fronius PV inverter - cn"),
			  mDBusClient->getValue(mServiceName, "/ProductName").toString());

	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/Current"));
	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/Voltage"));
	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/Power"));

	EXPECT_FALSE(mDBusClient->getValue(mServiceName, "/Ac/L1/Current").isValid());
}

TEST_F(DBusInverterBridgeTest, constructor3Phased)
{
	mInverter->setSupports3Phases(true);
	SetUpBridge();

	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/Power"));
	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/Current"));
	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/Voltage"));
	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/L1/Current"));
	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/L1/Voltage"));
	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/L2/Current"));
	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/L2/Voltage"));
	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/L3/Current"));
	EXPECT_EQ(QVariant(0.0), mDBusClient->getValue(mServiceName, "/Ac/L3/Voltage"));
}

TEST_F(DBusInverterBridgeTest, isConnected)
{
	SetUpBridge();

	EXPECT_FALSE(mInverter->isConnected());
	EXPECT_EQ(QVariant(0), mDBusClient->getValue(mServiceName, "/Connected"));
	mInverter->setIsConnected(true);
	qWait(100);
	EXPECT_EQ(QVariant(1), mDBusClient->getValue(mServiceName, "/Connected"));
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

void DBusInverterBridgeTest::SetUp()
{
	mInverter.reset(new Inverter("10.0.1.4", 80, "3", "756", "cn"));
}

void DBusInverterBridgeTest::TearDown()
{
	mDBusClient.reset();
	mBridge.reset();
	mInverter.reset();
}

void DBusInverterBridgeTest::SetUpBridge()
{
	mBridge.reset(new DBusInverterBridge(mInverter.data()));
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
	EXPECT_EQ(QVariant(v0), mDBusClient->getValue(mServiceName, path));

	pi->setProperty(property, v1);
	qWait(100);
	EXPECT_EQ(QVariant(v1), mDBusClient->getValue(mServiceName, path));
	EXPECT_EQ(text, mDBusClient->getText(mServiceName, path));
}

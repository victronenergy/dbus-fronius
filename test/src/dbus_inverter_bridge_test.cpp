#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QStringList>
#include "dbus_observer.h"
#include "dbus_inverter_bridge.h"
#include "inverter.h"
#include "power_info.h"
#include "test_helper.h"
#include "version.h"

static void checkValue(Inverter *inverter, PowerInfo *pi, const QString &path,
					   double v0, double v1, const QString &text,
					   const char *property);

TEST(DBusInverterBridgeTest, constructor)
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	DBusInverterBridge bridge(&inverter);

	QString serviceName("com.victronenergy.pvinverter.fronius_10014_3");

	DBusObserver dbusClient(serviceName);
	qWait(300);

	EXPECT_EQ(dbusClient.getValue(serviceName, "/Connected").toInt(), 0);

	EXPECT_EQ(QCoreApplication::arguments()[0],
			  dbusClient.getValue(serviceName, "/Mgmt/ProcessName").toString());
	EXPECT_EQ(QString(VERSION),
			  dbusClient.getValue(serviceName, "/Mgmt/ProcessVersion").toString());
	EXPECT_EQ(QString("10.0.1.4 - 3"),
			  dbusClient.getValue(serviceName, "/Mgmt/Connection").toString());
	EXPECT_EQ(QString("3"),
			  dbusClient.getValue(serviceName, "/Position").toString());
	// Note: we don't take the product ID from VE_PROD_ID_PV_INVERTER_FRONIUS,
	// because we want this test to fail if someone changes the define.
	EXPECT_EQ(QString::number(0xA142),
			  dbusClient.getValue(serviceName, "/ProductId").toString());
	EXPECT_EQ(QString("Fronius PV inverter"),
			  dbusClient.getValue(serviceName, "/ProductName").toString());

	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/Current").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/Voltage").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/Power").toDouble());

	EXPECT_FALSE(dbusClient.getValue(serviceName, "/Ac/L1/Current").isValid());
}

TEST(DBusInverterBridgeTest, constructor3Phased)
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	inverter.setSupports3Phases(true);
	DBusInverterBridge bridge(&inverter);

	QString serviceName("com.victronenergy.pvinverter.fronius_10014_3");

	DBusObserver dbusClient(serviceName);
	qWait(300);

	EXPECT_EQ(0.0 ,dbusClient.getValue(serviceName, "/Ac/Power").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/Current").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/Voltage").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/L1/Power").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/L1/Current").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/L1/Voltage").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/L2/Power").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/L2/Current").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/L2/Voltage").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/L3/Power").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/L3/Current").toDouble());
	EXPECT_EQ(0.0, dbusClient.getValue(serviceName, "/Ac/L3/Voltage").toDouble());
}

TEST(DBusInverterBridgeTest, isConnected)
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	DBusInverterBridge bridge(&inverter);

	QString serviceName("com.victronenergy.pvinverter.fronius_10014_3");
	DBusObserver dbusClient(serviceName);
	qWait(300);

	EXPECT_FALSE(inverter.isConnected());
	EXPECT_EQ(0, dbusClient.getValue(serviceName, "/Connected").toInt());
	inverter.setIsConnected(true);
	qWait(100);
	EXPECT_EQ(1, dbusClient.getValue(serviceName, "/Connected").toInt());
}

TEST(DBusInverterBridgeTest, acCurrent)
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	checkValue(&inverter, inverter.meanPowerInfo(), "/Ac/Current", 3.41, 2.03,
			   "2.0A", "current");
}

TEST(DBusInverterBridgeTest, acVoltage)
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	checkValue(&inverter, inverter.meanPowerInfo(), "/Ac/Voltage", 231, 232.9,
			   "233V", "voltage");
}

TEST(DBusInverterBridgeTest, acPower)
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	checkValue(&inverter, inverter.meanPowerInfo(), "/Ac/Power", 3412, 3417.3,
			   "3417W", "power");
}

static void checkValue(Inverter *inverter, PowerInfo *pi, const QString &path,
					   double v0, double v1, const QString &text,
					   const char *property)
{
	pi->setProperty(property, v0);
	DBusInverterBridge bridge(inverter);

	QString serviceName("com.victronenergy.pvinverter.fronius_10014_3");
	DBusObserver dbusClient(serviceName);
	qWait(300);

	EXPECT_EQ(v0, dbusClient.getValue(serviceName, path).toDouble());
	pi->setProperty(property, v1);
	qWait(100);
	EXPECT_EQ(v1, dbusClient.getValue(serviceName, path).toDouble());

	EXPECT_EQ(text, dbusClient.getText(serviceName, path));
}

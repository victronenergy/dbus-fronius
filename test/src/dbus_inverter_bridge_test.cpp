#include <QTest>
#include "dbus_client.h"
#include "dbus_inverter_bridge.h"
#include "dbus_inverter_bridge_test.h"
#include "inverter.h"
#include "power_info.h"
#include "version.h"

DBusInverterBridgeTest::DBusInverterBridgeTest(QObject *parent):
	QObject(parent)
{
}

void DBusInverterBridgeTest::constructor()
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	DBusInverterBridge bridge(&inverter);

	QString serviceName("com.victronenergy.pvinverter.fronius_10014_3");

	DBusClient dbusClient(serviceName);
	QTest::qWait(300);

	QCOMPARE(dbusClient.getValue(serviceName, "/Connected").toInt(), 0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Mgmt/ProcessName").toString(),
			 QString(QCoreApplication::arguments()[0]));
	QCOMPARE(dbusClient.getValue(serviceName, "/Mgmt/ProcessVersion").toString(),
			 QString(VERSION));
	QCOMPARE(dbusClient.getValue(serviceName, "/Mgmt/Connection").toString(),
			 QString("10.0.1.4 - 3"));
	QCOMPARE(dbusClient.getValue(serviceName, "/Position").toString(),
			 QString("3"));
	// Note: we don't take the product ID from VE_PROD_ID_PV_INVERTER_FRONIUS,
	// because we want this test to fail if someone changes the define.
	QCOMPARE(dbusClient.getValue(serviceName, "/ProductId").toString(),
			 QString::number(0xA142));
	QCOMPARE(dbusClient.getValue(serviceName, "/ProductName").toString(),
			 QString("Fronius PV inverter"));

	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/Current").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/Voltage").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/Power").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/L1/Current"), QVariant());
}

void DBusInverterBridgeTest::constructor3Phased()
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	inverter.setSupports3Phases(true);
	DBusInverterBridge bridge(&inverter);

	QString serviceName("com.victronenergy.pvinverter.fronius_10014_3");

	DBusClient dbusClient(serviceName);
	QTest::qWait(300);

	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/Power").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/Current").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/Voltage").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/L1/Power").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/L1/Current").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/L1/Voltage").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/L2/Power").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/L2/Current").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/L2/Voltage").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/L3/Power").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/L3/Current").toDouble(),
			 0.0);
	QCOMPARE(dbusClient.getValue(serviceName, "/Ac/L3/Voltage").toDouble(),
			 0.0);
}

void DBusInverterBridgeTest::isConnected()
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	DBusInverterBridge bridge(&inverter);

	QString serviceName("com.victronenergy.pvinverter.fronius_10014_3");
	DBusClient dbusClient(serviceName);
	QTest::qWait(300);

	QVERIFY(!inverter.isConnected());
	QCOMPARE(dbusClient.getValue(serviceName, "/Connected").toInt(), 0);
	inverter.setIsConnected(true);
	QTest::qWait(100);
	QCOMPARE(dbusClient.getValue(serviceName, "/Connected").toInt(), 1);
}

void DBusInverterBridgeTest::acCurrent()
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	checkValue(&inverter, inverter.meanPowerInfo(), "/Ac/Current", 3.41, 2.03,
			   "2.0A", "current");
}

void DBusInverterBridgeTest::acVoltage()
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	checkValue(&inverter, inverter.meanPowerInfo(), "/Ac/Voltage", 231, 232.9,
			   "233V", "voltage");
}

void DBusInverterBridgeTest::acPower()
{
	Inverter inverter("10.0.1.4", 80, "3", "756");
	checkValue(&inverter, inverter.meanPowerInfo(), "/Ac/Power", 3412, 3417.3,
			   "3417W", "power");
}

void DBusInverterBridgeTest::checkValue(Inverter *inverter, PowerInfo *pi,
										const QString &path, double v0,
										double v1, const QString &text,
										const char *property)
{
	pi->setProperty(property, v0);
	DBusInverterBridge bridge(inverter);

	QString serviceName("com.victronenergy.pvinverter.fronius_10014_3");
	DBusClient dbusClient(serviceName);
	QTest::qWait(300);

	QCOMPARE(dbusClient.getValue(serviceName, path).toDouble(), v0);
	pi->setProperty(property, v1);
	QTest::qWait(100);
	QCOMPARE(dbusClient.getValue(serviceName, path).toDouble(), v1);

	QCOMPARE(dbusClient.getText(serviceName, path), text);
}

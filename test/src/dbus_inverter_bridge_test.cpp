#include <cmath>
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QStringList>
#include <velib/qt/ve_qitem.hpp>
#include "dbus_observer.h"
#include "dbus_inverter_bridge.h"
#include "dbus_inverter_bridge_test.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "power_info.h"
#include "test_helper.h"

TEST_F(DBusInverterBridgeTest, constructor)
{
	mInverter->meanPowerInfo()->setCurrent(1.4);
	mInverter->meanPowerInfo()->setPower(137);
	mInverter->meanPowerInfo()->setVoltage(342);
	mInverter->l2PowerInfo()->setCurrent(1.7);
	mInverter->l2PowerInfo()->setPower(138);
	mInverter->l2PowerInfo()->setVoltage(344);
	mInverter->l2PowerInfo()->setTotalEnergy(0);
	mInverter->setDeviceInstance(22);

	SetUpBridge();

	checkValue(QVariant(0), getValue(mServiceName, "/Connected"));

	checkValue(QCoreApplication::arguments()[0],
			   getValue(mServiceName, "/Mgmt/ProcessName"));
	checkValue(QString(VERSION),
			   getValue(mServiceName, "/Mgmt/ProcessVersion"));
	checkValue(QString("10.0.1.4 - 3"),
			   getValue(mServiceName, "/Mgmt/Connection"));
	checkValue(QVariant(2),
			   getValue(mServiceName, "/Position"));
	// Note: we don't take the product ID from VE_PROD_ID_PV_INVERTER_FRONIUS,
	// because we want this test to fail if someone changes the define.
	checkValue(QString::number(0xA142),
			   getValue(mServiceName, "/ProductId").toString());
	checkValue(QString("Fronius Symo 8.2-3-M"),
			   getValue(mServiceName, "/ProductName"));
	checkValue(QString("Fronius Symo 8.2-3-M"),
			   getValue(mServiceName, "/CustomName"));
	checkValue(QVariant(22),
			   getValue(mServiceName, "/DeviceInstance"));

	checkValue(QVariant(1.4), getValue(mServiceName, "/Ac/Current"));
	checkValue(QVariant(342.0), getValue(mServiceName, "/Ac/Voltage"));
	checkValue(QVariant(137.0), getValue(mServiceName, "/Ac/Power"));

	checkValue(QVariant(1.7), getValue(mServiceName, "/Ac/L2/Current"));
	checkValue(QVariant(344.0), getValue(mServiceName, "/Ac/L2/Voltage"));
	checkValue(QVariant(138.0), getValue(mServiceName, "/Ac/L2/Power"));
}

TEST_F(DBusInverterBridgeTest, isConnected)
{
	SetUpBridge();

	EXPECT_FALSE(mInverter->isConnected());
	checkValue(QVariant(0), getValue(mServiceName, "/Connected"));
	mInverter->setIsConnected(true);
	qWait(100);
	checkValue(QVariant(1), getValue(mServiceName, "/Connected"));
}

TEST_F(DBusInverterBridgeTest, position)
{
	SetUpBridge();

	checkValue(QVariant(2), getValue(mServiceName, "/Position"));
	mSettings->setPosition(Output);
	qWait(100);
	checkValue(QVariant(1), getValue(mServiceName, "/Position"));
}

TEST_F(DBusInverterBridgeTest, deviceInstance)
{
	SetUpBridge();

	checkValue(QVariant(-1), getValue(mServiceName, "/DeviceInstance"));
	mInverter->setDeviceInstance(21);
	qWait(100);
	checkValue(QVariant(21), getValue(mServiceName, "/DeviceInstance"));
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

TEST_F(DBusInverterBridgeTest, acPowerNaN)
{
	SetUpBridge();
	checkValue(mInverter->meanPowerInfo(), "/Ac/Power", 3412, std::numeric_limits<double>::quiet_NaN(), "",
			   "power");
}

DBusInverterBridgeTest::DBusInverterBridgeTest():
	mServiceName("pub/com.victronenergy.pvinverter.fronius_123_756")
{
}

void DBusInverterBridgeTest::SetUpTestCase()
{
	qRegisterMetaType<InverterPosition>("Position");
	qRegisterMetaType<InverterPhase>("Phase");
}

void DBusInverterBridgeTest::SetUp()
{
	mItemProducer.reset(new VeQItemProducer(VeQItems::getRoot(), "pub"));
	mItemProducer->open();
	mInverter.reset(new Inverter("10.0.1.4", 80, "3", 123, "756", "cn"));
	setupPowerInfo(mInverter->meanPowerInfo());
	setupPowerInfo(mInverter->l1PowerInfo());
	setupPowerInfo(mInverter->l2PowerInfo());
	setupPowerInfo(mInverter->l3PowerInfo());
	mSettings.reset(new InverterSettings(mInverter->deviceType(),
										 mInverter->uniqueId()));
	mSettings->setPosition(Input2);
	mSettings->setPhase(PhaseL2);
}

void DBusInverterBridgeTest::TearDown()
{
	mBridge.reset();
	mInverter.reset();
	mSettings.reset();
	mItemProducer.reset();
}

void DBusInverterBridgeTest::SetUpBridge()
{
	mBridge.reset(new DBusInverterBridge(mInverter.data(), mSettings.data()));
//	mDBusClient.reset(new DBusObserver(mServiceName));
//	while (!mDBusClient->isInitialized(mServiceName)) {
//		QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
//	}
}

void DBusInverterBridgeTest::setupPowerInfo(PowerInfo *pi)
{
	pi->setVoltage(0);
	pi->setCurrent(0);
	pi->setPower(0);
	pi->setTotalEnergy(0);
}

QVariant DBusInverterBridgeTest::getValue(const QString &service, const QString &path)
{
	VeQItem *item = VeQItems::getRoot()->itemGetOrCreate(service + path, true);
	return item->getValue();
}

QString DBusInverterBridgeTest::getText(const QString &service, const QString &path)
{
	VeQItem *item = VeQItems::getRoot()->itemGetOrCreate(service + path, true);
	return item->getText();
}

void DBusInverterBridgeTest::checkValue(PowerInfo *pi, const QString &path,
										double v0, double v1,
										const QString &text,
										const char *property)
{
	pi->setProperty(property, v0);
	qWait(100);
	checkValue(QVariant(v0), getValue(mServiceName, path));

	pi->setProperty(property, v1);
	qWait(100);
	QVariant vv1;
	if (!std::isnan(v1))
		vv1 = QVariant(v1);
	checkValue(vv1, getValue(mServiceName, path));
	EXPECT_EQ(text, getText(mServiceName, path));
}

void DBusInverterBridgeTest::checkValue(const QVariant &expected,
										const QVariant &actual)
{
	EXPECT_EQ(expected.type(), actual.type());
	EXPECT_EQ(expected, actual);
}

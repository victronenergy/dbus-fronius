#include <cmath>
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <qnumeric.h>
#include <QStringList>
#include <velib/qt/ve_qitem.hpp>
#include "dbus_inverter_bridge_test.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "power_info.h"
#include "test_helper.h"

TEST_F(DBusInverterBridgeTest, constructor)
{
	mInverter->meanPowerInfo()->setPower(137.7);
	mInverter->l2PowerInfo()->setCurrent(1.79);
	mInverter->l2PowerInfo()->setPower(138.2);
	mInverter->l2PowerInfo()->setVoltage(343.9);
	mInverter->l2PowerInfo()->setTotalEnergy(0);
	mInverter->setPosition(Output);
	mInverter->setCustomName("South2");

	checkValue(QCoreApplication::arguments()[0],
			   getValue(mServiceName, "/Mgmt/ProcessName"));
	checkValue(QString(VERSION),
			   getValue(mServiceName, "/Mgmt/ProcessVersion"));
	checkValue(QString("10.0.1.4 - 3 (solarapi)"),
			   getValue(mServiceName, "/Mgmt/Connection"));
	checkValue(QVariant(1),
			   getValue(mServiceName, "/Position"));
	// Note: we don't take the product ID from VE_PROD_ID_PV_INVERTER_FRONIUS,
	// because we want this test to fail if someone changes the define.
	checkValue(QString::number(0xA144),
			   getValue(mServiceName, "/ProductId").toString());
	checkValue(QString("Fronius Symo 8.2-3-M"),
			   getValue(mServiceName, "/ProductName"));
	checkValue(QString("South2"),
			   getValue(mServiceName, "/CustomName"));
	checkValue(QVariant(123),
			   getValue(mServiceName, "/DeviceInstance"));
	checkValue(QVariant(1),
			   getValue(mServiceName, "/Connected"));

	checkValue(getItem(mServiceName, "Ac/Power"), 137.7, "138W");

	checkValue(getItem(mServiceName, "Ac/L2/Current"), 1.79, "1.8A");
	checkValue(getItem(mServiceName, "Ac/L2/Voltage"), 343.9, "344V");
	checkValue(getItem(mServiceName, "Ac/L2/Power"), 138.2, "138W");
}

DBusInverterBridgeTest::DBusInverterBridgeTest():
	mServiceName("pub/com.victronenergy.pvinverter.test")
{
}

void DBusInverterBridgeTest::SetUpTestCase()
{
	qRegisterMetaType<InverterPosition>("Position");
	qRegisterMetaType<InverterPhase>("Phase");
}

void DBusInverterBridgeTest::SetUp()
{
	mItemProducer.reset(new VeProducer(VeQItems::getRoot(), "pub"));
	mItemSubscriber.reset(new VeQItemProducer(VeQItems::getRoot(), "sub"));
	DeviceInfo deviceInfo;
	deviceInfo.hostName = "10.0.1.4";
	deviceInfo.port = 80;
	deviceInfo.uniqueId = "756";
	deviceInfo.networkId = 3;
	deviceInfo.productName = "Fronius Symo 8.2-3-M";
	deviceInfo.productId = 0xA144;
	VeQItem *root = mItemProducer->services()->itemGetOrCreate("com.victronenergy.pvinverter.test");
	mInverter.reset(new Inverter(root, deviceInfo, 123));
	setupPowerInfo(mInverter->l1PowerInfo());
	setupPowerInfo(mInverter->l2PowerInfo());
	setupPowerInfo(mInverter->l3PowerInfo());
	mInverter->meanPowerInfo()->setPower(0);
	mInverter->meanPowerInfo()->setTotalEnergy(0);
	VeQItem *settingsRoot = mItemSubscriber->services()->itemGetOrCreate("com.victronenergy.settings/Settings/Fronius/I123");
	mPosition = settingsRoot->itemGetOrCreate("Position");
	mPosition->setValue(static_cast<int>(Input2));
	mPhase = settingsRoot->itemGetOrCreate("Phase");
	mPhase->setValue(static_cast<int>(PhaseL2));
	mSettings.reset(new InverterSettings(settingsRoot));
}

void DBusInverterBridgeTest::TearDown()
{
	mInverter.reset();
	mSettings.reset();
	mItemProducer.reset();
	mItemSubscriber.reset();
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
	VeQItem *item = getItem(service, path);
	return item->getValue();
}

VeQItem *DBusInverterBridgeTest::getItem(const QString &service, const QString &path)
{
	return
		VeQItems::getRoot()->itemGetOrCreate(service, false)->itemGetOrCreate(path, true);
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

void DBusInverterBridgeTest::checkValue(const QVariant &expected, const QVariant &actual)
{
	EXPECT_EQ(expected.type(), actual.type());
	EXPECT_EQ(expected, actual);
}

void DBusInverterBridgeTest::checkValue(VeQItem *item, const QVariant &expected, const QString &text)
{
	QVariant actual = item->getValue();
	EXPECT_EQ(expected.type(), actual.type());
	EXPECT_EQ(expected, actual);
	EXPECT_EQ(text, item->getText());
}

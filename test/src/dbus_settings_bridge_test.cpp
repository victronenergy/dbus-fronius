#include <gtest/gtest.h>
#include "dbus_bridge.h"
#include "dbus_settings_bridge.h"
#include "dbus_settings_bridge_test.h"
#include "settings.h"
#include "test_helper.h"

void checkValue(const QVariant &expected, const QVariant &actual)
{
	EXPECT_EQ(expected.isValid(), actual.isValid());
	EXPECT_EQ(expected, actual);
}

TEST_F(DBusSettingsBridgeTest, addObjects)
{
	checkValue(QVariant(), getValue("/Settings/Fronius/PortNumber"));
	Settings settings;
	DBusSettingsBridge bridge(&settings);
	checkValue(QVariant(80), getValue("/Settings/Fronius/PortNumber"));
	checkValue(QVariant(""), getValue("/Settings/Fronius/IPAddresses"));
	checkValue(QVariant(""), getValue("/Settings/Fronius/KnownIPAddresses"));
}

TEST_F(DBusSettingsBridgeTest, portNumber)
{
	// Check default value from DBus
	EXPECT_EQ(80, mSettings->portNumber());
//	mSettingsClient->resetChangedPaths();
	mSettings->setPortNumber(8080);
	qWait(100);
	// Check new value
	QVariant portNumber = getValue("/Settings/Fronius/PortNumber");
	EXPECT_EQ(QVariant(8080), portNumber);
	// Check DBus signal
//	const QList<QString> &cp = mSettingsClient->changedPaths();
//	ASSERT_EQ(1, cp.size());
//	EXPECT_EQ(QString("/Settings/Fronius/PortNumber"), cp.first());
}

TEST_F(DBusSettingsBridgeTest, portNumberRemote)
{
	EXPECT_EQ(80, mSettings->portNumber());
	// Set new value on DBus
	setValue("/Settings/Fronius/PortNumber", 8051);
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	EXPECT_EQ(8051, mSettings->portNumber());
}

TEST_F(DBusSettingsBridgeTest, changeIPAddresses)
{
	changeIPAddresses("ipAddresses", "/Settings/Fronius/IPAddresses");
}

TEST_F(DBusSettingsBridgeTest, changeIPAddressesRemote)
{
	changeIPAddressesRemote("ipAddresses", "/Settings/Fronius/IPAddresses");
}

TEST_F(DBusSettingsBridgeTest, changeKnownIPAddresses)
{
	changeIPAddresses("knownIpAddresses", "/Settings/Fronius/KnownIPAddresses");
}

TEST_F(DBusSettingsBridgeTest, changeKnownIPAddressesRemote)
{
	changeIPAddressesRemote("knownIpAddresses", "/Settings/Fronius/KnownIPAddresses");
}

void DBusSettingsBridgeTest::SetUp()
{
	mItemProducer.reset(new BridgeItemProducer(VeQItems::getRoot(), "sub"));
	mItemProducer->open();
	mSettings.reset(new Settings());
	mBridge.reset(new DBusSettingsBridge(mSettings.data(), 0));
}

void DBusSettingsBridgeTest::TearDown()
{
	mSettings.reset();
	mBridge.reset();
	mItemProducer.reset();
}

void DBusSettingsBridgeTest::changeIPAddresses(const char *property, const char *path)
{
	// Check default value from DBus
	EXPECT_TRUE(mSettings->property(property).value<QList<QHostAddress> >().isEmpty());
	// mSettingsClient->resetChangedPaths();
	QList<QHostAddress> newValues;
	newValues.append(QHostAddress("192.168.1.3"));
	newValues.append(QHostAddress("192.168.1.7"));
	mSettings->setProperty(property, QVariant::fromValue(newValues));
	qWait(100);
	// Check new value
	QVariant qv = getValue(path);
	EXPECT_EQ(QString("192.168.1.3,192.168.1.7"), qv.toString());
	// Check DBus signal
//	const QList<QString> &cp = mSettingsClient->changedPaths();
//	ASSERT_EQ(1, cp.size());
//	EXPECT_EQ(QString(path), cp.first());
}

void DBusSettingsBridgeTest::changeIPAddressesRemote(const char *property, const char *path)
{
	EXPECT_TRUE(mSettings->property(property).value<QList<QHostAddress> >().isEmpty());
	// Set new value on DBus
	setValue(path, "192.168.1.3,192.168.1.7");
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	QList<QHostAddress> addr = mSettings->property(property).
			value<QList<QHostAddress> >();
	QList<QHostAddress> expected;
	expected.append(QHostAddress("192.168.1.3"));
	expected.append(QHostAddress("192.168.1.7"));
	// Check DBus signal
	EXPECT_EQ(expected, addr);
}

QVariant DBusSettingsBridgeTest::getValue(const QString &path)
{
	VeQItem *item = VeQItems::getRoot()->itemGetOrCreate("sub/com.victronenergy.settings" + path, true);
	return item->getValue();
}

void DBusSettingsBridgeTest::setValue(const QString &path, const QVariant &v)
{
	VeQItem *item = VeQItems::getRoot()->itemGetOrCreate("sub/com.victronenergy.settings" + path, true);
	item->setValue(v);
}

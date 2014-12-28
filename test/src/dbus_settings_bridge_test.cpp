#include <gtest/gtest.h>
#include "dbus_settings.h"
#include "dbus_settings_bridge.h"
#include "settings.h"
#include "test_helper.h"

static void changeIPAddresses(const char *property, const char *path);
static void changeIPAddressesRemote(const char *property, const char *path);
static void checkValue(const QVariant &actual, const QVariant &expected);

TEST(DBusSettingsBridgeTest, addObjects)
{
	DBusSettings settingsClient;
	checkValue(settingsClient.getValue("/Settings/Fronius/AutoDetect"), QVariant());
	DBusSettingsBridge::addDBusObjects();
	checkValue(settingsClient.getValue("/Settings/Fronius/AutoDetect"), QVariant(false));
	checkValue(settingsClient.getValue("/Settings/Fronius/IPAddresses"), QVariant(""));
	checkValue(settingsClient.getValue("/Settings/Fronius/KnownIPAddresses"), QVariant(""));
	checkValue(settingsClient.getValue("/Settings/Fronius/ScanProgress"), QVariant(0));
}

TEST(DBusSettingsBridgeTest, changeAutoDetect)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	qWait(100);
	// Check default value from DBus
	EXPECT_FALSE(settings.autoDetect());
	settingsClient.resetChangedPaths();
	settings.setAutoDetect(true);
	qWait(100);
	// Check new value
	QVariant autoDetect = settingsClient.getValue("/Settings/Fronius/AutoDetect");
	EXPECT_TRUE(autoDetect.isValid());
	EXPECT_TRUE(autoDetect.toBool());
	// Check DBus signal
	const QList<QString> &cp = settingsClient.changedPaths();
	EXPECT_EQ(cp.size(), 1);
	EXPECT_EQ(cp.first(), QString("/Settings/Fronius/AutoDetect"));
}

TEST(DBusSettingsBridgeTest, changeAutoDetectRemote)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	qWait(100);
	EXPECT_FALSE(settings.autoDetect());
	// Set new value on DBus
	settingsClient.setValue("/Settings/Fronius/AutoDetect", true);
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	EXPECT_TRUE(settings.autoDetect());
}

TEST(DBusSettingsBridgeTest, changeIPAddresses)
{
	changeIPAddresses("ipAddresses", "/Settings/Fronius/IPAddresses");
}

TEST(DBusSettingsBridgeTest, changeIPAddressesRemote)
{
	changeIPAddressesRemote("ipAddresses", "/Settings/Fronius/IPAddresses");
}

TEST(DBusSettingsBridgeTest, changeKnownIPAddresses)
{
	changeIPAddresses("knownIpAddresses", "/Settings/Fronius/KnownIPAddresses");
}

TEST(DBusSettingsBridgeTest, changeKnownIPAddressesRemote)
{
	changeIPAddressesRemote("knownIpAddresses", "/Settings/Fronius/KnownIPAddresses");
}

static void changeIPAddresses(const char *property, const char *path)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	qWait(100);
	// Check default value from DBus
	EXPECT_TRUE(settings.property(property).value<QList<QHostAddress> >().isEmpty());
	settingsClient.resetChangedPaths();
	QList<QHostAddress> newValues;
	newValues.append(QHostAddress("192.168.1.3"));
	newValues.append(QHostAddress("192.168.1.7"));
	settings.setProperty(property, QVariant::fromValue(newValues));
	qWait(100);
	// Check new value
	QVariant qv = settingsClient.getValue(path);
	EXPECT_EQ(qv.toString(), QString("192.168.1.3,192.168.1.7"));
	// Check DBus signal
	const QList<QString> &cp = settingsClient.changedPaths();
	EXPECT_EQ(cp.size(), 1);
	EXPECT_EQ(cp.first(), QString(path));
}

static void changeIPAddressesRemote(const char *property, const char *path)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	qWait(100);
	EXPECT_TRUE(settings.property(property).value<QList<QHostAddress> >().isEmpty());
	// Set new value on DBus
	settingsClient.setValue(path, "192.168.1.3,192.168.1.7");
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	QList<QHostAddress> addr = settings.property(property).
			value<QList<QHostAddress> >();
	QList<QHostAddress> expected;
	expected.append(QHostAddress("192.168.1.3"));
	expected.append(QHostAddress("192.168.1.7"));
	// Check DBus signal
	EXPECT_EQ(addr, expected);
}

static void checkValue(const QVariant &actual, const QVariant &expected)
{
	EXPECT_EQ(actual.isValid(), expected.isValid());
	EXPECT_EQ(actual, expected);
}

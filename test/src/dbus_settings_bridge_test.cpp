#include <gtest/gtest.h>
#include "dbus_settings.h"
#include "dbus_settings_bridge.h"
#include "settings.h"
#include "test_helper.h"

static void changeIPAddresses(const char *property, const char *path);
static void changeIPAddressesRemote(const char *property, const char *path);
static void checkValue(const QVariant &expected, const QVariant &actual);

TEST(DBusSettingsBridgeTest, addObjects)
{
	DBusSettings settingsClient;
	checkValue(QVariant(), settingsClient.getValue("/Settings/Fronius/AutoDetect"));
	DBusSettingsBridge::addDBusObjects();
	checkValue(QVariant(false), settingsClient.getValue("/Settings/Fronius/AutoDetect"));
	checkValue(QVariant(""), settingsClient.getValue("/Settings/Fronius/IPAddresses"));
	checkValue(QVariant(""), settingsClient.getValue("/Settings/Fronius/KnownIPAddresses"));
	checkValue(QVariant(0), settingsClient.getValue("/Settings/Fronius/ScanProgress"));
}

TEST(DBusSettingsBridgeTest, changeAutoDetect)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	qWait(300);
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
	EXPECT_EQ(1, cp.size());
	EXPECT_EQ(QString("/Settings/Fronius/AutoDetect"), cp.first());
}

TEST(DBusSettingsBridgeTest, changeAutoDetectRemote)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	qWait(300);
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
	qWait(300);
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
	EXPECT_EQ(QString("192.168.1.3,192.168.1.7"), qv.toString());
	// Check DBus signal
	const QList<QString> &cp = settingsClient.changedPaths();
	EXPECT_EQ(1, cp.size());
	EXPECT_EQ(QString(path), cp.first());
}

static void changeIPAddressesRemote(const char *property, const char *path)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	qWait(300);
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
	EXPECT_EQ(expected, addr);
}

static void checkValue(const QVariant &expected, const QVariant &actual)
{
	EXPECT_EQ(expected.isValid(), actual.isValid());
	EXPECT_EQ(expected, actual);
}

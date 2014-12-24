#include <QCoreApplication>
#include <QSignalSpy>
#include <QTest>
#include "dbus_settings.h"
#include "dbus_settings_bridge.h"
#include "dbus_settings_bridge_test.h"
#include "settings.h"

DBusSettingsBridgeTest::DBusSettingsBridgeTest(QObject *parent) :
	QObject(parent)
{
}

void DBusSettingsBridgeTest::init()
{
}

void DBusSettingsBridgeTest::cleanup()
{
}

void DBusSettingsBridgeTest::addObjects()
{
	DBusSettings settingsClient;
	checkValue(settingsClient.getValue("/Settings/Fronius/AutoDetect"), QVariant());
	DBusSettingsBridge::addDBusObjects();
	checkValue(settingsClient.getValue("/Settings/Fronius/AutoDetect"), QVariant(false));
	checkValue(settingsClient.getValue("/Settings/Fronius/IPAddresses"), QVariant(""));
	checkValue(settingsClient.getValue("/Settings/Fronius/KnownIPAddresses"), QVariant(""));
	checkValue(settingsClient.getValue("/Settings/Fronius/ScanProgress"), QVariant(0));
}

void DBusSettingsBridgeTest::changeAutoDetect()
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	QTest::qWait(100);
	// Check default value from DBus
	QCOMPARE(settings.autoDetect(), false);
	settingsClient.resetChangedPaths();
	settings.setAutoDetect(true);
	QTest::qWait(100);
	// Check new value
	QVariant autoDetect = settingsClient.getValue("/Settings/Fronius/AutoDetect");
	QVERIFY(autoDetect.isValid());
	QCOMPARE(autoDetect.toBool(), true);
	// Check DBus signal
	const QList<QString> &cp = settingsClient.changedPaths();
	QCOMPARE(cp.size(), 1);
	QCOMPARE(cp.first(), QString("/Settings/Fronius/AutoDetect"));
}

void DBusSettingsBridgeTest::changeAutoDetectRemote()
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	QTest::qWait(100);
	QCOMPARE(settings.autoDetect(), false);
	QSignalSpy spy(&settings, SIGNAL(autoDetectChanged()));
	// Set new value on DBus
	settingsClient.setValue("/Settings/Fronius/AutoDetect", true);
	// Allow value form DBus to trickle to our settings object
	QTest::qWait(100);
	QCOMPARE(settings.autoDetect(), true);
	// Check DBus signal
	QCOMPARE(spy.count(), 1);
}

void DBusSettingsBridgeTest::changeIPAddresses()
{
	changeIPAddresses("ipAddresses", "/Settings/Fronius/IPAddresses");
}

void DBusSettingsBridgeTest::changeIPAddressesRemote()
{
	changeIPAddressesRemote("ipAddresses", "/Settings/Fronius/IPAddresses");
}

void DBusSettingsBridgeTest::changeKnownIPAddresses()
{
	changeIPAddresses("knownIpAddresses", "/Settings/Fronius/KnownIPAddresses");
}

void DBusSettingsBridgeTest::changeKnownIPAddressesRemote()
{
	changeIPAddressesRemote("knownIpAddresses", "/Settings/Fronius/KnownIPAddresses");
}

void DBusSettingsBridgeTest::changeIPAddresses(const char *property,
											   const char *path)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	QTest::qWait(100);
	// Check default value from DBus
	QVERIFY(settings.property(property).value<QList<QHostAddress> >().isEmpty());
	settingsClient.resetChangedPaths();
	QList<QHostAddress> newValues;
	newValues.append(QHostAddress("192.168.1.3"));
	newValues.append(QHostAddress("192.168.1.7"));
	settings.setProperty(property, QVariant::fromValue(newValues));
	QTest::qWait(100);
	// Check new value
	QVariant qv = settingsClient.getValue(path);
	QCOMPARE(qv.toString(), QString("192.168.1.3,192.168.1.7"));
	// Check DBus signal
	const QList<QString> &cp = settingsClient.changedPaths();
	QCOMPARE(cp.size(), 1);
	QCOMPARE(cp.first(), QString(path));
}

void DBusSettingsBridgeTest::changeIPAddressesRemote(const char *property,
													 const char *path)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	QTest::qWait(100);
	QVERIFY(settings.property(property).value<QList<QHostAddress> >().isEmpty());
	QSignalSpy spy(&settings, SIGNAL(ipAddressesChanged()));
	// Set new value on DBus
	settingsClient.setValue(path, "192.168.1.3,192.168.1.7");
	// Allow value form DBus to trickle to our settings object
	QTest::qWait(100);
	QList<QHostAddress> addr = settings.property(property).
			value<QList<QHostAddress> >();
	QList<QHostAddress> expected;
	expected.append(QHostAddress("192.168.1.3"));
	expected.append(QHostAddress("192.168.1.7"));
	// Check DBus signal
	QCOMPARE(addr, expected);
}

void DBusSettingsBridgeTest::checkValue(const QVariant &actual,
										const QVariant &expected)
{
	QCOMPARE(actual.isValid(), expected.isValid());
	QCOMPARE(actual, expected);
}

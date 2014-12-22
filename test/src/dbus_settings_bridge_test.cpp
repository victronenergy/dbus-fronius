#include <QCoreApplication>
#include <QDebug>
#include <QSignalSpy>
#include <QTest>
#include "dbus_thread.h"
#include "dbus_settings.h"
#include "dbus_settings_bridge.h"
#include "dbus_settings_bridge_test.h"
#include "settings.h"

DbusSettingsBridgeTest::DbusSettingsBridgeTest(QObject *parent) :
	QObject(parent)
{
}

void DbusSettingsBridgeTest::init()
{
}

void DbusSettingsBridgeTest::cleanup()
{
}

void DbusSettingsBridgeTest::addObjects()
{
	DBusSettings settingsClient;
	checkValue(settingsClient.getValue("/Settings/Fronius/AutoDetect"), QVariant());
	DBusSettingsBridge::addDBusObjects();
	checkValue(settingsClient.getValue("/Settings/Fronius/AutoDetect"), QVariant(false));
	checkValue(settingsClient.getValue("/Settings/Fronius/IPAddresses"), QVariant(""));
	checkValue(settingsClient.getValue("/Settings/Fronius/KnownIPAddresses"), QVariant(""));
	checkValue(settingsClient.getValue("/Settings/Fronius/ScanProgress"), QVariant(0));
}

void DbusSettingsBridgeTest::changeAutoDetect()
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	processEvents(10, 10);
	// Check default value from DBus
	QCOMPARE(settings.autoDetect(), false);
	settingsClient.resetChangedPaths();
	settings.setAutoDetect(true);
	processEvents(10, 10);
	// Check new value
	QVariant autoDetect = settingsClient.getValue("/Settings/Fronius/AutoDetect");
	QVERIFY(autoDetect.isValid());
	QCOMPARE(autoDetect.toBool(), true);
	// Check DBus signal
	const QList<QString> &cp = settingsClient.changedPaths();
	QCOMPARE(cp.size(), 1);
	QCOMPARE(cp.first(), QString("/Settings/Fronius/AutoDetect"));
}

void DbusSettingsBridgeTest::changeAutoDetectRemote()
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	processEvents(10, 10);
	QCOMPARE(settings.autoDetect(), false);
	QSignalSpy spy(&settings, SIGNAL(autoDetectChanged()));
	// Set new value on DBus
	settingsClient.setValue("/Settings/Fronius/AutoDetect", true);
	// Allow value form DBus to trickle to our settings object
	processEvents(10, 10);
	QCOMPARE(settings.autoDetect(), true);
	// Check DBus signal
	QCOMPARE(spy.count(), 1);
}

void DbusSettingsBridgeTest::changeIPAddresses()
{
	changeIPAddresses("ipAddresses", "/Settings/Fronius/IPAddresses");
}

void DbusSettingsBridgeTest::changeIPAddressesRemote()
{
	changeIPAddressesRemote("ipAddresses", "/Settings/Fronius/IPAddresses");
}

void DbusSettingsBridgeTest::changeKnownIPAddresses()
{
	changeIPAddresses("knownIpAddresses", "/Settings/Fronius/KnownIPAddresses");
}

void DbusSettingsBridgeTest::changeKnownIPAddressesRemote()
{
	changeIPAddressesRemote("knownIpAddresses", "/Settings/Fronius/KnownIPAddresses");
}

void DbusSettingsBridgeTest::changeIPAddresses(const char *property,
											   const char *path)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	processEvents(10, 10);
	// Check default value from DBus
	QVERIFY(settings.property(property).value<QList<QHostAddress> >().isEmpty());
	settingsClient.resetChangedPaths();
	QList<QHostAddress> newValues;
	newValues.append(QHostAddress("192.168.1.3"));
	newValues.append(QHostAddress("192.168.1.7"));
	settings.setProperty(property, QVariant::fromValue(newValues));
	processEvents(10, 10);
	// Check new value
	QVariant qv = settingsClient.getValue(path);
	QCOMPARE(qv.toString(), QString("192.168.1.3,192.168.1.7"));
	// Check DBus signal
	const QList<QString> &cp = settingsClient.changedPaths();
	QCOMPARE(cp.size(), 1);
	QCOMPARE(cp.first(), QString(path));
}

void DbusSettingsBridgeTest::changeIPAddressesRemote(const char *property,
													 const char *path)
{
	DBusSettings settingsClient;
	DBusSettingsBridge::addDBusObjects();
	Settings settings;
	DBusSettingsBridge bridge(&settings, 0);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	processEvents(10, 10);
	QVERIFY(settings.property(property).value<QList<QHostAddress> >().isEmpty());
	QSignalSpy spy(&settings, SIGNAL(ipAddressesChanged()));
	// Set new value on DBus
	settingsClient.setValue(path, "192.168.1.3,192.168.1.7");
	// Allow value form DBus to trickle to our settings object
	processEvents(10, 10);
	QList<QHostAddress> addr = settings.property(property).
			value<QList<QHostAddress> >();
	QList<QHostAddress> expected;
	expected.append(QHostAddress("192.168.1.3"));
	expected.append(QHostAddress("192.168.1.7"));
	// Check DBus signal
	QCOMPARE(addr, expected);
}

void DbusSettingsBridgeTest::processEvents(int sleep, int count)
{
	for (int i=0; i<count; ++i) {
		QCoreApplication::processEvents();
		QTest::qWait(sleep);
	}
}

void DbusSettingsBridgeTest::checkValue(const QVariant &actual,
										const QVariant &expected)
{
	QCOMPARE(actual.isValid(), expected.isValid());
	QCOMPARE(actual, expected);
}

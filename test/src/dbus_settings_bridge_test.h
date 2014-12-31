#ifndef DBUS_SETTINGS_BRIDGE_TEST_H
#define DBUS_SETTINGS_BRIDGE_TEST_H

#include <gtest/gtest.h>
#include <QScopedPointer>

class DBusSettings;
class DBusSettingsBridge;
class QVariant;
class Settings;

class DBusSettingsBridgeTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	void changeIPAddresses(const char *property, const char *path);

	void changeIPAddressesRemote(const char *property, const char *path);

	QScopedPointer<Settings> mSettings;
	QScopedPointer<DBusSettingsBridge> mBridge;
	QScopedPointer<DBusSettings> mSettingsClient;
};

#endif // DBUS_SETTINGS_BRIDGE_TEST_H


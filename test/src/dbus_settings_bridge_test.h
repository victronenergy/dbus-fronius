#ifndef DBUS_SETTINGS_BRIDGE_TEST_H
#define DBUS_SETTINGS_BRIDGE_TEST_H

#include <gtest/gtest.h>
#include <QScopedPointer>

class DBusSettingsBridge;
class QVariant;
class Settings;
class VeQItemProducer;

class DBusSettingsBridgeTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	void changeIPAddresses(const char *property, const char *path);

	void changeIPAddressesRemote(const char *property, const char *path);

	static QVariant getValue(const QString &path);

	static void setValue(const QString &path, const QVariant &v);

	QScopedPointer<VeQItemProducer> mItemProducer;
	QScopedPointer<Settings> mSettings;
	QScopedPointer<DBusSettingsBridge> mBridge;
};

#endif // DBUS_SETTINGS_BRIDGE_TEST_H


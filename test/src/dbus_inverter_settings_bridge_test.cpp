#include "dbus_inverter_settings_bridge.h"
#include "dbus_inverter_settings_bridge_test.h"
#include "dbus_settings.h"
#include "inverter_settings.h"
#include "test_helper.h"

TEST_F(DBusInverterSettingsBridgeTest, changePosition)
{
	// Check default value from DBus
	EXPECT_EQ(InverterSettings::Input1, mSettings->position());
	mSettingsClient->resetChangedPaths();
	mSettings->setPosition(InverterSettings::Input2);
	qWait(100);
	// Check new value
	QVariant position = mSettingsClient->getValue("/Settings/Fronius/Inverters/I475b/Position");
	EXPECT_EQ(QVariant(2), position);
	// Check DBus signal
	const QList<QString> &cp = mSettingsClient->changedPaths();
	ASSERT_EQ(1, cp.size());
	EXPECT_EQ(QString("/Settings/Fronius/Inverters/I475b/Position"), cp.first());
}

TEST_F(DBusInverterSettingsBridgeTest, changePositionRemote)
{
	EXPECT_EQ(InverterSettings::Input1, mSettings->position());
	// Set new value on DBus
	mSettingsClient->setValue("/Settings/Fronius/Inverters/I475b/Position", 1);
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	EXPECT_EQ(InverterSettings::Output, mSettings->position());
}

TEST_F(DBusInverterSettingsBridgeTest, changePhase)
{
	// Check default value from DBus
	EXPECT_EQ(InverterSettings::AllPhases, mSettings->phase());
	mSettingsClient->resetChangedPaths();
	mSettings->setPhase(InverterSettings::L2);
	qWait(100);
	// Check new value
	QVariant phase = mSettingsClient->getValue("/Settings/Fronius/Inverters/I475b/Phase");
	EXPECT_EQ(QVariant(2), phase);
	// Check DBus signal
	const QList<QString> &cp = mSettingsClient->changedPaths();
	ASSERT_EQ(1, cp.size());
	EXPECT_EQ(QString("/Settings/Fronius/Inverters/I475b/Phase"), cp.first());
}

TEST_F(DBusInverterSettingsBridgeTest, changePhaseRemote)
{
	EXPECT_EQ(InverterSettings::AllPhases, mSettings->phase());
	// Set new value on DBus
	mSettingsClient->setValue("/Settings/Fronius/Inverters/I475b/Phase", 2);
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	EXPECT_EQ(InverterSettings::L2, mSettings->phase());
}

void DBusInverterSettingsBridgeTest::SetUpTestCase()
{
	qRegisterMetaType<InverterSettings::Position>("Position");
	qRegisterMetaType<InverterSettings::Phase>("Phase");
}

void DBusInverterSettingsBridgeTest::SetUp()
{
	mSettingsClient.reset(new DBusSettings());
	mSettings.reset(new InverterSettings("475b"));
	mBridge.reset(new DBusInverterSettingsBridge(mSettings.data(), 0));
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	qWait(300);
}

void DBusInverterSettingsBridgeTest::TearDown()
{
	mSettings.reset();
	mBridge.reset();
	mSettingsClient.reset();
}

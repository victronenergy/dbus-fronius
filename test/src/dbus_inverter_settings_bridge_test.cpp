#include "dbus_inverter_settings_bridge.h"
#include "dbus_inverter_settings_bridge_test.h"
#include "dbus_settings.h"
#include "inverter_settings.h"
#include "test_helper.h"

static const QString BasePath = "/Settings/Fronius/Inverters/I224_475b";

TEST_F(DBusInverterSettingsBridgeTest, changePosition)
{
	// Check default value from DBus
	EXPECT_EQ(Input1, mSettings->position());
	mSettingsClient->resetChangedPaths();
	mSettings->setPosition(Input2);
	qWait(100);
	// Check new value
	QVariant position = mSettingsClient->getValue(BasePath + "/Position");
	EXPECT_EQ(QVariant(2), position);
	// Check DBus signal
	const QList<QString> &cp = mSettingsClient->changedPaths();
	ASSERT_EQ(1, cp.size());
	EXPECT_EQ(QString(BasePath + "/Position"), cp.first());
}

TEST_F(DBusInverterSettingsBridgeTest, changePositionRemote)
{
	EXPECT_EQ(Input1, mSettings->position());
	// Set new value on DBus
	mSettingsClient->setValue(BasePath + "/Position", 1);
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	EXPECT_EQ(Output, mSettings->position());
}

TEST_F(DBusInverterSettingsBridgeTest, changePhase)
{
	// Check default value from DBus
	EXPECT_EQ(ThreePhases, mSettings->phase());
	mSettingsClient->resetChangedPaths();
	mSettings->setPhase(PhaseL2);
	qWait(100);
	// Check new value
	QVariant phase = mSettingsClient->getValue(BasePath + "/Phase");
	EXPECT_EQ(QVariant(2), phase);
	// Check DBus signal
	const QList<QString> &cp = mSettingsClient->changedPaths();
	ASSERT_EQ(1, cp.size());
	EXPECT_EQ(QString(BasePath + "/Phase"), cp.first());
}

TEST_F(DBusInverterSettingsBridgeTest, changePhaseRemote)
{
	EXPECT_EQ(ThreePhases, mSettings->phase());
	// Set new value on DBus
	mSettingsClient->setValue(BasePath + "/Phase", 2);
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	EXPECT_EQ(PhaseL2, mSettings->phase());
}

TEST_F(DBusInverterSettingsBridgeTest, changeL1Energy)
{
	checkStoredEnergy(PhaseL1);
}

TEST_F(DBusInverterSettingsBridgeTest, changeL1EnergyRemote)
{
	checkStoredEnergyRemote(PhaseL1);
}

TEST_F(DBusInverterSettingsBridgeTest, changeL2Energy)
{
	checkStoredEnergy(PhaseL2);
}

TEST_F(DBusInverterSettingsBridgeTest, changeL2EnergyRemote)
{
	checkStoredEnergyRemote(PhaseL2);
}

TEST_F(DBusInverterSettingsBridgeTest, changeL3Energy)
{
	checkStoredEnergy(PhaseL3);
}

TEST_F(DBusInverterSettingsBridgeTest, changeL3EnergyRemote)
{
	checkStoredEnergyRemote(PhaseL3);
}

void DBusInverterSettingsBridgeTest::SetUpTestCase()
{
	qRegisterMetaType<InverterPosition>("Position");
	qRegisterMetaType<InverterPhase>("Phase");
}

void DBusInverterSettingsBridgeTest::SetUp()
{
	mSettingsClient.reset(new DBusSettings());
	mSettings.reset(new InverterSettings(224, "475b"));
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

void DBusInverterSettingsBridgeTest::checkStoredEnergy(InverterPhase phase)
{
	EXPECT_EQ(0.0, mSettings->getEnergy(phase));
	mSettingsClient->resetChangedPaths();
	mSettings->setEnergy(phase, 37.4);
	qWait(100);
	// Check new value
	QString path = QString(BasePath + "/L%1Energy").
				   arg(static_cast<int>(phase));
	QVariant e = mSettingsClient->getValue(path);
	EXPECT_EQ(QVariant(37.4), e);
	// Check DBus signal
	const QStringList &cp = mSettingsClient->changedPaths();
	ASSERT_EQ(1, cp.size());
	EXPECT_EQ(path, cp.first());
}

void DBusInverterSettingsBridgeTest::checkStoredEnergyRemote(
		InverterPhase phase)
{
	EXPECT_EQ(ThreePhases, mSettings->getEnergy(phase));
	// Set new value on DBus
	QString path = QString(BasePath + "/L%1Energy").
				   arg(static_cast<int>(phase));
	mSettingsClient->setValue(path, 1234.8);
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	EXPECT_EQ(1234.8, mSettings->getEnergy(phase));
}

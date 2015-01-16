#ifndef DBUSINVERTERSETTINGSBRIDGETEST_H
#define DBUSINVERTERSETTINGSBRIDGETEST_H

#include <gtest/gtest.h>
#include <QScopedPointer>
#include "defines.h"

class DBusSettings;
class DBusInverterSettingsBridge;
class InverterSettings;

class DBusInverterSettingsBridgeTest : public testing::Test
{
protected:
	static void SetUpTestCase();

	virtual void SetUp();

	virtual void TearDown();

	void checkStoredEnergy(InverterPhase phase);

	void checkStoredEnergyRemote(InverterPhase phase);

	QScopedPointer<InverterSettings> mSettings;
	QScopedPointer<DBusInverterSettingsBridge> mBridge;
	QScopedPointer<DBusSettings> mSettingsClient;
};

#endif // DBUSINVERTERSETTINGSBRIDGETEST_H

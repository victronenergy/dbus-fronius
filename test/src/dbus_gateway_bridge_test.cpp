#include "dbus_gateway_bridge.h"
#include "dbus_gateway_bridge_test.h"
#include "dbus_service_observer.h"
#include "inverter_gateway.h"
#include "settings.h"
#include "test_helper.h"

TEST_F(DBusGatewayBridgeTest, changeAutoDetect)
{
	// Check default value from DBus
	EXPECT_FALSE(mGateway->autoDetect());
	mDBusObserver->resetChangedPaths();
	mGateway->setAutoDetect(true);
	qWait(100);
	// Check new value
	QVariant autoDetect = mDBusObserver->getValue("/AutoDetect");
	EXPECT_TRUE(autoDetect.isValid());
	EXPECT_EQ(1, autoDetect.toInt());
	// Check DBus signal
	const QStringList &cp = mDBusObserver->changedPaths();
	EXPECT_TRUE(cp.contains("/AutoDetect"));
}

TEST_F(DBusGatewayBridgeTest, changeAutoDetectRemote)
{
	EXPECT_FALSE(mGateway->autoDetect());
	// Set new value on DBus
	mDBusObserver->setValue("/AutoDetect", 1);
	// Allow value form DBus to trickle to our settings object
	qWait(100);
	EXPECT_TRUE(mGateway->autoDetect());
}

void DBusGatewayBridgeTest::SetUp()
{
	mGateway.reset(new InverterGateway(new Settings()));
	mBridge.reset(new DBusGatewayBridge(mGateway.data()));
	mDBusObserver.reset(new DBusServiceObserver("com.victronenergy.fronius"));
	mDBusObserver->setLogChangedPaths(true);
	// Wait for DBusSettingsBridge to fill settings object with default values
	// specified by DBusSettingsBridge::addDBusObjects
	qWait(300);
}

void DBusGatewayBridgeTest::TearDown()
{
	mDBusObserver.reset();
	mBridge.reset();
	mGateway.reset();
}

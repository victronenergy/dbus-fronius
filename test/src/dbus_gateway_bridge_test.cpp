#include "dbus_bridge.h"
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
	mGateway->setAutoDetect(true);

	QVariant autoDetect = getValue("/AutoDetect");
	EXPECT_TRUE(autoDetect.isValid());
	EXPECT_EQ(1, autoDetect.toInt());

	mGateway->setAutoDetect(false);
	autoDetect = getValue("/AutoDetect");
	EXPECT_TRUE(autoDetect.isValid());
	EXPECT_EQ(0, autoDetect.toInt());
}

TEST_F(DBusGatewayBridgeTest, changeAutoDetectRemote)
{
	EXPECT_FALSE(mGateway->autoDetect());
	// Set new value on DBus
	setValue("/AutoDetect", 1);
	EXPECT_TRUE(mGateway->autoDetect());
}

void DBusGatewayBridgeTest::SetUp()
{
	mItemProducer.reset(new BridgeItemProducer(VeQItems::getRoot(), "pub"));
	mItemProducer->open();
	mGateway.reset(new InverterGateway(new Settings()));
	mBridge.reset(new DBusGatewayBridge(mGateway.data()));
}

void DBusGatewayBridgeTest::TearDown()
{
	mBridge.reset();
	mGateway.reset();
	mItemProducer.reset();
}

QVariant DBusGatewayBridgeTest::getValue(const QString &path)
{
	VeQItem *item = VeQItems::getRoot()->itemGetOrCreate("pub/com.victronenergy.fronius" + path, true);
	return item->getValue();
}

void DBusGatewayBridgeTest::setValue(const QString &path, const QVariant &v)
{
	VeQItem *item = VeQItems::getRoot()->itemGetOrCreate("pub/com.victronenergy.fronius" + path, true);
	item->setValue(v);
}

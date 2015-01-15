#ifndef DBUSGATEWAYBRIDGETEST_H
#define DBUSGATEWAYBRIDGETEST_H

#include <gtest/gtest.h>
#include <QScopedPointer>

class DBusServiceObserver;
class DBusGatewayBridge;
class InverterGateway;

class DBusGatewayBridgeTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	QScopedPointer<InverterGateway> mGateway;
	QScopedPointer<DBusGatewayBridge> mBridge;
	QScopedPointer<DBusServiceObserver> mDBusObserver;
};

#endif // DBUSGATEWAYBRIDGETEST_H

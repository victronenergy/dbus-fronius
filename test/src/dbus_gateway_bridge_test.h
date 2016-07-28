#ifndef DBUSGATEWAYBRIDGETEST_H
#define DBUSGATEWAYBRIDGETEST_H

#include <gtest/gtest.h>
#include <QScopedPointer>
#include <QVariant>

class DBusGatewayBridge;
class InverterGateway;
class VeQItemProducer;

class DBusGatewayBridgeTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	QVariant getValue(const QString &path);

	void setValue(const QString &path, const QVariant &v);

	QScopedPointer<InverterGateway> mGateway;
	QScopedPointer<DBusGatewayBridge> mBridge;
	QScopedPointer<VeQItemProducer> mItemProducer;
};

#endif // DBUSGATEWAYBRIDGETEST_H

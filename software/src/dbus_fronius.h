#ifndef DBUS_TEST2_H
#define DBUS_TEST2_H

#include "gateway_interface.h"
#include "ve_service.h"

class AbstractDetector;
class InverterGateway;
class InverterMediator;
class Settings;
class VeQItem;

struct DeviceInfo;

class DBusFronius : public VeService, public GatewayInterface
{
	Q_OBJECT
public:
	DBusFronius(QObject *parent = 0);

	virtual void startDetection();

	virtual int handleSetValue(VeQItem *item, const QVariant &variant);

private slots:
	void onSettingsInitialized();

	void onInverterFound(const DeviceInfo &deviceInfo);

	void onScanProgressChanged();

	void onAutoDetectChanged();

private:
	void addGateway(AbstractDetector *detector);

	QList<InverterMediator *> mMediators;
	QList<InverterGateway *> mGateways;
	Settings *mSettings;
	VeQItem *mAutoDetect;
	VeQItem *mScanProgress;
};

#endif // DBUS_TEST2_H

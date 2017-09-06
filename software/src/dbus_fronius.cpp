#include <QsLog.h>
#include "dbus_fronius.h"
#include "defines.h"
#include "inverter_gateway.h"
#include "inverter_mediator.h"
#include "settings.h"
#include "solar_api_detector.h"
#include "sunspec_detector.h"
#include "ve_qitem_init_monitor.h"

DBusFronius::DBusFronius(QObject *parent) :
	VeService(VeQItems::getRoot()->itemGetOrCreate("pub/com.victronenergy.fronius"), parent),
	mSettings(new Settings(VeQItems::getRoot()->itemGetOrCreate("sub/com.victronenergy.settings/Settings/Fronius", false), this)),
	mAutoDetect(createItem("AutoDetect")),
	mScanProgress(createItem("ScanProgress"))
{
	VeQItemInitMonitor::monitor(mSettings->root(), this, SLOT(onSettingsInitialized()));
	registerService();
}

void DBusFronius::startDetection()
{
	foreach (InverterGateway *gateway, mGateways)
		gateway->startDetection();
}

int DBusFronius::handleSetValue(VeQItem *item, const QVariant &variant)
{
	if (item == mAutoDetect) {
		bool autoDetect = variant.toBool();
		foreach (InverterGateway *gateway, mGateways)
			gateway->setAutoDetect(autoDetect);
		return 0;
	}
	return VeService::handleSetValue(item, variant);
}

void DBusFronius::onSettingsInitialized()
{
	Q_ASSERT(mGateways.isEmpty());
	addGateway(new SolarApiDetector(mSettings, this));
	addGateway(new SunspecDetector(126, this));
	onScanProgressChanged();
	onAutoDetectChanged();
	startDetection();
}

void DBusFronius::onInverterFound(const DeviceInfo &deviceInfo)
{
	foreach (InverterMediator *m, mMediators) {
		if (m->processNewInverter(deviceInfo))
			return;
	}
	InverterMediator *m = new InverterMediator(deviceInfo, this, mSettings, this);
	mMediators.append(m);
}

void DBusFronius::onScanProgressChanged()
{
	int progress = 0;
	if (!mGateways.isEmpty()) {
		foreach (InverterGateway *gateway, mGateways)
			progress += gateway->scanProgress();
		progress /= mGateways.size();
	}
	produceDouble(mScanProgress, progress, 0, "%");
}

void DBusFronius::onAutoDetectChanged()
{
	foreach (InverterGateway *gateway, mGateways)
	{
		if (gateway->autoDetect()) {
			produceValue(mAutoDetect, 1, "Busy");
			return;
		}
	}
	produceValue(mAutoDetect, 0, "Idle");
}

void DBusFronius::addGateway(AbstractDetector *detector)
{
	InverterGateway *gateway = new InverterGateway(detector, mSettings, this);
	mGateways.append(gateway);
	connect(gateway, SIGNAL(inverterFound(DeviceInfo)), this, SLOT(onInverterFound(DeviceInfo)));
	connect(gateway, SIGNAL(autoDetectChanged()), this, SLOT(onAutoDetectChanged()));
	connect(gateway, SIGNAL(scanProgressChanged()), this, SLOT(onScanProgressChanged()));
}

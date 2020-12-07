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
	mScanProgress(createItem("ScanProgress")),
	mGateway(new InverterGateway(mSettings, this))
{
	connect(mGateway, SIGNAL(inverterFound(DeviceInfo)), this, SLOT(onInverterFound(DeviceInfo)));
	connect(mGateway, SIGNAL(autoDetectChanged()), this, SLOT(onAutoDetectChanged()));
	connect(mGateway, SIGNAL(scanProgressChanged()), this, SLOT(onScanProgressChanged()));

	VeQItemInitMonitor::monitor(mSettings->root(), this, SLOT(onSettingsInitialized()));
	registerService();
}

void DBusFronius::startDetection()
{
	mGateway->startDetection();
}

int DBusFronius::handleSetValue(VeQItem *item, const QVariant &variant)
{
	if (item == mAutoDetect) {
		if (variant.toBool())
			mGateway->fullScan(); // This causes /AutoDetect to change to 1
		return 0;
	}
	return VeService::handleSetValue(item, variant);
}

void DBusFronius::onSettingsInitialized()
{
	mGateway->addDetector(new SolarApiDetector(mSettings, this));
	mGateway->addDetector(new SunspecDetector(126, this));
	mGateway->initializeSettings();
	onScanProgressChanged();
	onAutoDetectChanged();
	startDetection();
}

void DBusFronius::onInverterFound(const DeviceInfo &deviceInfo)
{
	// Filter out inverters with storage, we don't want to support those.
	if (qIsFinite(deviceInfo.storageCapacity) && (deviceInfo.storageCapacity > 0)) {
		QString location = QString("%1@%2:%3").arg(deviceInfo.uniqueId).
			arg(deviceInfo.hostName).
			arg(deviceInfo.networkId);
		QLOG_INFO() << "Skipping storage inverter" << deviceInfo.productName << "@" << location;
		return;
	}


	// Check if any of our mediators know about this inverter already
	foreach (InverterMediator *m, mMediators) {
		if (m->processNewInverter(deviceInfo))
			return;
	}

	// Allocate a new one
	InverterMediator *m = new InverterMediator(deviceInfo, this, mSettings, this);
	mMediators.append(m);
}

void DBusFronius::onScanProgressChanged()
{
	produceDouble(mScanProgress, mGateway->scanProgress(), 0, "%");
}

void DBusFronius::onAutoDetectChanged()
{
	if (mGateway->autoDetect()) {
		produceValue(mAutoDetect, 1, "Busy");
		return;
	}
	produceValue(mAutoDetect, 0, "Idle");
}

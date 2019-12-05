#include <QsLog.h>
#include "defines.h"
#include "inverter.h"
#include "fronius_inverter.h"
#include "gateway_interface.h"
#include "inverter_mediator.h"
#include "sunspec_updater.h"
#include "inverter_settings.h"
#include "solar_api_updater.h"
#include "settings.h"
#include "ve_qitem_init_monitor.h"

InverterMediator::InverterMediator(const DeviceInfo &device, GatewayInterface *gateway,
								   Settings *settings, QObject *parent):
	QObject(parent),
	mDeviceInfo(device),
	mInverter(0),
	mGateway(gateway),
	mSettings(settings)
{
	QString settingsPath = QString("Inverters/%1").arg(
		Settings::createInverterId(device.uniqueId));
	VeQItem *settingsRoot = settings->root()->itemGetOrCreate(settingsPath, false);
	mInverterSettings = new InverterSettings(settingsRoot, this);
	VeQItemInitMonitor::monitor(mInverterSettings->root(), this, SLOT(onSettingsInitialized()));
}

bool InverterMediator::processNewInverter(const DeviceInfo &deviceInfo)
{
	if (mDeviceInfo.uniqueId != deviceInfo.uniqueId) {
		if (mInverter != 0 &&
			mInverter->hostName() == deviceInfo.hostName &&
			mInverter->deviceInfo().networkId == deviceInfo.networkId) {
			QLOG_INFO() << "Another inverter found @"
						<< deviceInfo.hostName
						<< ':' << deviceInfo.networkId
						<< "closing down" << deviceInfo.uniqueId;
			// So we found an inverter whose communication settings matches ours.
			// We can only assume this inverter is no longer available there, so
			// we give up and hope it will return somewhere else.
			delete mInverter;
			mInverter = 0;
		}
		return false;
	}
	if (mInverter != 0 && mDeviceInfo.retrievalMode != deviceInfo.retrievalMode) {
		QLOG_INFO() << "Inverter retrieval mode has changed @" << mInverter->location();
		delete mInverter;
		mInverter = 0;
	}
	mDeviceInfo = deviceInfo;
	if (mInverter != 0) {
		if (mInverter->hostName() != deviceInfo.hostName ||
			mInverter->port() != deviceInfo.port) {
			mInverter->setHostName(deviceInfo.hostName);
			mInverter->setPort(deviceInfo.port);
			QLOG_INFO() << "Updated connection settings:" << mInverter->location();
		}
		return true;
	}
	if (!mInverterSettings->isActive())
		return true;
	mInverter = createInverter();
	if (mInverter) {
		QLOG_INFO() << "Inverter reactivated:" << mInverter->location();
		startAcquisition();
		onSettingsCustomNameChanged();
	}
	return true;
}

void InverterMediator::onSettingsInitialized()
{
	// Connect the signals now that the settings are up
	connect(mInverterSettings, SIGNAL(isActiveChanged()), this, SLOT(onIsActivatedChanged()));
	connect(mInverterSettings, SIGNAL(positionChanged()), this, SLOT(onPositionChanged()));
	connect(mInverterSettings, SIGNAL(customNameChanged()), this, SLOT(onSettingsCustomNameChanged()));

	if (!mInverterSettings->isActive()) {
		delete mInverter;
		mInverter = 0;
		return;
	}

	/* Migration, if this is a multi-phase inverter, make sure localsettings
	 * reflect it. */
	if (mDeviceInfo.phaseCount > 1 && mInverterSettings->phase() != MultiPhase) {
		mInverterSettings->setPhase(MultiPhase);
	}

	/* Persist the phase count to settings so the GUI has access to it. */
	mInverterSettings->setPhaseCount(mDeviceInfo.phaseCount);

	Q_ASSERT(mInverter == 0);
	mInverter = createInverter();
	if (mInverter) {
		QLOG_INFO() << "New inverter:" << mInverter->location();
		startAcquisition();
		onSettingsCustomNameChanged();
	}
}

void InverterMediator::onIsActivatedChanged()
{
	if (mInverterSettings->isActive()) {
		mGateway->startDetection();
	} else {
		if (mInverter == 0)
			return;
		QLOG_INFO() << "Inverter deactivated:" << mInverter->location();
		delete mInverter;
		mInverter = 0;
	}
}

void InverterMediator::onConnectionLost()
{
	QLOG_WARN() << "Lost connection with: " << mInverter->location();
	// Start device scan, maybe the IP address of the data card has changed.
	mGateway->startDetection();
	// Do not delete the inverter here because right now a function within The updater is emitting
	// the isConnectedChanged signal. Deleting the inverter will also delete the updater while a
	// function in the class is still on the stack.
	mInverter->deleteLater();
	mInverter = 0;
}

void InverterMediator::onInverterModelChanged()
{
	QLOG_WARN() << "Config change in: " << mInverter->location();
	// Start device scan, which will force a config reread.
	mGateway->startDetection();
	// Do not delete the inverter here because right now a function within The updater is emitting
	// the isConnectedChanged signal. Deleting the inverter will also delete the updater while a
	// function in the class is still on the stack.
	mInverter->deleteLater();
	mInverter = 0;
}

void InverterMediator::onPositionChanged()
{
	if (mInverter == 0)
		return;
	mInverter->setPosition(mInverterSettings->position());
}

void InverterMediator::onSettingsCustomNameChanged()
{
	if (mInverter == 0)
		return;
	mInverter->setCustomName(mInverterSettings->customName());
}

void InverterMediator::onInverterCustomNameChanged()
{
	if (mInverter == 0)
		return;
	mInverterSettings->setCustomName(mInverter->customName());
}

void InverterMediator::startAcquisition()
{
	Q_ASSERT(mInverter != 0);
	mInverter->setPosition(mInverterSettings->position());
	if (mDeviceInfo.retrievalMode == ProtocolFroniusSolarApi) {
		SolarApiUpdater *updater = new SolarApiUpdater(mInverter, mInverterSettings, mInverter);
		connect(updater, SIGNAL(connectionLost()), this, SLOT(onConnectionLost()));
	} else {
		SunspecUpdater *updater = new SunspecUpdater(mInverter, mInverterSettings, mInverter);
		connect(updater, SIGNAL(connectionLost()), this, SLOT(onConnectionLost()));
		connect(updater, SIGNAL(inverterModelChanged()), this, SLOT(onInverterModelChanged()));
	}
}

Inverter *InverterMediator::createInverter()
{
	int deviceInstance = mSettings->registerInverter(mDeviceInfo.uniqueId);
	if (deviceInstance < 0)
		return 0;

	QString path = QString("pub/com.victronenergy.pvinverter.pv_%1").arg(mDeviceInfo.uniqueId);
	VeQItem *root = VeQItems::getRoot()->itemGetOrCreate(path, false);
	Inverter *inverter;
	if (mDeviceInfo.deviceType != 0) {
		inverter = new FroniusInverter(root, mDeviceInfo, deviceInstance, this);
	} else {
		inverter = new Inverter(root, mDeviceInfo, deviceInstance, this);
	}
	connect(inverter, SIGNAL(customNameChanged()), this, SLOT(onInverterCustomNameChanged()));
	onPositionChanged();
	onSettingsCustomNameChanged();
	return inverter;
}

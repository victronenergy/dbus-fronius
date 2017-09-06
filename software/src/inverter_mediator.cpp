#include <QsLog.h>
#include "defines.h"
#include "inverter.h"
#include "gateway_interface.h"
#include "inverter_mediator.h"
#include "inverter_modbus_updater.h"
#include "inverter_settings.h"
#include "inverter_updater.h"
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
	connect(mInverterSettings, SIGNAL(isActiveChanged()), this, SLOT(onIsActivatedChanged()));
	connect(mInverterSettings, SIGNAL(positionChanged()), this, SLOT(onPositionChanged()));
	connect(mInverterSettings, SIGNAL(customNameChanged()), this, SLOT(onSettingsCustomNameChanged()));
	VeQItemInitMonitor::monitor(mInverterSettings->root(), this, SLOT(onSettingsInitialized()));
}

bool InverterMediator::processNewInverter(const DeviceInfo &deviceInfo)
{
	if (mDeviceInfo.uniqueId != deviceInfo.uniqueId) {
		if (mInverter != 0 &&
			mInverter->hostName() == deviceInfo.hostName &&
			mInverter->id() == deviceInfo.networkId) {
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
			QLOG_INFO() << "Updated connection settings:"
						<< deviceInfo.uniqueId << "@" << deviceInfo.hostName
						<< ':' << deviceInfo.networkId;
		}
		return true;
	}
	if (!mInverterSettings->isActive())
		return true;
	mInverter = createInverter();
	QLOG_INFO() << "Inverter reactivated:"
				<< mInverter->uniqueId() << "@" << mInverter->hostName()
				<< ':' << mInverter->id();
	startAcquisition();
	onSettingsCustomNameChanged();
	return true;
}

void InverterMediator::onSettingsInitialized()
{
	if (!mInverterSettings->isActive()) {
		delete mInverter;
		mInverter = 0;
		return;
	}
	Q_ASSERT(mInverter == 0);
	mInverter = createInverter();
	QLOG_INFO() << "New inverter:"
				<< mInverter->uniqueId() << "@" << mInverter->hostName()
				<< ':' << mInverter->id();
	startAcquisition();
	onSettingsCustomNameChanged();
}

void InverterMediator::onIsActivatedChanged()
{
	if (mInverterSettings->isActive()) {
		mGateway->startDetection();
	} else {
		if (mInverter == 0)
			return;
		QLOG_INFO() << "Inverter deactivated:"
					<< mInverter->uniqueId()
					<< "@" << mInverter->hostName()
					<< ':' << mInverter->id();
		delete mInverter;
		mInverter = 0;
	}
}

void InverterMediator::onConnectionLost()
{
	QLOG_WARN() << "Lost connection with: " << mInverter->uniqueId()
				<< "@ " << mInverter->hostName() << ':' << mInverter->port();
	// Start device scan, maybe the IP address of the data card has changed.
	mGateway->startDetection();
	// Do not delete the inverter here because right now a function within
	// InverterUpdater is emitting the isConnectedChanged signal. Deleting
	// the inverter will also delete the InverterUpdater
	mInverter->deleteLater();
	mInverter = 0;
}

void InverterMediator::onInverterModelChanged()
{
	QLOG_WARN() << "Config change in: " << mInverter->uniqueId()
				<< "@ " << mInverter->hostName() << ':' << mInverter->port();
	// Start device scan, which will force a config reread.
	mGateway->startDetection();
	// Do not delete the inverter here because right now a function within
	// InverterUpdater is emitting the isConnectedChanged signal. Deleting
	// the inverter will also delete the InverterUpdater
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
	QString name = mInverterSettings->customName();
	if (name.isEmpty())
		name = mInverter->productName();
	mInverter->setCustomName(name);
}

void InverterMediator::onInverterCustomNameChanged()
{
	if (mInverter == 0)
		return;
	QString name = mInverter->customName();
	if (name == mInverter->productName())
		name.clear();
	mInverterSettings->setCustomName(name);
}

void InverterMediator::startAcquisition()
{
	Q_ASSERT(mInverter != 0);
	mInverter->setPosition(mInverterSettings->position());
	if (mDeviceInfo.retrievalMode == ProtocolFroniusSolarApi) {
		InverterUpdater *updater = new InverterUpdater(mInverter, mInverterSettings, mInverter);
		connect(updater, SIGNAL(connectionLost()), this, SLOT(onConnectionLost()));
	} else {
		InverterModbusUpdater *updater = new InverterModbusUpdater(mInverter, mInverterSettings, mInverter);
		connect(updater, SIGNAL(connectionLost()), this, SLOT(onConnectionLost()));
		connect(updater, SIGNAL(inverterModelChanged()), this, SLOT(onInverterModelChanged()));
	}
}

Inverter *InverterMediator::createInverter()
{
	mSettings->registerInverter(mDeviceInfo.uniqueId);
	int deviceInstance = mSettings->getDeviceInstance(mDeviceInfo.uniqueId);
	QString path = QString("pub/com.victronenergy.pvinverter.fronius_%1").arg(mDeviceInfo.uniqueId);
	VeQItem *root = VeQItems::getRoot()->itemGetOrCreate(path, false);
	Inverter *inverter = new Inverter(root, mDeviceInfo, deviceInstance, this);
	connect(inverter, SIGNAL(customNameChanged()), this, SLOT(onInverterCustomNameChanged()));
	onPositionChanged();
	onSettingsCustomNameChanged();
	return inverter;
}

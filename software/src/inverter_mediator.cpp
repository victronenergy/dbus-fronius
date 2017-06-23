#include <QsLog.h>
#include "defines.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_mediator.h"
#include "inverter_modbus_updater.h"
#include "inverter_settings.h"
#include "inverter_updater.h"
#include "settings.h"
#include "ve_qitem_init_monitor.h"

InverterMediator::InverterMediator(const DeviceInfo &device, InverterGateway *gateway,
								   Settings *settings, QObject *parent):
	QObject(parent),
	mInverter(createInverter(device)),
	mGateway(gateway),
	mSettings(settings),
	mDeviceType(device.deviceType),
	mUniqueId(device.uniqueId)
{
	QString settingsPath = QString("Inverters/%1").arg(
		Settings::createInverterId(device.deviceType, device.uniqueId));
	VeQItem *settingsRoot = settings->root()->itemGetOrCreate(settingsPath, false);
	mInverterSettings = new InverterSettings(settingsRoot, this);
	connect(mInverterSettings, SIGNAL(isActiveChanged()), this, SLOT(onIsActivatedChanged()));
	connect(mInverterSettings, SIGNAL(positionChanged()), this, SLOT(onPositionChanged()));
	connect(mInverterSettings, SIGNAL(customNameChanged()), this, SLOT(onSettingsCustomNameChanged()));
	QLOG_INFO() << "New inverter:"
				<< mInverter->uniqueId() << "@" << mInverter->hostName()
				<< ':' << mInverter->id();
	VeQItemInitMonitor::monitor(mInverterSettings->root(), this, SLOT(onSettingsInitialized()));
}

bool InverterMediator::processNewInverter(const DeviceInfo &deviceInfo)
{
	if (mDeviceType != deviceInfo.deviceType || mUniqueId != deviceInfo.uniqueId) {
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
	mInverter = createInverter(deviceInfo);
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
	if (mInverter == 0)
		return;
	startAcquisition();
	onSettingsCustomNameChanged();
}

void InverterMediator::onIsActivatedChanged()
{
	if (mInverterSettings->isActive()) {
		mGateway->setAutoDetect(true);
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
	mGateway->setAutoDetect(true);
	// Do not delete the inverter here because right now a function within
	// InverterUpdater is emitting the isConnectedChanged signal. Deleting
	// the inverter will also delete the InverterUpdater
	mInverter->deleteLater();
	mInverter = 0;
}

void InverterMediator::onPositionChanged()
{
	mInverter->setPosition(mInverterSettings->position());
}

void InverterMediator::onSettingsCustomNameChanged()
{
	QString name = mInverterSettings->customName();
	if (name.isEmpty())
		name = mInverter->productName();
	mInverter->setCustomName(name);
}

void InverterMediator::onInverterCustomNameChanged()
{
	QString name = mInverter->customName();
	if (name == mInverter->productName())
		name.clear();
	mInverterSettings->setCustomName(name);
}

void InverterMediator::startAcquisition()
{
	mSettings->registerInverter(mInverter->deviceType(), mInverter->uniqueId());
	int deviceInstance = mSettings->getDeviceInstance(
							 mInverter->deviceType(), mInverter->uniqueId());
	if (deviceInstance != mInverter->deviceInstance()) {
		QLOG_INFO() << "Assigning device instance on" << mInverter->uniqueId()
					<< "from" << mInverter->deviceInstance()
					<< "to" << deviceInstance;
		mInverter->setDeviceInstance(deviceInstance);
	}
	if (mInverter->phaseCount() > 1) {
		mInverterSettings->setPhase(MultiPhase);
	} else if (mInverterSettings->phase() == MultiPhase) {
		QLOG_ERROR() << "Inverter is single phased, but settings report"
					 << "multiphase. Adjusting settings.";
		mInverterSettings->setPhase(PhaseL1);
	}
	mInverter->setPosition(mInverterSettings->position());
	InverterUpdater *updater = new InverterUpdater(mInverter, mInverterSettings, mInverter);
	connect(updater, SIGNAL(connectionLost()), this, SLOT(onConnectionLost()));
	new InverterModbusUpdater(mInverter, mInverter);
}

Inverter *InverterMediator::createInverter(const DeviceInfo &device)
{
	QString path = QString("pub/com.victronenergy.pvinverter.fronius_%1_%2").
		arg(device.deviceType).
		arg(device.uniqueId);
	VeQItem *root = VeQItems::getRoot()->itemGetOrCreate(path, false);
	Inverter *inverter = new Inverter(root, device, this);
	connect(inverter, SIGNAL(customNameChanged()), this, SLOT(onInverterCustomNameChanged()));
	return inverter;
}

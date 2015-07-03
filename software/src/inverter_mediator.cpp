#include <QsLog.h>
#include "dbus_inverter_bridge.h"
#include "dbus_inverter_settings_bridge.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_mediator.h"
#include "inverter_settings.h"
#include "inverter_updater.h"
#include "settings.h"

InverterMediator::InverterMediator(Inverter *inverter, InverterGateway *gateway,
								   Settings *settings, QObject *parent):
	QObject(parent),
	mInverter(inverter),
	mGateway(gateway),
	mSettings(settings)
{
	mInverterSettings = new InverterSettings(inverter->deviceType(),
											 inverter->uniqueId(),
											 this);
	mInverter->setParent(this);
	DBusInverterSettingsBridge *bridge =
		new DBusInverterSettingsBridge(mInverterSettings, mInverterSettings);
	connect(bridge, SIGNAL(initialized()),
			this, SLOT(onSettingsInitialized()));
	connect(mInverterSettings, SIGNAL(isActiveChanged()),
			this, SLOT(onIsActivatedChanged()));
	connect(mInverter, SIGNAL(isConnectedChanged()),
			this, SLOT(onIsConnectedChanged()));
	QLOG_INFO() << "New inverter:"
				<< inverter->uniqueId() << "@" << inverter->hostName()
				<< ':' << inverter->id();
}

bool InverterMediator::processNewInverter(Inverter *inverter)
{
	if (!inverterMatches(inverter))
		return false;
	if (mInverter != 0) {
		if (mInverter->hostName() != inverter->hostName() ||
			mInverter->port() != inverter->port()) {
			mInverter->setHostName(inverter->hostName());
			mInverter->setPort(inverter->port());
			QLOG_INFO() << "Updated connection settings:"
						<< inverter->uniqueId() << "@" << inverter->hostName()
						<< ':' << inverter->id();
		}
		return true;
	}
	if (!mInverterSettings->isActive())
		return true;
	inverter->setParent(this);
	connect(inverter, SIGNAL(isConnectedChanged()),
			this, SLOT(onIsConnectedChanged()));
	mInverter = inverter;
	QLOG_INFO() << "Inverter reactivated:"
				<< inverter->uniqueId() << "@" << inverter->hostName()
				<< ':' << inverter->id();
	startAcquisition();
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
}

void InverterMediator::onInverterInitialized()
{
	new DBusInverterBridge(mInverter, mInverterSettings, mInverter);
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

void InverterMediator::onIsConnectedChanged()
{
	Inverter *inverter = static_cast<Inverter *>(sender());
	if (inverter->isConnected())
		return;
	QLOG_WARN() << "Lost connection with: " << inverter->uniqueId()
				<< "@ " << inverter->hostName() << ':' << inverter->port();
	// Start device scan, maybe the IP address of the data card has changed.
	mGateway->setAutoDetect(true);
	// Do not delete the inverter here because right now a function within
	// InverterUpdater is emitting the isConnectedChanged signal. Deleting
	// the inverter will also delete the InverterUpdater
	mInverter->deleteLater();
	mInverter = 0;
}

bool InverterMediator::inverterMatches(Inverter *inverter)
{
	return
		mInverterSettings->deviceType() == inverter->deviceType() &&
		mInverterSettings->uniqueId() == inverter->uniqueId();
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
	InverterUpdater *iu = new InverterUpdater(mInverter, mInverterSettings, mInverter);
	connect(iu, SIGNAL(initialized()), this, SLOT(onInverterInitialized()));
}

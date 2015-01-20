#include <QsLog.h>
#include "dbus_fronius.h"
#include "dbus_gateway_bridge.h"
#include "dbus_inverter_bridge.h"
#include "dbus_inverter_settings_bridge.h"
#include "dbus_settings_bridge.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_settings.h"
#include "inverter_updater.h"
#include "settings.h"

DBusFronius::DBusFronius(QObject *parent) :
	QObject(parent),
	mSettings(new Settings(this)),
	mGateway(new InverterGateway(mSettings, this)),
	mSettingsBridge(new DBusSettingsBridge(mSettings, this)),
	mGatewayBridge(new DBusGatewayBridge(mGateway, this))
{
	// This enables us to retrieve values from QT properties via the
	// QObject::property function.
	qRegisterMetaType<QList<QHostAddress> >();
	qRegisterMetaType<QHostAddress>();
	qRegisterMetaType<InverterPosition>("Position");
	qRegisterMetaType<InverterPhase>("Phase");

	connect(mGateway, SIGNAL(inverterFound(Inverter *)),
			this, SLOT(onInverterFound(Inverter *)));
	connect(mSettingsBridge, SIGNAL(initialized()),
			this, SLOT(onSettingsInitialized()));
}

void DBusFronius::onSettingsInitialized()
{
	mGateway->startDetection();
}

void DBusFronius::onInverterFound(Inverter *inverter)
{
	Inverter *oldInverter = findInverter(inverter->deviceType(),
										 inverter->uniqueId());
	if (oldInverter != 0) {
		if (oldInverter->hostName() == inverter->hostName() &&
			oldInverter->port() == inverter->port()) {
			return;
		}
		mInverters.removeOne(oldInverter);
		delete oldInverter;
	}
	inverter->setParent(this);
	mInverters.append(inverter);
	// connect(inverter, SIGNAL(isConnectedChanged()),
	// 		this, SLOT(onIsConnectedChanged()));
	QLOG_INFO() << "New inverter:" << inverter->uniqueId()
				<< "@" << inverter->hostName() << ':' << inverter->id();
	InverterSettings *settings =
		new InverterSettings(inverter->deviceType(), inverter->uniqueId(),
							 inverter);
	// The custom name we set here will be use as default (and initial) value
	// of the D-Bus parameter. If the parameter already exists, this value
	// will be overwritten by the current value taken from the D-Bus.
	settings->setCustomName(inverter->productName());
	DBusInverterSettingsBridge *bridge =
		new DBusInverterSettingsBridge(settings, settings);
	connect(bridge, SIGNAL(initialized()),
			this, SLOT(onInverterSettingsInitialized()));
}

void DBusFronius::onInverterSettingsInitialized()
{
	DBusInverterSettingsBridge *bridge =
		static_cast<DBusInverterSettingsBridge *>(sender());
	InverterSettings *settings =
		static_cast<InverterSettings *>(bridge->parent());
	Inverter *inverter =
		static_cast<Inverter *>(settings->parent());
	if (inverter->supports3Phases()) {
		settings->setPhase(ThreePhases);
	} else if (settings->phase() == ThreePhases) {
		QLOG_ERROR() << "Inverter is single phased, but settings report"
					 << "multiphase. Adjusting settings.";
		settings->setPhase(PhaseL1);
	}
	InverterUpdater *iu = new InverterUpdater(inverter, settings, inverter);
	connect(iu, SIGNAL(initialized()), this, SLOT(onInverterInitialized()));
}

void DBusFronius::onInverterInitialized()
{
	InverterUpdater *ui = static_cast<InverterUpdater *>(sender());
	Inverter *inverter = ui->inverter();
	InverterSettings *settings = ui->settings();
	new DBusInverterBridge(inverter, settings, inverter);
}

void DBusFronius::onIsConnectedChanged()
{
	Inverter *inverter = static_cast<Inverter *>(sender());
	if (inverter->isConnected())
		return;
	QLOG_INFO() << "Lost connection with: " << inverter->hostName() << " - "
				<< inverter->id();
	// Do not delete the inverter here because right now a function within
	// Inverter is emitting the isConnectedChanged signal.
	inverter->deleteLater();
	foreach (Inverter *i, mInverters) {
		if (i == inverter) {
			i->deleteLater();
			mInverters.removeOne(i);
			break;
		}
	}
}

Inverter *DBusFronius::findInverter(int deviceType, const QString &uniqueId)
{
	foreach (Inverter *inverter, mInverters)
	{
		if (inverter->deviceType() == deviceType &&
			inverter->uniqueId() == uniqueId) {
			return inverter;
		}
	}
	return 0;
}

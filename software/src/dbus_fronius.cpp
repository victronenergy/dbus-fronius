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
	if (findUpdater(inverter->hostName(), inverter->id()) != 0)
		return;
	inverter->setParent(this);
	// connect(inverter, SIGNAL(isConnectedChanged()),
	// 		this, SLOT(onIsConnectedChanged()));
	QLOG_INFO() << "New inverter:" << inverter->hostName() << inverter->id();
	InverterSettings *settings =
		new InverterSettings(inverter->uniqueId(), inverter);
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
					 << "muliphase. Adjusting settings.";
		settings->setPhase(PhaseL1);
	}
	InverterUpdater *iu = new InverterUpdater(inverter, settings);
	connect(iu, SIGNAL(initialized()), this, SLOT(onInverterInitialized()));
	mUpdaters.append(iu);
}

void DBusFronius::onInverterInitialized()
{
	InverterUpdater *ui = static_cast<InverterUpdater *>(sender());
	Inverter *inverter = ui->inverter();
	InverterSettings *settings = ui->settings();

	// DBusInverterBridge will set inverter as its parent, so we have no
	// memory leak here.
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
	foreach (InverterUpdater *ui, mUpdaters) {
		if (ui->inverter() == inverter) {
			ui->deleteLater();
			mUpdaters.removeOne(ui);
			break;
		}
	}
}

InverterUpdater *DBusFronius::findUpdater(const QString &hostName,
										  const QString &deviceId)
{
	for (QList<InverterUpdater *>::iterator it = mUpdaters.begin();
		 it != mUpdaters.end();
		 ++it)
	{
		const Inverter *inverter = (*it)->inverter();
		if (inverter->hostName() == hostName && inverter->id() == deviceId)
			return *it;
	}
	return 0;
}

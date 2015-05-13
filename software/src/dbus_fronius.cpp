#include <QsLog.h>
#include "dbus_fronius.h"
#include "dbus_gateway_bridge.h"
#include "dbus_settings_bridge.h"
#include "defines.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_mediator.h"
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
	foreach (InverterMediator *m, mMediators) {
		if (m->processNewInverter(inverter))
			return;
	}
	InverterMediator *m = new InverterMediator(inverter, mGateway, mSettings, this);
	mMediators.append(m);
}

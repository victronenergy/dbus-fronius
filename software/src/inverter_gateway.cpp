#include <QTimer>
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_updater.h"
#include "settings.h"

static const int Port = 8080;
static const int MaxSimultaneousRequests = 10;

InverterGateway::InverterGateway(Settings *settings, QObject *parent) :
	QObject(parent),
	mSettings(settings)
{
	Q_ASSERT(settings != 0);
	for (int i=0; i<MaxSimultaneousRequests; ++i) {
		onStartDetection();
	}
	connect(settings, SIGNAL(propertyChanged(QString)),
			this, SLOT(onSettingsChanged(QString)));
}

void InverterGateway::onStartDetection()
{
	QString hostName;
	if (!mAddressGenerator.hasNext()) {
		updateAddressGenerator();
		mAddressGenerator.reset();
	}
	else
	{
		hostName = mAddressGenerator.next().toString();
		foreach (FroniusSolarApi *api, mApis) {
			if (api->hostName() == hostName) {
				hostName.clear();
				break;
			}
		}
	}
	if (hostName.isEmpty())
	{
		QTimer::singleShot(5000, this, SLOT(onStartDetection()));
		return;
	}
	qDebug() << __FUNCTION__ << hostName;
	FroniusSolarApi *api = new FroniusSolarApi(hostName, Port, this);
	mApis.append(api);
	connect(api, SIGNAL(converterInfoFound(InverterListData)),
			this, SLOT(onConverterInfoFound(InverterListData)));
	api->getConverterInfoAsync();
}

void InverterGateway::onConverterInfoFound(const InverterListData &data)
{
	FroniusSolarApi *api = static_cast<FroniusSolarApi *>(sender());
	if (data.error == InverterListData::NoError)
	{
		QString hostName = api->hostName();
		int port = api->port();
		qDebug() << __FUNCTION__ << hostName << port;
		QList<QHostAddress> knownIpAddresses = mSettings->knownIpAddresses();
		QHostAddress addr(hostName);
		if (!knownIpAddresses.contains(addr)) {
			knownIpAddresses.append(addr);
			mSettings->setKnownIpAddresses(knownIpAddresses);
		}
		for (QList<InverterInfo>::const_iterator it = data.inverters.begin();
			 it != data.inverters.end();
			 ++it)
		{
			InverterUpdater *iu = findUpdater(hostName, it->id);
			if (iu == 0)
			{
				Inverter *inverter = new Inverter(hostName, port, it->id,
												  it->uniqueId, this);
				InverterUpdater *updater = new InverterUpdater(inverter, this);
				mUpdaters.push_back(updater);
				emit inverterFound(*updater);
			}
		}
	}
	onStartDetection();
	mApis.removeOne(api);
	api->deleteLater();
}

void InverterGateway::onSettingsChanged(const QString &property)
{
	qDebug() << __FUNCTION__ << property;
	if (property == "autoDetect" || property == "ipAddresses" ||
			property == "knownIpAddresses") {
		updateAddressGenerator();
	}
}

InverterUpdater *InverterGateway::findUpdater(const QString &hostName,
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

InverterUpdater *InverterGateway::findUpdater(const QString &hostName)
{
	for (QList<InverterUpdater *>::iterator it = mUpdaters.begin();
		 it != mUpdaters.end();
		 ++it)
	{
		const Inverter *inverter = (*it)->inverter();
		if (inverter->hostName() == hostName)
			return *it;
	}
	return 0;
}

void InverterGateway::updateAddressGenerator()
{
	QList<QHostAddress> addresses = mSettings->ipAddresses();
	foreach (QHostAddress a, mSettings->knownIpAddresses()) {
		if (!addresses.contains(a)) {
			qDebug() << __FUNCTION__ << a;
			addresses.append(a);
		}
	}
	mAddressGenerator.setPriorityAddresses(addresses);
	mAddressGenerator.setPriorityOnly(!mSettings->autoDetect());
}

#include <QTimer>
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_updater.h"
#include "local_ip_address_generator.h"

/// @todo EV magic number
static const QString HostName = "192.168.4.144";
static const int Port = 8080;
static const int MaxSimultaneousRequests = 10;

InverterGateway::InverterGateway(QObject *parent) :
	QObject(parent),
	mAddressGenerator(0)
{
	for (int i=0; i<MaxSimultaneousRequests; ++i) {
		onStartDetection();
	}
}

InverterGateway::~InverterGateway()
{
	delete mAddressGenerator;
}

void InverterGateway::onStartDetection()
{
	if (mAddressGenerator == 0) {
		mAddressGenerator = new LocalIpAddressGenerator();
	}
	if (!mAddressGenerator->hasNext()) {
		// Delete to generator. When this function is called again, a new
		// generator will be created effectively restarting the IP address
		// generation.
		delete mAddressGenerator;
		mAddressGenerator = 0;
		QTimer::singleShot(5000, this, SLOT(onStartDetection()));
		return;
	}
	QString hostName = mAddressGenerator->next().toString();
	FroniusSolarApi *api = new FroniusSolarApi(hostName, Port, this);
	connect(
		api, SIGNAL(converterInfoFound(InverterInfoData)),
				this, SLOT(onConverterInfoFound(InverterInfoData)));
	api->getConverterInfoAsync();
}

void InverterGateway::onConverterInfoFound(const InverterInfoData &data)
{
	FroniusSolarApi *api = static_cast<FroniusSolarApi *>(sender());
	if (data.error == InverterInfoData::NoError)
	{
		QString hostName = api->hostName();
		int port = api->port();
		qDebug() << __FUNCTION__ << hostName << port;
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
	api->deleteLater();
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

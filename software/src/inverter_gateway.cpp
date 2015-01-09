#include <QTimer>
#include <QsLog.h>
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_settings.h"
#include "inverter_updater.h"
#include "settings.h"

static const int MaxSimultaneousRequests = 10;

InverterGateway::InverterGateway(Settings *settings, QObject *parent) :
	QObject(parent),
	mSettings(settings),
	mSettingsBusy(true)
{
	Q_ASSERT(settings != 0);
	connect(settings, SIGNAL(autoDetectChanged()),
			this, SLOT(onAutoDetectChanged()));
	connect(settings, SIGNAL(ipAddressesChanged()),
			this, SLOT(onSettingsChanged()));
	connect(settings, SIGNAL(knownIpAddressesChanged()),
			this, SLOT(onSettingsChanged()));
}

int InverterGateway::scanProgress() const
{
	return mAddressGenerator.progress();
}

void InverterGateway::startDetection()
{
	mSettingsBusy = true;
	mSettings->setAutoDetect(true);
	mSettingsBusy = false;
	updateAddressGenerator();
}

void InverterGateway::updateDetection()
{
	QString hostName;
	while (hostName.isEmpty() && mAddressGenerator.hasNext()) {
		hostName = mAddressGenerator.next().toString();
		foreach (FroniusSolarApi *api, mApis) {
			if (api->hostName() == hostName) {
				// There's already a web request send to this host.
				hostName.clear();
			}
		}
	}
	if (hostName.isEmpty()) {
		if (!mApis.isEmpty()) {
			// Wait for pending requests to return
			return;
		}
		bool autoDetect = false;
		if (mAddressGenerator.priorityOnly()) {
			// We scanned all known addresses. Have we found what we needed?
			QList<QHostAddress> addresses = mAddressGenerator.priorityAddresses();
			int count = 0;
			foreach (QHostAddress a, addresses) {
				if (findUpdater(a.toString()) != 0) {
					++count;
					break;
				}
			}
			if (count < addresses.size() || mUpdaters.size() == 0) {
				/// @todo EV We may get here when auto detect is disabled manually
				/// *before* any inverter have been found.
				// Some devices were missing or no devices were found at all.
				// Start auto scan.
				QLOG_INFO() << "Not all devices found, starting auto IP scan";
				autoDetect = true;
			} else {
				QLOG_INFO() << "Auto IP scan disabled, all devices found";
			}
		} else {
			QLOG_INFO() << "Auto IP scan completed. Detection finished";
		}
		mSettingsBusy = true;
		mSettings->setAutoDetect(autoDetect);
		mSettingsBusy = false;
		if (autoDetect) {
			mAddressGenerator.setPriorityOnly(false);
			mAddressGenerator.reset();
			for (int i=0; i<MaxSimultaneousRequests; ++i)
				updateDetection();
		}
		scanProgressChanged();
		return;
	}
	int portNumber = mSettings->portNumber();
	QLOG_TRACE() << "Scanning at" << hostName << ':' << portNumber;
	FroniusSolarApi *api = new FroniusSolarApi(hostName, portNumber, this);
	mApis.append(api);
	connect(api, SIGNAL(converterInfoFound(InverterListData)),
			this, SLOT(onConverterInfoFound(InverterListData)));
	api->getConverterInfoAsync();
}

void InverterGateway::onConverterInfoFound(const InverterListData &data)
{
	FroniusSolarApi *api = static_cast<FroniusSolarApi *>(sender());
	if (data.error == InverterListData::NoError) {
		QString hostName = api->hostName();
		int port = api->port();
		QLOG_INFO() << "Inverter found at" << hostName << ':' << port;
		QList<QHostAddress> knownIpAddresses = mSettings->knownIpAddresses();
		QHostAddress addr(hostName);
		if (!knownIpAddresses.contains(addr)) {
			knownIpAddresses.append(addr);
			mSettingsBusy = true;
			mSettings->setKnownIpAddresses(knownIpAddresses);
			mSettingsBusy = false;
		}
		for (QList<InverterInfo>::const_iterator it = data.inverters.begin();
			 it != data.inverters.end();
			 ++it) {
			InverterUpdater *iu = findUpdater(hostName, it->id);
			if (iu == 0) {
				Inverter *inverter = new Inverter(hostName, port, it->id,
												  it->uniqueId, it->customName,
												  this);
				// connect(inverter, SIGNAL(isConnectedChanged()),
				// 		this, SLOT(onIsConnectedChanged()));
				InverterSettings *is =
						mSettings->findInverterSettings(inverter->uniqueId());
				if (is == 0) {
					is = new InverterSettings(inverter->uniqueId(), this);
					QList<InverterSettings *> settings = mSettings->inverterSettings();
					settings.append(is);
					mSettings->setInverterSettings(settings);
				}
				InverterUpdater *updater = new InverterUpdater(inverter, is, this);
				mUpdaters.push_back(updater);
				emit inverterFound(updater);
			}
		}
	}
	mApis.removeOne(api);
	api->deleteLater();
	emit scanProgressChanged();
	updateDetection();
}

void InverterGateway::onAutoDetectChanged()
{
	if (mSettingsBusy)
		return;
	updateAddressGenerator();
}

void InverterGateway::onSettingsChanged()
{
	if (mSettingsBusy)
		return;
	mSettingsBusy = true;
	mSettings->setAutoDetect(true);
	mSettingsBusy = false;
	updateAddressGenerator();
}

void InverterGateway::onIsConnectedChanged()
{
	Inverter *inverter = static_cast<Inverter *>(sender());
	if (inverter->isConnected())
		return;
	QLOG_INFO() << "Lost connection with: " << inverter->hostName() << " - "
				<< inverter->id();
	// Do not call delete here, because this slot is connected to an inverter
	// slot, so one of its methods is still running.
	inverter->deleteLater();
	foreach (InverterUpdater *ui, mUpdaters) {
		if (ui->inverter() == inverter) {
			ui->deleteLater();
			mUpdaters.removeOne(ui);
			break;
		}
	}
	mSettingsBusy = true;
	mSettings->setAutoDetect(true);
	mSettingsBusy = false;
	updateAddressGenerator();
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
	QLOG_TRACE() << __FUNCTION__
				 << mSettings->autoDetect()
				 << mSettings->ipAddresses().size();
	if (mSettings->autoDetect()) {
		QList<QHostAddress> addresses = mSettings->ipAddresses();
		foreach (QHostAddress a, mSettings->knownIpAddresses()) {
			if (!addresses.contains(a)) {
				addresses.append(a);
			}
		}
		if (addresses.isEmpty()) {
			QLOG_INFO() << "Starting auto IP scan";
			mAddressGenerator.setPriorityOnly(false);
			mAddressGenerator.reset();
		} else {
			QLOG_INFO() << "Starting known IP scan";
			mAddressGenerator.setPriorityAddresses(addresses);
			mAddressGenerator.setPriorityOnly(true);
			mAddressGenerator.reset();
		}
		for (int i=mApis.size(); i<MaxSimultaneousRequests; ++i)
			updateDetection();
	} else {
		QLOG_INFO() << "Auto IP scan disabled";
		mAddressGenerator.setPriorityOnly(true);
		mAddressGenerator.setPriorityAddresses(QList<QHostAddress>());
		mAddressGenerator.reset();
	}
	emit scanProgressChanged();
}

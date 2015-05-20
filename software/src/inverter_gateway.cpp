#include <QTimer>
#include <QsLog.h>
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "inverter_settings.h"
#include "inverter_updater.h"
#include "settings.h"

static const int MaxSimultaneousRequests = 64;

InverterGateway::InverterGateway(Settings *settings, QObject *parent) :
	QObject(parent),
	mSettings(settings),
	mTimer(new QTimer(this)),
	mSettingsBusy(false),
	mAutoDetect(false),
	mFullScanRequested(false),
	mFullScanIfNoDeviceFound(false)
{
	mAddressGenerator.setNetMaskLimit(QHostAddress(0xFFFFF000));
	Q_ASSERT(settings != 0);
	connect(settings, SIGNAL(portNumberChanged()),
			this, SLOT(onSettingsChanged()));
	connect(settings, SIGNAL(ipAddressesChanged()),
			this, SLOT(onSettingsChanged()));
	connect(settings, SIGNAL(knownIpAddressesChanged()),
			this, SLOT(onSettingsChanged()));
	mTimer->setInterval(60000);
	mTimer->start();
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

bool InverterGateway::autoDetect() const
{
	return mAutoDetect;
}

void InverterGateway::setAutoDetect(bool b)
{
	if (mAutoDetect == b)
		return;
	mAutoDetect	= b;
	if (!mSettingsBusy) {
		mFullScanRequested = b;
		mFullScanIfNoDeviceFound = b;
		updateAddressGenerator();
	}
	emit autoDetectChanged();
}

int InverterGateway::scanProgress() const
{
	return mAddressGenerator.progress();
}

void InverterGateway::startDetection()
{
	mSettingsBusy = true;
	mFullScanIfNoDeviceFound = true;
	setAutoDetect(true);
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
				if (mDevicesFound.contains(a)) {
					++count;
					break;
				}
			}
			if (mFullScanRequested) {
				QLOG_INFO() << "Full scan requested, starting auto IP scan";
				autoDetect = true;
				mFullScanRequested = false;
			} else if (!mFullScanIfNoDeviceFound) {
				QLOG_DEBUG() << "No auto IP scan requested. Detection finished";
			} else if (count < addresses.size() || mDevicesFound.isEmpty()) {
				/// @todo EV We may get here when auto detect is disabled manually
				/// *before* any inverter have been found.
				// Some devices were missing or no devices were found at all.
				// Start auto scan.
				QLOG_INFO() << "Not all devices found, starting auto IP scan";
				autoDetect = true;
			} else {
				QLOG_DEBUG() << "Auto IP scan disabled, all devices found";
			}
		} else {
			QLOG_INFO() << "Auto IP scan completed. Detection finished";
		}
		mSettingsBusy = true;
		setAutoDetect(autoDetect);
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
		QLOG_DEBUG() << "Inverter found at" << hostName << ':' << port;
		QList<QHostAddress> knownIpAddresses = mSettings->knownIpAddresses();
		QHostAddress addr(hostName);
		if (!knownIpAddresses.contains(addr)) {
			knownIpAddresses.append(addr);
			mSettingsBusy = true;
			mSettings->setKnownIpAddresses(knownIpAddresses);
			mSettingsBusy = false;
		}
		mDevicesFound.append(addr);
		for (QList<InverterInfo>::const_iterator it = data.inverters.begin();
			 it != data.inverters.end();
			 ++it) {
			Inverter *inverter = new Inverter(hostName, port, it->id,
											  it->deviceType, it->uniqueId,
											  it->customName, this);
			emit inverterFound(inverter);
			if (inverter->parent() == this) {
				delete inverter;
			}
		}
	}
	mApis.removeOne(api);
	api->deleteLater();
	emit scanProgressChanged();
	updateDetection();
}

void InverterGateway::onSettingsChanged()
{
	if (mSettingsBusy)
		return;
	mSettingsBusy = true;
	setAutoDetect(true);
	mSettingsBusy = false;
	updateAddressGenerator();
}

void InverterGateway::onTimer()
{
	if (mAddressGenerator.hasNext())
		return;
	QList<QHostAddress> addresses = mSettings->ipAddresses();
	foreach (QHostAddress a, mSettings->knownIpAddresses()) {
		if (!addresses.contains(a)) {
			addresses.append(a);
		}
	}
	if (addresses.isEmpty())
		return;
	QLOG_DEBUG() << "Starting known IP scan (timer based)";
	mFullScanIfNoDeviceFound = false;
	mAddressGenerator.setPriorityAddresses(addresses);
	mAddressGenerator.setPriorityOnly(true);
	mAddressGenerator.reset();
	for (int i=mApis.size(); i<MaxSimultaneousRequests; ++i)
		updateDetection();
}

void InverterGateway::updateAddressGenerator()
{
	if (autoDetect()) {
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
			QLOG_DEBUG() << "Starting known IP scan";
			mAddressGenerator.setPriorityAddresses(addresses);
			mAddressGenerator.setPriorityOnly(true);
			mAddressGenerator.reset();
		}
		for (int i=mApis.size(); i<MaxSimultaneousRequests; ++i)
			updateDetection();
	} else {
		QLOG_DEBUG() << "Auto IP scan disabled";
		mAddressGenerator.setPriorityOnly(true);
		mAddressGenerator.setPriorityAddresses(QList<QHostAddress>());
		mAddressGenerator.reset();
	}
	emit scanProgressChanged();
}

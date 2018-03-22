#include <QTimer>
#include <QsLog.h>
#include "inverter_gateway.h"
#include "abstract_detector.h"
#include "settings.h"

static const int MaxSimultaneousRequests = 32;

InverterGateway::InverterGateway(AbstractDetector *detector, Settings *settings, QObject *parent) :
	QObject(parent),
	mSettings(settings),
	mDetector(detector),
	mTimer(new QTimer(this)),
	mSettingsBusy(false),
	mAutoDetect(false),
	mFullScanRequested(false),
	mFullScanIfNoDeviceFound(false)
{
	Q_ASSERT(settings != 0);
	mAddressGenerator.setNetMaskLimit(QHostAddress(0xFFFFF000));
	updateScanProgress();
	connect(settings, SIGNAL(portNumberChanged()), this, SLOT(onSettingsChanged()));
	connect(settings, SIGNAL(ipAddressesChanged()), this, SLOT(onSettingsChanged()));
	connect(settings, SIGNAL(knownIpAddressesChanged()), this, SLOT(onSettingsChanged()));
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
	mAutoDetect = b;
	if (!mSettingsBusy) {
		mFullScanRequested = b;
		mFullScanIfNoDeviceFound = b;
		updateAddressGenerator();
	}
	emit autoDetectChanged();
}

int InverterGateway::scanProgress() const
{
	return mAutoDetect ? mAddressGenerator.progress(mActiveHostNames.count()) : 100;
}

void InverterGateway::startDetection()
{
	mSettingsBusy = true;
	mFullScanIfNoDeviceFound = true;
	setAutoDetect(true);
	mSettingsBusy = false;
	updateAddressGenerator();
}

void InverterGateway::onInverterFound(const DeviceInfo &deviceInfo)
{
	QList<QHostAddress> addresses = mSettings->knownIpAddresses();
	QHostAddress addr(deviceInfo.hostName);
	if (!addresses.contains(addr)) {
		addresses.append(addr);
		mSettingsBusy = true; // prevent onSettingsChanged from firing
		mSettings->setKnownIpAddresses(addresses);
		mSettingsBusy = false;
	}
	emit inverterFound(deviceInfo);
}


void InverterGateway::onDetectionDone()
{
	DetectorReply *reply = static_cast<DetectorReply *>(sender());
	reply->deleteLater();
	mActiveHostNames.removeOne(QHostAddress(reply->hostName()));
	updateScanProgress();
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
	for (int i=mActiveHostNames.size(); i<MaxSimultaneousRequests; ++i)
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
		for (int i=mActiveHostNames.size(); i<MaxSimultaneousRequests; ++i)
			updateDetection();
	} else {
		QLOG_DEBUG() << "Auto IP scan disabled";
		mAddressGenerator.setPriorityOnly(true);
		mAddressGenerator.setPriorityAddresses(QList<QHostAddress>());
		mAddressGenerator.reset();
	}
	updateScanProgress();
}

void InverterGateway::updateDetection()
{
	QString hostName;
	while (hostName.isEmpty() && mAddressGenerator.hasNext()) {
		hostName = mAddressGenerator.next().toString();
		if (mActiveHostNames.contains(QHostAddress(hostName)))
			hostName.clear();
	}
	if (hostName.isEmpty()) {
		if (!mActiveHostNames.isEmpty()) {
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
			for (int i=mActiveHostNames.size(); i<MaxSimultaneousRequests; ++i)
				updateDetection();
		}
		updateScanProgress();
		return;
	}
	QLOG_TRACE() << "Scanning" << hostName;
	mActiveHostNames.append(QHostAddress(hostName));
	DetectorReply *reply = mDetector->start(hostName);
	connect(reply, SIGNAL(deviceFound(const DeviceInfo &)),
			this, SLOT(onInverterFound(const DeviceInfo &)));
	connect(reply, SIGNAL(finished()), this, SLOT(onDetectionDone()));
}

void InverterGateway::updateScanProgress()
{
	emit scanProgressChanged();
}

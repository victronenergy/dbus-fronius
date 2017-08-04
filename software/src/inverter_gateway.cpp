#include <QTimer>
#include <QsLog.h>
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_gateway.h"
#include "settings.h"

static const int MaxSimultaneousRequests = 64;

InverterGateway::InverterGateway(Settings *settings, VeQItem *root, QObject *parent) :
	VeService(root, parent),
	mSettings(settings),
	mTimer(new QTimer(this)),
	mSettingsBusy(false),
	mAutoDetect(createItem("AutoDetect")),
	mScanProgress(createItem("ScanProgress")),
	mFullScanRequested(false),
	mFullScanIfNoDeviceFound(false)
{
	Q_ASSERT(settings != 0);
	mAddressGenerator.setNetMaskLimit(QHostAddress(0xFFFFF000));
	produceValue(mAutoDetect, 0, "Idle");
	updateScanProgress();
	connect(settings, SIGNAL(portNumberChanged()), this, SLOT(onSettingsChanged()));
	connect(settings, SIGNAL(ipAddressesChanged()), this, SLOT(onSettingsChanged()));
	connect(settings, SIGNAL(knownIpAddressesChanged()), this, SLOT(onSettingsChanged()));
	mTimer->setInterval(60000);
	mTimer->start();
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
	registerService();
}

bool InverterGateway::autoDetect() const
{
	return mAutoDetect->getValue().toBool();
}

void InverterGateway::setAutoDetect(bool b)
{
	if (autoDetect() == b)
		return;
	produceValue(mAutoDetect, b ? 1 : 0, b ? "Busy" : "Idle");
	if (!mSettingsBusy) {
		mFullScanRequested = b;
		mFullScanIfNoDeviceFound = b;
		updateAddressGenerator();
	}
	emit autoDetectChanged();
}

void InverterGateway::startDetection()
{
	mSettingsBusy = true;
	mFullScanIfNoDeviceFound = true;
	setAutoDetect(true);
	mSettingsBusy = false;
	updateAddressGenerator();
}

int InverterGateway::handleSetValue(VeQItem *item, const QVariant &variant)
{
	if (item == mAutoDetect) {
		setAutoDetect(variant.toBool());
		return 0;
	}
	return VeService::handleSetValue(item, variant);
}

void InverterGateway::onDetectionDone(const QString &hostName)
{
	mActiveHostNames.removeOne(QHostAddress(hostName));
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
	mDetector->start(hostName);
}

void InverterGateway::updateScanProgress()
{
	produceDouble(mScanProgress, mAddressGenerator.progress(), 0, "%");
}

#include <qnumeric.h>
#include <QsLog.h>
#include <velib/vecan/products.h>
#include "froniussolar_api.h"
#include "fronius_device_info.h"
#include "settings.h"
#include "solar_api_detector.h"
#include "sunspec_detector.h"

QList<QString> SolarApiDetector::mInvalidDevices;

SolarApiDetector::SolarApiDetector(const Settings *settings, QObject *parent):
	AbstractDetector(parent),
	mSunspecDetector(new SunspecDetector(this)),
	mSettings(settings)
{
}

DetectorReply *SolarApiDetector::start(const QString &hostName)
{
	Reply *reply = new Reply(this);
	reply->api = new FroniusSolarApi(hostName, mSettings->portNumber(), reply);
	mApiToReply[reply->api] = reply;
	connect(reply->api, SIGNAL(converterInfoFound(InverterListData)),
		this, SLOT(onConverterInfoFound(InverterListData)));
	reply->api->getConverterInfoAsync();
	return reply;
}

void SolarApiDetector::onConverterInfoFound(const InverterListData &data)
{
	FroniusSolarApi *api = static_cast<FroniusSolarApi *>(sender());
	Reply *reply = mApiToReply.value(api);
	bool setFinished = true;
	for (QList<InverterInfo>::const_iterator it = data.inverters.begin();
		 it != data.inverters.end();
		 ++it) {
		// Sometimes (during startup?) PV inverters will send 255 as device
		// type instead of the real type. We have only seen this in a test
		// setup with a Fronius IG Plus 50 V-1.
		if (it->deviceType == 255) {
			if (!mInvalidDevices.contains(it->uniqueId)) {
				mInvalidDevices.append(it->uniqueId);
				QLOG_WARN() << "PV inverter reported type 255. Serial:" << it->uniqueId;
			}
		} else {
			ReplyToInverter device;
			device.reply = reply;
			device.inverter = *it;

			mSunspecDetector->setUnitId(it->id);
			DetectorReply *dr = mSunspecDetector->start(api->hostName());
			connect(dr, SIGNAL(deviceFound(DeviceInfo)),
					this, SLOT(onSunspecDeviceFound(DeviceInfo)));
			connect(dr, SIGNAL(finished()), this, SLOT(onSunspecDone()));
			mDetectorReplyToInverter[dr] = device;
			setFinished = false;
		}
	}
	if (setFinished)
		reply->setFinished();
}

void SolarApiDetector::onSunspecDeviceFound(const DeviceInfo &info)
{
	DetectorReply *dr = static_cast<DetectorReply *>(sender());
	ReplyToInverter &device = mDetectorReplyToInverter[dr];
	Q_ASSERT(device.reply != 0);
	if (device.reply == 0)
		return;
	DeviceInfo i2(info);
	// unique ID is used to find inverter settings, and to determine the device instance. In
	// previous versions, the unique ID constructed from data retrieved using the solar API.
	// The sunspec detector uses the serial number of the inverter as unique ID, which is not
	// available in the solar API.
	i2.uniqueId = fixUniqueId(device.inverter);
	device.deviceFound = true;
	device.reply->setResult(i2);
}

void SolarApiDetector::onSunspecDone()
{
	DetectorReply *dr = static_cast<DetectorReply *>(sender());
	dr->deleteLater();
	ReplyToInverter device = mDetectorReplyToInverter.take(dr);
	Q_ASSERT(device.reply != 0);
	if (device.reply == 0)
		return;
	if (device.deviceFound) {
		checkFinished(device.reply);
		return;
	}
	// Sunspec was not enabled for this inverter, so we fall back to solar api.
	DeviceInfo info;
	info.networkId = device.inverter.id;
	info.uniqueId = fixUniqueId(device.inverter);
	info.hostName = device.reply->api->hostName();
	info.port = device.reply->api->port();
	info.productId = VE_PROD_ID_PV_INVERTER_FRONIUS;
	info.maxPower = qQNaN();
	const FroniusDeviceInfo *deviceInfo = FroniusDeviceInfo::find(device.inverter.deviceType);
	if (deviceInfo == 0) {
		QLOG_WARN() << "Unknown inverter type:" << device.inverter.deviceType;
		info.productName = "Unknown PV Inverter";
		info.phaseCount = 1;
	} else {
		info.productName = deviceInfo->name;
		info.phaseCount = deviceInfo->phaseCount;
	}
	device.deviceFound = true;
	device.reply->setResult(info);
	checkFinished(device.reply);
}

QString SolarApiDetector::fixUniqueId(const InverterInfo &inverterInfo)
{
	bool isOk = false;
	QString result;
	foreach (QChar c, inverterInfo.uniqueId) {
		c = c.toLatin1();
		if (!c.isLetterOrNumber()) {
			c = '_';
		} else {
			isOk = true;
		}
		result += c;
	}
	if (!isOk)
		result = QString("T%1").arg(inverterInfo.id);
	return QString("%1_%2").arg(inverterInfo.deviceType).arg(result);
}

void SolarApiDetector::checkFinished(Reply *reply)
{
	// Check if there are any pending replies from the sunspec detector associated `reply`.
	// We need this because a single call to getConverterInfoAsync in the solar API may give us
	// multiple PV inverters. Each PV inverter is tested for ModbusTCP support. Only after the last
	// test has been completed the request, which initiated the call to getConverterInfoAsync can
	// be finished.
	foreach (const ReplyToInverter &rti, mDetectorReplyToInverter) {
		if (rti.reply == reply && !rti.deviceFound)
			return;
	}
	reply->setFinished();
}

SolarApiDetector::Reply::Reply(QObject *parent):
	DetectorReply(parent),
	api(0)
{
}

SolarApiDetector::Reply::~Reply()
{
}

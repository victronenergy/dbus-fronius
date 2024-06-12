#include <qnumeric.h>
#include "products.h"
#include "froniussolar_api.h"
#include "fronius_device_info.h"
#include "settings.h"
#include "solar_api_detector.h"
#include "sunspec_detector.h"
#include "logging.h"

QList<QString> SolarApiDetector::mInvalidDevices;

SolarApiDetector::SolarApiDetector(const Settings *settings, QObject *parent):
	AbstractDetector(parent),
	mSunspecDetector(new SunspecDetector(this)),
	mSettings(settings)
{
}

DetectorReply *SolarApiDetector::start(const QString &hostName, int timeout)
{
	Reply *reply = new Reply(this);
	reply->api = new Api(hostName, mSettings->portNumber(), timeout, reply);
	connect(reply->api, SIGNAL(deviceInfoFound(DeviceInfoData)),
		this, SLOT(onDeviceInfoFound(DeviceInfoData)));
	connect(reply->api, SIGNAL(converterInfoFound(InverterListData)),
		this, SLOT(onConverterInfoFound(InverterListData)));
	connect(reply->api, SIGNAL(threePhasesDataFound(const ThreePhasesInverterData &)),
		this, SLOT(onThreePhaseDataFound(const ThreePhasesInverterData &)));
	reply->api->getDeviceInfoAsync();
	return reply;
}

void SolarApiDetector::onDeviceInfoFound(const DeviceInfoData &data)
{
	Api *api = static_cast<Api *>(sender());
	Reply *reply = static_cast<Reply *>(api->parent());
	reply->serialInfo = data.serialInfo; // Store for later use
	reply->api->getConverterInfoAsync();
}

void SolarApiDetector::onConverterInfoFound(const InverterListData &data)
{
	Api *api = static_cast<Api *>(sender());
	Reply *reply = static_cast<Reply *>(api->parent());
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
				qWarning() << "PV inverter reported type 255. Serial:" << it->uniqueId;
			}
		} else {
			// We already know we have a Fronius DataManager on this address.
			// Allowing a longer timeout for sunspec only slows us down where
			// we already know there is a Fronius PV-inverter, and this caters
			// for very slow DataManagers with several PV-inverters connected.
			DetectorReply *dr = mSunspecDetector->start(api->hostName(), 25000, it->id);
			if (dr == 0) {
				// If we already have a connection to this inverter, the detector will return
				// null.
				qDebug() << QString("SunSpec scan skipped for %1:%2").arg(api->hostName()).arg(it->id);
				continue;
			}

			ReplyToInverter device;
			device.reply = reply;
			device.inverter = *it;
			connect(dr, SIGNAL(deviceFound(DeviceInfo)),
					this, SLOT(onSunspecDeviceFound(DeviceInfo)));
			connect(dr, SIGNAL(finished()), this, SLOT(onSunspecDone()));
			// Lookup structurs so we can find this data later in
			// onSunspecDone/onSunspecDeviceFound, or based on the networkId.
			mDetectorReplyToInverter[dr] = device;
			mIdReplyToInverter[it->id] = device;
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
	// The sunspec detector uses the serial number of the inverter as unique ID, which was
	// not available in the solar API in older firmware versions. To keep things consistent
	// across models, continue to use the uniqueId even for sunspec inverters.
	//
	// At least, that was the theory until 2023.  A bug in some datamanagers
	// causes PV-inverters to have uniqueId 16777215.  This is 2^24-1, the max
	// 24-bit number. With these datamanagers, the uniqueId isn't unique, and
	// we switch to the serial number instead (which is known at this point,
	// because the SunSpec probe is done)). This breaks consistency somewhat
	// between solarapi and sunspec, but is better than ignoring one of the
	// PV-inverters.
	if (mSettings->idBySerial())
		i2.uniqueId = fixUniqueId(device.inverter.deviceType, info.serialNumber, device.inverter.id);
	else
		i2.uniqueId = fixUniqueId(device.inverter);

	// Fronius inverters have a deviceType that is exposed on solarAPI but not
	// via sunspec. Transplant it here. If this is not a Fronius inverter
	// this value will simply be a zero.
	i2.deviceType = device.inverter.deviceType;

	device.deviceFound = true;
	device.reply->setResult(i2);
}

// This is called multiple times, for each DetectorReply from the
// SunspecDetector
void SolarApiDetector::onSunspecDone()
{
	// Get the DetectorReply that sent this signal, and remove it
	// from mDetectorReplyToInverter. Leave mIdReplyToInverter
	// alone for now, we may need it later, but clear it
	// the moment you know a device was found.
	DetectorReply *dr = static_cast<DetectorReply *>(sender());
	dr->deleteLater();
	ReplyToInverter device = mDetectorReplyToInverter.take(dr);
	Q_ASSERT(device.reply != 0);
	if (device.reply == 0)
		return;

	// If a sunspec device was found, we're done with this DetectorReply
	if (device.deviceFound) {
		mIdReplyToInverter.remove(device.inverter.id);
		checkSunspecFinished(device.reply);
		return;
	}

	// Sunspec was not enabled for this inverter, so we fall back to solar api.
	// First attempt to look it up in our list of known inverters, if that
	// fails, fetch the 3PInverterData collection to check if it has 3-phase
	// support.
	const FroniusDeviceInfo *deviceInfo = FroniusDeviceInfo::find(device.inverter.deviceType);
	if (deviceInfo == 0) {
		qWarning() << "Unknown inverter type:" << device.inverter.deviceType;
		device.reply->api->getThreePhasesInverterDataAsync(device.inverter.id);
		return; // We're done at this point
	}

	// Remove from hash, it can be handled.
	mIdReplyToInverter.remove(device.inverter.id);

	DeviceInfo info;
	info.networkId = device.inverter.id;
	info.uniqueId = fixUniqueId(device.inverter);
	info.hostName = device.reply->api->hostName();
	info.port = device.reply->api->port();
	info.deviceType = device.inverter.deviceType;
	info.productId = VE_PROD_ID_PV_INVERTER_FRONIUS;
	info.maxPower = qQNaN();
	info.serialNumber = device.reply->serialInfo.value(device.inverter.id, QString());
	info.productName = deviceInfo->name;
	info.phaseCount = deviceInfo->phaseCount;
	device.deviceFound = true;
	device.reply->setResult(info);
	checkSunspecFinished(device.reply);
}

void SolarApiDetector::onThreePhaseDataFound(const ThreePhasesInverterData &data)
{
	// If we are here, we're dealing with an unknown SolarApi inverter, and
	// we're trying to find the phase config. For Solar.Net devices, the
	// deviceId is the numeric id that matches the one we asked for, so
	// we can use that to fetch the data pertaining to the outstanding
	// DetectorReply, and then complete it.
	bool ok;
	int deviceId = data.deviceId.toInt(&ok);
	if (!ok)
		return;

	ReplyToInverter device = mIdReplyToInverter.take(deviceId);

	DeviceInfo info;
	info.networkId = device.inverter.id;
	info.uniqueId = fixUniqueId(device.inverter);
	info.hostName = device.reply->api->hostName();
	info.port = device.reply->api->port();
	info.deviceType = device.inverter.deviceType;
	info.productId = VE_PROD_ID_PV_INVERTER_FRONIUS;
	info.maxPower = qQNaN();
	info.serialNumber = device.reply->serialInfo.value(device.inverter.id, QString());
	// To my knowledge Fronius has no split-phase (with neutral) inverters.
	// North American inverters are 1P inverters across L1 and L2.
	info.phaseCount = data.valid ? 3 : 1;
	info.productName = QString("%1-phase PV Inverter").arg(data.valid?"Three":"Single");

	// And complete the detection
	device.deviceFound = true;
	device.reply->setResult(info);
	checkSunspecFinished(device.reply);
}

QString SolarApiDetector::fixUniqueId(const InverterInfo &inverterInfo)
{
	return fixUniqueId(inverterInfo.deviceType, inverterInfo.uniqueId, inverterInfo.id);
}

QString SolarApiDetector::fixUniqueId(int deviceType, QString uniqueId, int id)
{
	bool isOk = false;
	QString result;
	foreach (QChar c, uniqueId) {
		c = c.toLatin1();
		if (!c.isLetterOrNumber()) {
			c = '_';
		} else {
			isOk = true;
		}
		result += c;
	}
	if (!isOk)
		result = QString("T%1").arg(id);
	return QString("%1_%2").arg(deviceType).arg(result);
}

void SolarApiDetector::checkSunspecFinished(Reply *reply)
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

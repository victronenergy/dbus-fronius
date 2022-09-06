#include "ve_service.h"
#include "fronius_inverter.h"

// Fronius inverters send a null payload during certain solar net timeouts. We
// want to filter for those.
static const QVector<quint16> FroniusNullFrame = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7 };


FroniusInverter::FroniusInverter(VeQItem *root, const DeviceInfo &deviceInfo,
					int deviceInstance, QObject *parent) :
	Inverter(root, deviceInfo, deviceInstance, parent)
{
	produceValue(createItem("FroniusDeviceType"), deviceInfo.deviceType);

	// If it has sunspec model 123, powerLimitScale will be non-zero.
	// Enable the power limiter and initialise it to maxPower.
	if (deviceInfo.powerLimitScale)
		setPowerLimit(deviceInfo.maxPower);
}

bool FroniusInverter::validateSunspecMonitorFrame(QVector<quint16> frame)
{
	// When there are communication timeouts between a Fronius
	// datamanager and the PV-inverters, we will sometimes receive a
	// frame consisting entirely of zeroes, except for an operating
	// state of 7 (Fault) and a vendor state of 10. Fronius recommendeds
	// that we simply filter these values.
	if (deviceInfo().retrievalMode == ProtocolSunSpecIntSf &&
			frame.mid(2, 37) == FroniusNullFrame)
		return false;
	return true;
}

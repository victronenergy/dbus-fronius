#include "ve_service.h"
#include "fronius_inverter.h"

// Fronius inverters send a null payload during certain solar net timeouts. We
// want to filter for those.
static const QVector<quint16> FroniusNullFrame = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 10 };


FroniusInverter::FroniusInverter(VeQItem *root, const DeviceInfo &deviceInfo,
					int deviceInstance, QObject *parent) :
	Inverter(root, deviceInfo, deviceInstance, parent)
{
	produceValue(createItem("FroniusDeviceType"), deviceInfo.deviceType);
}

bool FroniusInverter::validateSunspecMonitorFrame(QVector<quint16> frame)
{
	// When there are communication timeouts between a Fronius
	// datamanager and the PV-inverters, we will sometimes receive a
	// frame consisting entirely of zeroes, except for an operating
	// state of 7 (Fault) and a vendor state of 10. Fronius recommendeds
	// that we simply filter these values.
	if (deviceInfo().retrievalMode == ProtocolSunSpecIntSf &&
			frame.mid(2, 38) == FroniusNullFrame)
		return false;
	return true;
}

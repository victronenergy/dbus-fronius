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
}

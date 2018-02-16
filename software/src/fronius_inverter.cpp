#include "ve_service.h"
#include "fronius_inverter.h"

FroniusInverter::FroniusInverter(VeQItem *root, const DeviceInfo &deviceInfo,
					int deviceInstance, QObject *parent) :
	Inverter(root, deviceInfo, deviceInstance, parent)
{
	produceValue(createItem("FroniusDeviceType"), deviceInfo.deviceType);
}

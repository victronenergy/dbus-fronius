#ifndef FRONIUS_INVERTER_H
#define FRONIUS_INVERTER_H

#include "inverter.h"

class FroniusInverter : public Inverter
{
	Q_OBJECT
public:
	FroniusInverter(VeQItem *root, const DeviceInfo &deviceInfo, int deviceInstance, QObject *parent = 0);
};

#endif // FRONIUS_INVERTER_H

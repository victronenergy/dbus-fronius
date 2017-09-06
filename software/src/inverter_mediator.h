#ifndef INVERTERMEDIATOR_H
#define INVERTERMEDIATOR_H

#include <QObject>
#include "defines.h"

class GatewayInterface;
class Inverter;
class InverterSettings;
class Settings;

class InverterMediator : public QObject
{
	Q_OBJECT
public:
	explicit InverterMediator(const DeviceInfo &device, GatewayInterface *gateway,
							  Settings *settings, QObject *parent = 0);

	bool processNewInverter(const DeviceInfo &deviceInfo);

private slots:
	void onSettingsInitialized();

	void onIsActivatedChanged();

	void onConnectionLost();

	void onInverterModelChanged();

	void onPositionChanged();

	void onSettingsCustomNameChanged();

	void onInverterCustomNameChanged();

private:
	void startAcquisition();

	Inverter *createInverter();

	DeviceInfo mDeviceInfo;
	Inverter *mInverter;
	InverterSettings *mInverterSettings;
	GatewayInterface *mGateway;
	Settings *mSettings;
};

#endif // INVERTERMEDIATOR_H

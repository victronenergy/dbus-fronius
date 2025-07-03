#ifndef INVERTERMEDIATOR_H
#define INVERTERMEDIATOR_H

#include <QObject>
#include "defines.h"

class GatewayInterface;
class Inverter;
class InverterSettings;
class Settings;

/*!
 * Represents a PV inverter, and manages data retrieval and D-Bus publishing.
 */
class InverterMediator : public QObject
{
	Q_OBJECT
public:
	explicit InverterMediator(const DeviceInfo &device, GatewayInterface *gateway,
							  Settings *settings, QObject *parent = 0);

	/*!
	 * Checks if the given device is represented by this class.
	 * If so, it will make the necessary adjustments to the data retrieval (like changing
	 * the IP address or the retrieval mode).
	 * @param deviceInfo Describes a newly found PV inverter
	 * @return true is this class represent the given device info. False otherwise.
	 */
	bool processNewInverter(const DeviceInfo &deviceInfo);

private slots:
	void onSettingsInitialized();

	void onIsActivatedChanged();

	void onConnectionLost();

	void onInverterModelChanged();

	void onSettingsPositionChanged();

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

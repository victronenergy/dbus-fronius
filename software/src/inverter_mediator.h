#ifndef INVERTERMEDIATOR_H
#define INVERTERMEDIATOR_H

#include <QObject>

struct DeviceInfo;
class Inverter;
class InverterGateway;
class InverterSettings;
class Settings;

class InverterMediator : public QObject
{
	Q_OBJECT
public:
	explicit InverterMediator(const DeviceInfo &device, InverterGateway *gateway,
							  Settings *settings, QObject *parent = 0);

	bool processNewInverter(const DeviceInfo &deviceInfo);

private slots:
	void onSettingsInitialized();

	void onIsActivatedChanged();

	void onConnectionLost();

	void onPositionChanged();

	void onSettingsCustomNameChanged();

	void onInverterCustomNameChanged();

private:
	void startAcquisition();

	Inverter *createInverter(const DeviceInfo &di);

	Inverter *mInverter;
	InverterSettings *mInverterSettings;
	InverterGateway *mGateway;
	Settings *mSettings;
	int mDeviceType;
	QString mUniqueId;
};

#endif // INVERTERMEDIATOR_H

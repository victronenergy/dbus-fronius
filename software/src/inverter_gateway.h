#ifndef INVERTER_GATEWAY_H
#define INVERTER_GATEWAY_H

#include <QHostAddress>
#include <QPointer>
#include <QStringList>
#include "defines.h"
#include "local_ip_address_generator.h"
#include "ve_service.h"

class AbstractDetector;
class QTimer;
class Settings;

/*!
 * @brief Tries to find fronius PV inverters on the network.
 * This class InverterGateway will try to find fronius PV inverters using the
 * solar API. How this is done is determined by the `Settings` object passed
 * to the constructor. Whenever the `Settings` object changes the search
 * strategy will be adjusted.
 * There are 2 possible strategies:
 * - Scanning a list of known devices. IP addresses are taken from the
 *   ipAddresses and knownIpAddresses in the settings.
 * - Scanning all IP addresses within the local network.
 *
 * The diagram below shows in which order devices are scanned.
 * @dotfile ipaddress_scanning.dot
 */
class InverterGateway : public VeService
{
	Q_OBJECT
public:
	InverterGateway(Settings *settings, VeQItem *root, QObject *parent = 0);

	 bool autoDetect() const;

	 void setAutoDetect(bool b);

	// int scanProgress() const;

	void startDetection();

	virtual int handleSetValue(VeQItem *item, const QVariant &variant);

signals:
	void inverterFound(const DeviceInfo &deviceInfo);

	void autoDetectChanged();

private slots:
	void onInverterFound(const DeviceInfo &deviceInfo);

	void onDetectionDone();

	void onSettingsChanged();

	void onTimer();

private:
	void updateAddressGenerator();

	void updateDetection();

	void updateScanProgress();

	QPointer<Settings> mSettings;
	QList<QHostAddress> mDevicesFound;
	QList<QHostAddress> mActiveHostNames;
	LocalIpAddressGenerator mAddressGenerator;
	AbstractDetector *mDetector;
	QTimer *mTimer;
	bool mSettingsBusy;
	VeQItem *mAutoDetect;
	VeQItem *mScanProgress;
	bool mFullScanRequested;
	bool mFullScanIfNoDeviceFound;
};

#endif // INVERTER_GATEWAY_H

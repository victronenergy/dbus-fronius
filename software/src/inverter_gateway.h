#ifndef INVERTER_GATEWAY_H
#define INVERTER_GATEWAY_H

#include <QHostAddress>
#include <QPointer>
#include <QStringList>
#include "defines.h"
#include "gateway_interface.h"
#include "local_ip_address_generator.h"

class AbstractDetector;
class QTimer;
class Settings;

/*!
 * Handles device detection logistics for a single communication protocol.
 * There are 2 possible strategies:
 * - Scanning a list of known devices. IP addresses are taken from the
 *   ipAddresses and knownIpAddresses in the settings. This scan is very quick because a limited
 *   number of IP-addresses is queried and will be repeated every minute.
 * - Scanning all IP addresses within the local network (limited is the netmark is too wide). This
 *   scan is performed on startup and can be requested manually by calling `startDetection`.
 *
 * The diagram below shows in which order devices are scanned.
 * @dotfile ipaddress_scanning.dot
 */
class InverterGateway : public QObject, public GatewayInterface
{
	Q_OBJECT
public:
	InverterGateway(AbstractDetector *detector, Settings *settings, QObject *parent = 0);

	bool autoDetect() const;

	void setAutoDetect(bool b);

	int scanProgress() const;

	virtual void startDetection();

signals:
	void inverterFound(const DeviceInfo &deviceInfo);

	void autoDetectChanged();

	void scanProgressChanged();

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
	bool mAutoDetect;
	bool mFullScanRequested;
	bool mFullScanIfNoDeviceFound;
};

#endif // INVERTER_GATEWAY_H

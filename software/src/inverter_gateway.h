#ifndef INVERTER_GATEWAY_H
#define INVERTER_GATEWAY_H

#include <QHostAddress>
#include <QPointer>
#include <QStringList>
#include "defines.h"
#include "gateway_interface.h"
#include "local_ip_address_generator.h"

class AbstractDetector;
class FroniusUdpDetector;
class QTimer;
class Settings;
class HostScan;
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
	InverterGateway(Settings *settings, QObject *parent = 0);

	void addDetector(AbstractDetector *detector);

	bool autoDetect() const;

	int scanProgress() const;

	void initializeSettings();

	virtual void startDetection();

	void fullScan();

signals:
	void inverterFound(const DeviceInfo &deviceInfo);

	void autoDetectChanged();

	void scanProgressChanged();

private slots:
	void setAutoDetect(bool b);

	void onInverterFound(const DeviceInfo &deviceInfo);

	void onDetectionDone();

	void onPortNumberChanged();

	void onIpAddressesChanged();

	void onTimer();

	void continueScan();

private:
	enum ScanType
	{
		None, // Not scanning at the moment
		Full, // Full scan
		Priority, // Scan known addresses
		TryPriority // Do priority, switch to full if all not found
	};

	void updateScanProgress();

	void scanHost(QString hostName);

	void scan(enum ScanType scanType);

	QPointer<Settings> mSettings;
	QSet<QHostAddress> mDevicesFound;
	QList<HostScan *> mActiveHosts;
	LocalIpAddressGenerator mAddressGenerator;
	QList<AbstractDetector *> mDetectors;
	QTimer *mTimer;
	FroniusUdpDetector *mUdpDetector;
	bool mAutoDetect;
	bool mTriedFull;
	enum ScanType mScanType;
};

class HostScan: public QObject
{
    Q_OBJECT
public:
	HostScan(QList<AbstractDetector *> detectors, QString hostname, QObject *parent = 0);
	QString hostName() { return mHostname; }
	void scan();

signals:
	void deviceFound(const DeviceInfo &deviceInfo);
	void finished();

private slots:
	void continueScan();
	void onDeviceFound(const DeviceInfo &deviceInfo);

private:
	QList<AbstractDetector *> mDetectors;
	QString mHostname;
};

#endif // INVERTER_GATEWAY_H

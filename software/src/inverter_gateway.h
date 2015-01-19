#ifndef INVERTER_GATEWAY_H
#define INVERTER_GATEWAY_H

#include <QHostAddress>
#include <QObject>
#include <QPointer>
#include "local_ip_address_generator.h"

class FroniusSolarApi;
class Inverter;
class Settings;
struct InverterListData;

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
class InverterGateway : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool autoDetect READ autoDetect WRITE setAutoDetect NOTIFY autoDetectChanged)
	Q_PROPERTY(int scanProgress READ scanProgress NOTIFY scanProgressChanged)
public:
	InverterGateway(Settings *settings, QObject *parent = 0);

	bool autoDetect() const;

	void setAutoDetect(bool b);

	int scanProgress() const;

	void startDetection();

signals:
	void inverterFound(Inverter *inverter);

	void autoDetectChanged();

	void scanProgressChanged();

private slots:
	void onConverterInfoFound(const InverterListData &data);

	void onSettingsChanged();

private:
	void updateAddressGenerator();

	void updateDetection();

	void setAutoDetectInternal(bool b);

	QPointer<Settings> mSettings;
	QList<QHostAddress> mDevicesFound;
	QList<FroniusSolarApi *> mApis;
	LocalIpAddressGenerator mAddressGenerator;
	bool mSettingsBusy;
	bool mAutoDetect;
	bool mFullScanRequested;
};

#endif // INVERTER_GATEWAY_H

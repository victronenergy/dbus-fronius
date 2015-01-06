#ifndef INVERTER_GATEWAY_H
#define INVERTER_GATEWAY_H

#include <QHostAddress>
#include <QObject>
#include <QPointer>
#include "local_ip_address_generator.h"

class FroniusSolarApi;
class InverterUpdater;
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
	Q_PROPERTY(int scanProgress READ scanProgress NOTIFY scanProgressChanged)
public:
	InverterGateway(Settings *settings, QObject *parent = 0);

	int scanProgress() const;

	void startDetection();

signals:
	void inverterFound(InverterUpdater *iu);

	void scanProgressChanged();

private slots:
	void onConverterInfoFound(const InverterListData &data);

	void onAutoDetectChanged();

	void onSettingsChanged();

	void onIsConnectedChanged();

private:
	void updateAddressGenerator();

	void updateDetection();

	InverterUpdater *findUpdater(const QString &hostName,
								 const QString &deviceId);

	InverterUpdater *findUpdater(const QString &hostName);

	QPointer<Settings> mSettings;
	QList<InverterUpdater *> mUpdaters;
	QList<FroniusSolarApi *> mApis;
	LocalIpAddressGenerator mAddressGenerator;
	bool mSettingsBusy;
};

#endif // INVERTER_GATEWAY_H

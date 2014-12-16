#ifndef INVERTER_GATEWAY_H
#define INVERTER_GATEWAY_H

#include <QObject>
#include <QtNetwork/QHostAddress>
#include "local_ip_address_generator.h"

class FroniusSolarApi;
class InverterUpdater;
class Settings;
struct InverterListData;

class InverterGateway : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int scanProgress READ scanProgress)
public:
	InverterGateway(Settings *settings, QObject *parent = 0);

	int scanProgress() const;

signals:
	void inverterFound(InverterUpdater &iu);

	void propertyChanged(const QString &property);

private slots:
	void onStartDetection();

	void onConverterInfoFound(const InverterListData &data);

	void onSettingsChanged(const QString &property);

private:
	void updateAddressGenerator();

	InverterUpdater *findUpdater(const QString &hostName,
								 const QString &deviceId);

	InverterUpdater *findUpdater(const QString &hostName);

	Settings *mSettings;
	QList<InverterUpdater *> mUpdaters;
	QList<FroniusSolarApi *> mApis;
	LocalIpAddressGenerator mAddressGenerator;
};

#endif // INVERTER_GATEWAY_H

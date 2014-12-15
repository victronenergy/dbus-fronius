#ifndef INVERTER_GATEWAY_H
#define INVERTER_GATEWAY_H

#include <QObject>
#include <QtNetwork/QHostAddress>
#include "local_ip_address_generator.h"

class InverterUpdater;
class Settings;
struct InverterListData;

class InverterGateway : public QObject
{
	Q_OBJECT
public:
	InverterGateway(Settings *settings, QObject *parent = 0);

signals:
	void inverterFound(InverterUpdater &iu);

private slots:
	void onStartDetection();

	void onConverterInfoFound(const InverterListData &data);

	void onSettingsChanged(const QString &property);

private:
	InverterUpdater *findUpdater(const QString &hostName,
								 const QString &deviceId);

	InverterUpdater *findUpdater(const QString &hostName);

	QList<InverterUpdater *> mUpdaters;
	LocalIpAddressGenerator mAddressGenerator;
};

#endif // INVERTER_GATEWAY_H

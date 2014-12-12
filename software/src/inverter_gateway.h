#ifndef INVERTER_GATEWAY_H
#define INVERTER_GATEWAY_H

#include <QObject>

class FroniusSolarApi;
class InverterInfo;
class InverterUpdater;
class LocalIpAddressGenerator;
class QTimer;
struct InverterInfoData;

class InverterGateway : public QObject
{
	Q_OBJECT
public:
	explicit InverterGateway(QObject *parent = 0);

	~InverterGateway();

signals:
	void inverterFound(InverterUpdater &iu);

private slots:
	void onStartDetection();

	void onConverterInfoFound(const InverterInfoData &data);

private:
	InverterUpdater *findUpdater(const QString &hostName,
								 const QString &deviceId);

	InverterUpdater *findUpdater(const QString &hostName);

	// FroniusSolarApi *mSolarApi;
	QList<InverterUpdater *> mUpdaters;
	LocalIpAddressGenerator *mAddressGenerator;
};

#endif // INVERTER_GATEWAY_H

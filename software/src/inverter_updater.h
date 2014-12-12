#ifndef INVERTER_UPDATER_H
#define INVERTER_UPDATER_H

#include <QObject>

class FroniusSolarApi;
class Inverter;
class QTimer;
struct CommonInverterData;
struct SolarApiReply;
struct ThreePhasesInverterData;

class InverterUpdater : public QObject
{
	Q_OBJECT
public:
	InverterUpdater(Inverter *inverter, QObject *parent = 0);

	Inverter *inverter();

signals:
	void initialized();

private slots:
	void onStartRetrieval();

	void onCommonDataFound(const CommonInverterData &data);

	void onThreePhasesDataFound(const ThreePhasesInverterData &data);

private:
	void scheduleRetrieval();

	void setInitialized();

	Inverter *mInverter;
	FroniusSolarApi *mSolarApi;
	bool mInitialized;
	int mRetryCount;
};

#endif // INVERTER_UPDATER_H

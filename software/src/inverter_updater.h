#ifndef INVERTER_UPDATER_H
#define INVERTER_UPDATER_H

#include <QObject>

class FroniusSolarApi;
class Inverter;
class InverterSettings;
class QTimer;
struct CommonInverterData;
struct ThreePhasesInverterData;

class InverterUpdater : public QObject
{
	Q_OBJECT
public:
	InverterUpdater(Inverter *inverter, InverterSettings *settings,
					QObject *parent = 0);

	Inverter *inverter();

	InverterSettings *settings();

signals:
	void initialized();

private slots:
	void onStartRetrieval();

	void onCommonDataFound(const CommonInverterData &data);

	void onThreePhasesDataFound(const ThreePhasesInverterData &data);

	void onPhaseChanged();

private:
	void scheduleRetrieval();

	void setInitialized();

	Inverter *mInverter;
	InverterSettings *mSettings;
	FroniusSolarApi *mSolarApi;
	bool mInitialized;
	int mRetryCount;
};

#endif // INVERTER_UPDATER_H

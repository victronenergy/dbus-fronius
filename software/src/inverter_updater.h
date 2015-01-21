#ifndef INVERTER_UPDATER_H
#define INVERTER_UPDATER_H

#include <QObject>
#include "fronius_data_processor.h"

class FroniusSolarApi;
class Inverter;
class InverterSettings;
class PowerInfo;
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

	void onSettingsTimer();

	void onConnectionDataChanged();

private:
	void scheduleRetrieval();

	void setInitialized();

	Inverter *mInverter;
	InverterSettings *mSettings;
	FroniusSolarApi *mSolarApi;
	QTimer *mSettingsTimer;
	FroniusDataProcessor mProcessor;
	bool mInitialized;
	int mRetryCount;
};

#endif // INVERTER_UPDATER_H

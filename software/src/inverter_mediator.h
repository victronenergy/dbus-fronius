#ifndef INVERTERMEDIATOR_H
#define INVERTERMEDIATOR_H

#include <QObject>

class Inverter;
class InverterGateway;
class InverterSettings;
class Settings;

class InverterMediator : public QObject
{
	Q_OBJECT
public:
	explicit InverterMediator(Inverter *inverter, InverterGateway *gateway,
							  Settings *settings, QObject *parent = 0);

	bool processNewInverter(Inverter *inverter);

private slots:
	void onSettingsInitialized();

	void onInverterInitialized();

	void onIsActivatedChanged();

	void onIsConnectedChanged();

private:
	bool inverterMatches(Inverter *inverter);

	void startAcquisition();

	Inverter *mInverter;
	InverterSettings *mInverterSettings;
	InverterGateway *mGateway;
	Settings *mSettings;
};

#endif // INVERTERMEDIATOR_H

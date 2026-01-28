#ifndef SOLAREDGE_LIMITER_H
#define SOLAREDGE_LIMITER_H

#include "sunspec_updater.h"

class Inverter;

class SolarEdgeLimiter : public BaseLimiter
{
	Q_OBJECT
public:
	explicit SolarEdgeLimiter(Inverter *parent);

	void onConnected(ModbusTcpClient *client) override;

	void initialize() override;

	ModbusReply *writePowerLimit(double powerLimitPct) override;

	ModbusReply *resetPowerLimit() override;

private slots:
	void onReadMaxPowerCompleted();

	void writeCommands(bool firstCommand = false);

private:
	void initLimiter();

	QList<std::pair<uint16_t, QVector<uint16_t>>> mCommands;
};

#endif

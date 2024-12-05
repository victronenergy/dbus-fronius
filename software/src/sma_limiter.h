#ifndef SMA_LIMITER_H
#define SMA_LIMITER_H

#include "sunspec_updater.h"

class Inverter;

class SmaLimiter : public SunspecLimiter
{
	Q_OBJECT
public:
	explicit SmaLimiter(Inverter *parent);

	void onConnected(ModbusTcpClient *client) override;

	ModbusReply *writePowerLimit(double powerLimitPct) override;

	ModbusReply *resetPowerLimit() override;

private slots:
	void onReadLimitEnabledCompleted();
};

class Sma2018Limiter : public Sunspec2018Limiter
{
	Q_OBJECT
public:
	explicit Sma2018Limiter(Inverter *parent);

	void onConnected(ModbusTcpClient *client) override;

	ModbusReply *writePowerLimit(double powerLimitPct) override;

	ModbusReply *resetPowerLimit() override;

private slots:
	void onReadLimitEnabledCompleted();
};


#endif

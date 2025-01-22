#include "defines.h"
#include "inverter.h"
#include "modbus_tcp_client.h"
#include "sma_limiter.h"

SmaLimiter::SmaLimiter(Inverter *parent) :
	SunspecLimiter(parent)
{
}

void SmaLimiter::onConnected(ModbusTcpClient *client)
{
	BaseLimiter::onConnected(client);
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();

	// Check if WMaxLimPctEna is set, which we can only do if we have
	// the required info model
	if (deviceInfo.immediateControlOffset > 0) {
		// The WMaxLimPctEna register is at offset 14 for model 704, 9 for
		// model 123.
		quint16 offset = deviceInfo.immediateControlModel==704 ? 14 : 9;
		ModbusReply *reply = client->readHoldingRegisters(
			deviceInfo.networkId, deviceInfo.immediateControlOffset + offset, 1);
		connect(reply, SIGNAL(finished()), this,
			SLOT(onReadLimitEnabledCompleted()));
	} else {
		emit initialised(false);
	}
}

void SmaLimiter::onReadLimitEnabledCompleted()
{
	ModbusReply *reply = static_cast<ModbusReply *>(sender());
	reply->deleteLater();
	if (reply->error()) {
		emit initialised(false);
		return;
	}

	QVector<quint16> values = reply->registers();
	emit initialised(values.size() > 0 && values[0] == 1);
}

ModbusReply *SmaLimiter::writePowerLimit(double powerLimitPct)
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	quint16 pct = static_cast<quint16>(qRound(powerLimitPct * deviceInfo.powerLimitScale));
	return mClient->writeSingleHoldingRegister(deviceInfo.networkId, deviceInfo.immediateControlOffset + 5, pct);
}

ModbusReply *SmaLimiter::resetPowerLimit()
{
	// Do nothing
	return 0;
}

Sma2018Limiter::Sma2018Limiter(Inverter *parent) :
	Sunspec2018Limiter(parent)
{
}

void Sma2018Limiter::onConnected(ModbusTcpClient *client)
{
	BaseLimiter::onConnected(client);
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();

	// Check if WMaxLimPctEna is set, which we can only do if we have
	// the required info model
	if (deviceInfo.immediateControlOffset > 0) {
		ModbusReply *reply = client->readHoldingRegisters(
			deviceInfo.networkId, deviceInfo.immediateControlOffset + 14, 1);
		connect(reply, SIGNAL(finished()), this,
			SLOT(onReadLimitEnabledCompleted()));
	} else {
		emit initialised(false);
	}
}

void Sma2018Limiter::onReadLimitEnabledCompleted()
{
	ModbusReply *reply = static_cast<ModbusReply *>(sender());
	reply->deleteLater();
	if (reply->error()) {
		emit initialised(false);
		return;
	}

	QVector<quint16> values = reply->registers();
	emit initialised(values.size() > 0 && values[0] == 1);
}

ModbusReply *Sma2018Limiter::writePowerLimit(double powerLimitPct)
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	quint16 pct = static_cast<quint16>(qRound(powerLimitPct * deviceInfo.powerLimitScale));
	return mClient->writeSingleHoldingRegister(deviceInfo.networkId, deviceInfo.immediateControlOffset + 15, pct);
}

ModbusReply *Sma2018Limiter::resetPowerLimit()
{
	// Do nothing
	return 0;
}

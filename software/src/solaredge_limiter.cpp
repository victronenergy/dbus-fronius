#include "defines.h"
#include "modbus_tcp_client.h"
#include "inverter.h"
#include "logging.h"
#include "solaredge_limiter.h"

// Extended classes relating to SolarEdge specific updating
// ========================================================
enum PowerControl : uint16_t {
	// Enhanced Dynamic Power Control (EDPC) Active & Reactive
	EnableDynamicPowerControl = 0xF300,          //  Uint16,          0 - 1,     [-]
	CommandTimeout = 0xF310,                     //  Uint32, 0 - MAX_UINT32,     [s]  (Default = 60)

	// EDPC - Active
	MaxActivePower = 0xF304,                     // Float32,     Inv rating,     [W]
	FallbackActivePowerLimit = 0xF312,           // Float32,        0 - 100,     [%]  (Default = 100)
	ActivePowerRampUpRate = 0xF318,              // Float32,        0 - 100, [%/min]  (Default = 5, Disable = -1)
	ActivePowerRampDownRate = 0xF31A,            // Float32,        0 - 100, [%/min]  (Default = 5, Disable = -1)
	DynamicActivePowerLimit = 0xF322,            // Float32,        0 - 100,     [%]

	// EDPC - Reactive
	/* add if/when needed */
};

static constexpr float FallbackActivePowerLimitValue = 100; // [%] For timeout on inverter (e.g. communication failure)
static constexpr float PowerLimitDisableValue = 100;        // [%] For timeout on updating powerLimit (venus-os)

template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, bool>::type = true>
QVector<uint16_t> toWords(T value) {
	QVector<uint16_t> words((sizeof(T)+1)/2);
	memcpy(words.data(), &value, sizeof(T));
	return words;
}

SolarEdgeLimiter::SolarEdgeLimiter(Inverter *parent) :
	BaseLimiter(parent)
{
}

void SolarEdgeLimiter::onConnected(ModbusTcpClient *client)
{
	BaseLimiter::onConnected(client);
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();

	// If the maximum power was not obtained from model 704, or model 120,
	// attempt to fetch it from the SolarEdge specific registers.
	// Always fetch maximum power from the SolarEdge specific float32 register.
	ModbusReply *reply = client->readHoldingRegisters(deviceInfo.networkId, MaxActivePower, 2);
	connect(reply, SIGNAL(finished()), this, SLOT(onReadMaxPowerCompleted()));
}

ModbusReply *SolarEdgeLimiter::writePowerLimit(double powerLimitPct)
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	return mClient->writeMultipleHoldingRegisters(deviceInfo.networkId,
		DynamicActivePowerLimit,
		toWords(static_cast<float>(powerLimitPct * 100)));
}

ModbusReply *SolarEdgeLimiter::resetPowerLimit()
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	mInverter->setPowerLimit(deviceInfo.maxPower);
	return mClient->writeMultipleHoldingRegisters(deviceInfo.networkId,
		DynamicActivePowerLimit, toWords(PowerLimitDisableValue));
}

void SolarEdgeLimiter::onReadMaxPowerCompleted()
{
	ModbusReply *reply = static_cast<ModbusReply *>(sender());
	reply->deleteLater();
	if (reply->error()) {
		emit initialised(false);
	} else {
		QVector <quint16> words = reply->registers();
		double value = static_cast<double>(*reinterpret_cast<float *>(words.data()));
		if (value > 0) {
			qInfo() << "Setting 'Ac/MaxPower' and 'Ac/PowerLimit' to" << value << "for SolarEdge Inverter:" << mInverter->location();
			mInverter->setMaxPower(value);
			mInverter->setPowerLimit(value);
			initLimiter();
		} else {
			emit initialised(false);
		}
	}
}

void SolarEdgeLimiter::initLimiter()
{
	mCommands.append({ActivePowerRampUpRate,     toWords(static_cast<float>(100))});
	mCommands.append({ActivePowerRampDownRate,   toWords(static_cast<float>(100))});
	mCommands.append({ActivePowerRampUpRate,     toWords(static_cast<float>(-1))});
	mCommands.append({ActivePowerRampDownRate,   toWords(static_cast<float>(-1))});
	mCommands.append({FallbackActivePowerLimit,  toWords(FallbackActivePowerLimitValue)});
	mCommands.append({CommandTimeout,            toWords(static_cast<uint32_t>(120))}); // 2 minutes
	mCommands.append({EnableDynamicPowerControl, {1}});
	qInfo() << "Writing EDPC settings to SolarEdge Inverter:" << mInverter->location();
	writeCommands(true);
}

void SolarEdgeLimiter::writeCommands(bool firstCommand)
{
	if (!firstCommand) {
		ModbusReply *previousReply = static_cast<ModbusReply *>(sender());
		previousReply->deleteLater();
		if (previousReply->error()) {
			emit initialised(false);
			return;
		}
	}

	if (mCommands.isEmpty()) {
		emit initialised(true);
		return;
	}

	auto cmd = mCommands.takeFirst();
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	ModbusReply *reply = mClient->writeMultipleHoldingRegisters(
		deviceInfo.networkId, cmd.first, cmd.second);
	connect(reply, SIGNAL(finished()), this, SLOT(writeCommands()));
}

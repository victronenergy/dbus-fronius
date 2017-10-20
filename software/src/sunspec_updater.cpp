#include <qnumeric.h>
#include <QsLog.h>
#include <QTimer>
#include "froniussolar_api.h"
#include "data_processor.h"
#include "inverter.h"
#include "sunspec_updater.h"
#include "inverter_settings.h"
#include "modbus_tcp_client.h"
#include "modbus_reply.h"
#include "power_info.h"
#include "sunspec_tools.h"

// The PV inverter will reset the power limit to maximum after this interval. The reset will cause
// the power of the inverter to increase (or stay at its current value), so a large value for the
// timeout is pretty safe.
static const int PowerLimitTimeout = 120;
static const int PowerLimitScale = 100; /// @todo EV This may cause problems with hub4control

SunspecUpdater::SunspecUpdater(Inverter *inverter, InverterSettings *settings, QObject *parent):
	QObject(parent),
	mInverter(inverter),
	mSettings(settings),
	mModbusClient(new ModbusTcpClient(this)),
	mTimer(new QTimer(this)),
	mDataProcessor(new DataProcessor(inverter, settings)),
	mCurrentState(Idle),
	mPowerLimitPct(100),
	mRetryCount(0),
	mWritePowerLimitRequested(false)
{
	Q_ASSERT(inverter != 0);
	connectModbusClient();
	mModbusClient->setTimeout(5000);
	mModbusClient->connectToServer(inverter->hostName());
	connect(
		mInverter, SIGNAL(powerLimitRequested(double)),
		this, SLOT(onPowerLimitRequested(double)));
	mTimer->setSingleShot(true);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
	connect(mSettings, SIGNAL(phaseChanged()), this, SLOT(onPhaseChanged()));
}

void SunspecUpdater::startNextAction(ModbusState state)
{
	mCurrentState = state;
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	switch (mCurrentState) {
	case ReadPowerLimit:
		readHoldingRegisters(deviceInfo.immediateControlOffset + 5, 5);
		break;
	case ReadPowerAndVoltage:
		if (deviceInfo.retrievalMode == ProtocolSunSpecFloat)
			readHoldingRegisters(deviceInfo.inverterModelOffset, 62);
		else
			readHoldingRegisters(deviceInfo.inverterModelOffset, 52);
		break;
	case WritePowerLimit:
	{
		QVector<quint16> values;
		if (mPowerLimitPct < 100) {
			quint16 pct = static_cast<quint16>(qRound(mPowerLimitPct * deviceInfo.powerLimitScale));
			values.append(pct);
			values.append(0); // unused
			values.append(PowerLimitTimeout);
			values.append(0); // unused
			values.append(1); // enabled power throttle mode
			writeMultipleHoldingRegisters(deviceInfo.immediateControlOffset + 5, values);
		} else {
			values.append(0);
			writeMultipleHoldingRegisters(deviceInfo.immediateControlOffset + 9, values);
		}
		break;
	}
	case Idle:
		startIdleTimer();
		break;
	default:
		Q_ASSERT(false);
		break;
	}
}

void SunspecUpdater::startIdleTimer()
{
	mTimer->setInterval(mCurrentState == Idle ? 1000 : 5000);
	mTimer->start();
}

void SunspecUpdater::setInverterState(int sunSpecState)
{
	int froniusState = 0;
	switch (sunSpecState) {
	case 1: // Off
		froniusState = 0;
		break;
	case 2: // Sleeping
	case 6: // Shutting down
	case 8: // Standby
		froniusState = 8;
		break;
	case 3: // Starting
		froniusState = 3;
		break;
	case 4: // MPPT
	case 5: // Throttled
		froniusState = 7;
		break;
	case 7: // Fault
		froniusState = 10;
		break;
	default:
		mInverter->invalidateStatusCode();
		return;
	}
	mInverter->setStatusCode(froniusState);
}

void SunspecUpdater::readHoldingRegisters(quint16 startRegister, quint16 count)
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	ModbusReply *reply = mModbusClient->readHoldingRegisters(deviceInfo.networkId, startRegister, count);
	connect(reply, SIGNAL(finished()), this, SLOT(onReadCompleted()));
}

void SunspecUpdater::writeMultipleHoldingRegisters(quint16 startReg, const QVector<quint16> &values)
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	ModbusReply *reply = mModbusClient->writeMultipleHoldingRegisters(deviceInfo.networkId, startReg, values);
	connect(reply, SIGNAL(finished()), this, SLOT(onWriteCompleted()));
}

bool SunspecUpdater::handleModbusError(ModbusReply *reply)
{
	if (reply->error() == ModbusReply::NoException) {
		mRetryCount = 0;
		return true;
	}
	handleError();
	return false;
}

void SunspecUpdater::handleError()
{
	++mRetryCount;
	if (mRetryCount > 5) {
		mRetryCount = 0;
		emit connectionLost();
	}
	startIdleTimer();
}

void SunspecUpdater::onReadCompleted()
{
	ModbusReply *reply = static_cast<ModbusReply *>(sender());
	reply->deleteLater();
	if (!handleModbusError(reply))
		return;

	QVector<quint16> values = reply->registers();

	mRetryCount = 0;

	ModbusState nextState = mCurrentState;
	switch (mCurrentState) {
	case ReadPowerAndVoltage:
	{
		if (values.isEmpty())
			break;
		int modelId = values[0];
		const DeviceInfo &deviceInfo = mInverter->deviceInfo();
		ProtocolType retrievalMode = modelId > 103 ? ProtocolSunSpecFloat : ProtocolSunSpecIntSf;
		int phaseCount = modelId % 10;
		if (retrievalMode != deviceInfo.retrievalMode || phaseCount != deviceInfo.phaseCount) {
			emit inverterModelChanged();
			nextState = Idle;
			break;
		}
		if (deviceInfo.retrievalMode == ProtocolSunSpecFloat) {
			if (values.size() != 62)
				break;
			/// @todo EV Value and scale at zero seems to indicate an error state...
			double power = getFloat(values, 22);
			if (qIsFinite(power)) {
				CommonInverterData cid;
				cid.acCurrent = getFloat(values, 2);
				cid.acPower = power;
				cid.acVoltage = getFloat(values, 16); /// @todo EV This is phase 1 voltage
				cid.totalEnergy = getFloat(values, 32);
				mDataProcessor->process(cid);

				ThreePhasesInverterData tpid;
				if (deviceInfo.phaseCount > 1) {
					tpid.acCurrentPhase1 = getFloat(values, 4);
					tpid.acCurrentPhase2 = getFloat(values, 6);
					tpid.acCurrentPhase3 = getFloat(values, 8);
					tpid.acVoltagePhase1 = getFloat(values, 16);
					tpid.acVoltagePhase2 = getFloat(values, 18);
					tpid.acVoltagePhase3 = getFloat(values, 20);
					mDataProcessor->process(tpid);
				}
			}
			setInverterState(values[48]);
		} else {
			if (values.size() != 52)
				break;
			/// @todo EV Value and scale at zero seems to indicate an error state...
			double power = getScaledValue(values, 14, 1, 15, true);
			if (qIsFinite(power)) {
				CommonInverterData cid;
				cid.acCurrent = getScaledValue(values, 2, 1, 6, false);
				cid.acPower = power;
				cid.acVoltage = getScaledValue(values, 10, 1, 13, false); /// @todo EV This is phase 1 voltage
				cid.totalEnergy = getScaledValue(values, 24, 2, 26, false);
				mDataProcessor->process(cid);

				ThreePhasesInverterData tpid;
				if (deviceInfo.phaseCount > 1) {
					tpid.acCurrentPhase1 = getScaledValue(values, 3, 1, 6, false);
					tpid.acCurrentPhase2 = getScaledValue(values, 4, 1, 6, false);
					tpid.acCurrentPhase3 = getScaledValue(values, 5, 1, 6, false);
					tpid.acVoltagePhase1 = getScaledValue(values, 10, 1, 13, false);
					tpid.acVoltagePhase2 = getScaledValue(values, 11, 1, 13, false);
					tpid.acVoltagePhase3 = getScaledValue(values, 12, 1, 13, false);
					mDataProcessor->process(tpid);
				}
			}
			setInverterState(values[38]);
		}
		nextState = mWritePowerLimitRequested ? WritePowerLimit : Idle;
		break;
	}
	case ReadPowerLimit:
		if (values.size() == 5) {
			double powerLimit = qQNaN();
			const DeviceInfo &deviceInfo = mInverter->deviceInfo();
			if (deviceInfo.powerLimitScale >= PowerLimitScale) {
				if (values[4] == 1) {
					if (values[0] != 0xFFFF)
						powerLimit = values[0] * deviceInfo.maxPower / deviceInfo.powerLimitScale;
				} else {
					powerLimit = deviceInfo.maxPower;
				}
			}
			mInverter->setPowerLimit(powerLimit);
			nextState = ReadPowerAndVoltage;
		}
		break;
	default:
		Q_ASSERT(false);
		nextState = Init;
		break;
	}
	startNextAction(nextState);
}

void SunspecUpdater::onWriteCompleted()
{
	ModbusReply *reply = static_cast<ModbusReply *>(sender());
	reply->deleteLater();
	mWritePowerLimitRequested = false;
	startNextAction(Start);
}

void SunspecUpdater::onPowerLimitRequested(double value)
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	double powerLimitScale = deviceInfo.powerLimitScale;
	if (powerLimitScale < PowerLimitScale)
		return;
	// An invalid power limit means that power limiting is not supported. So we ignore the request.
	// See comment in the getInitState function.
	if (!qIsFinite(mInverter->powerLimit()))
		return;
	mPowerLimitPct = qBound(0.0, value / deviceInfo.maxPower, 100.0);
	if (mTimer->isActive()) {
		mTimer->stop();
		if (mCurrentState == Idle) {
			startNextAction(WritePowerLimit);
			return; // Skip setting of mWritePowerLimitRequested
		}
		startNextAction(mCurrentState);
	}
	mWritePowerLimitRequested = true;
}

void SunspecUpdater::onConnected()
{
	startNextAction(Init);
}

void SunspecUpdater::onDisconnected()
{
	mCurrentState = Init;
	handleError();
}

void SunspecUpdater::onTimer()
{
	Q_ASSERT(!mTimer->isActive());
	if (mModbusClient->isConnected())
		startNextAction(mCurrentState == Idle ? Start : mCurrentState);
	else
		mModbusClient->connectToServer(mInverter->hostName());
}

void SunspecUpdater::onPhaseChanged()
{
	if (mInverter->deviceInfo().phaseCount > 1)
		return;
	mInverter->l1PowerInfo()->resetValues();
	mInverter->l2PowerInfo()->resetValues();
	mInverter->l3PowerInfo()->resetValues();
}

void SunspecUpdater::connectModbusClient()
{
	connect(mModbusClient, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(mModbusClient, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
}
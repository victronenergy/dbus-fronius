#include <qnumeric.h>
#include <QsLog.h>
#include <QTimer>
#include "inverter.h"
#include "inverter_modbus_updater.h"
#include "modbus_tcp_client.h"
#include "power_info.h"

// The PV inverter will reset the power limit to maximum after this interval. The reset will cause
// the power of the inverter to increase (or stay at its current value), so a large value for the
// timeout is pretty safe.
static const int PowerLimitTimeout = 120;

InverterModbusUpdater::InverterModbusUpdater(Inverter *inverter, QObject *parent):
	QObject(parent),
	mInverter(inverter),
	mModbusClient(new ModbusTcpClient),
	mCurrentState(Idle),
	mPowerLimit(qQNaN()),
	mWritePowerLimitRequested(false),
	mTimer(new QTimer(this))
{
	Q_ASSERT(inverter != 0);
	mModbusClient->connectToServer(inverter->hostName());
	connect(
		mModbusClient, SIGNAL(readHoldingRegistersCompleted(quint8, QList<quint16>)),
		this, SLOT(onReadCompleted(quint8, QList<quint16>)));
	connect(
		mModbusClient, SIGNAL(writeMultipleHoldingRegistersCompleted(quint8, quint16, quint16)),
		this, SLOT(onWriteCompleted(quint8, quint16, quint16)));
	connect(
		mModbusClient, SIGNAL(errorReceived(quint8, quint8, quint8)),
		this, SLOT(onError(quint8, quint8, quint8)));
	connect(
		mModbusClient, SIGNAL(connected()),
		this, SLOT(onConnected()));
	connect(
		mModbusClient, SIGNAL(disconnected()),
		this, SLOT(onDisconnected()));
	connect(
		mInverter, SIGNAL(powerLimitRequested(double)),
		this, SLOT(onPowerLimitRequested(double)));
	mTimer->setSingleShot(true);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
}

void InverterModbusUpdater::startNextAction(ModbusState state)
{
	mCurrentState = state;
	quint8 unitId = static_cast<quint8>(mInverter->id().toInt());
	switch (mCurrentState) {
	case ReadMaxPower:
		mModbusClient->readHoldingRegisters(unitId, 40124, 2);
		break;
	case ReadPowerLimit:
		mModbusClient->readHoldingRegisters(unitId, 40232, 5);
		break;
	case ReadCurrentPower:
		mModbusClient->readHoldingRegisters(unitId, 40083, 2);
		break;
	case WritePowerLimit:
	{
		QList<quint16> values;
		quint16 pct = 100;
		if (qIsFinite(mPowerLimit)) {
			pct = static_cast<quint16>(
				qBound(0, qRound(100 * mPowerLimit / mInverter->maxPower()), 100));
		}
		if (pct < 100) {
			values.append(pct);
			values.append(0); // unused
			values.append(PowerLimitTimeout);
			values.append(0); // unused
			values.append(1); // enabled power throttle mode
			mModbusClient->writeMultipleHoldingRegisters(unitId, 40232, values);
		} else {
			values.append(0);
			mModbusClient->writeMultipleHoldingRegisters(unitId, 40236, values);
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

void InverterModbusUpdater::startIdleTimer()
{
	mTimer->setInterval(mInverter->isConnected() ? 1000 : 30000);
	mTimer->start();
}

void InverterModbusUpdater::onReadCompleted(quint8 unitId, QList<quint16> values)
{
	if (unitId != mInverter->id().toInt())
		return;
	ModbusState nextState = mCurrentState;
	switch (mCurrentState) {
	case ReadMaxPower:
		if (values.size() == 2 && (values[0] != 0 || values[1] != 0)) {
			double maxPower = getScaledValue(values);
			mInverter->setMaxPower(maxPower);
			mInverter->setPowerLimitStepSize(maxPower / 100);
			mInverter->setMinPowerLimit(maxPower / 10);
			nextState = Start;
		}
		break;
	case ReadCurrentPower:
		// Both value set to zero seems to indicate a busy state. If the power is really zero,
		// values[0] will be zero and values[1] non-zero.
		if (values.size() == 2 && (values[0] != 0 || values[1] != 0)) {
			mInverter->meanPowerInfo()->setPower(getScaledValue(values));
			nextState = mWritePowerLimitRequested ? WritePowerLimit : Idle;
		}
		break;
	case ReadPowerLimit:
		if (values.size() == 5) {
			if (values[4] == 1)
				mInverter->setPowerLimit(values[0] * mInverter->maxPower() / 100.0);
			else
				mInverter->setPowerLimit(mInverter->maxPower());
			nextState = ReadCurrentPower;
		}
		break;
	default:
		Q_ASSERT(false);
		nextState = Init;
		break;
	}
	startNextAction(nextState);
}

void InverterModbusUpdater::onWriteCompleted(quint8 unitId, quint16 startReg, quint16 regCount)
{
	Q_UNUSED(startReg)
	Q_UNUSED(regCount)
	if (unitId != mInverter->id().toInt())
		return;
	mWritePowerLimitRequested = false;
	startNextAction(Start);
}

void InverterModbusUpdater::onError(quint8 functionCode, quint8 unitId, quint8 exception)
{
	QLOG_TRACE() << "Modbus TCP error" << mCurrentState << functionCode << unitId << exception;
	startIdleTimer();
}

void InverterModbusUpdater::onPowerLimitRequested(double value)
{
	mPowerLimit = value;
	if (mTimer->isActive()) {
		mTimer->stop();
		startNextAction(mCurrentState == Idle ? WritePowerLimit : mCurrentState);
	} else {
		mWritePowerLimitRequested = true;
	}
}

void InverterModbusUpdater::onConnected()
{
	startNextAction(Init);
}

void InverterModbusUpdater::onDisconnected()
{
	mCurrentState = Init;
	startIdleTimer();
}

void InverterModbusUpdater::onTimer()
{
	if (mModbusClient->isConnected())
		startNextAction(mCurrentState == Idle ? Start : mCurrentState);
	else
		mModbusClient->connectToServer(mInverter->hostName());
}

// static
double InverterModbusUpdater::getScaledValue(const QList<quint16> &values, int offset)
{
	double power = values[offset];
	qint16 e = static_cast<qint16>(values[offset + 1]);
	for (;e > 0; --e) {
		power *= 10;
	}
	for (;e < 0; ++e) {
		power /= 10;
	}
	return power;
}

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
static const int MinPowerLimitScale = 10000;
static const int MaxInitCount = 10;

InverterModbusUpdater::InverterModbusUpdater(Inverter *inverter, QObject *parent):
	QObject(parent),
	mInverter(inverter),
	mModbusClient(new ModbusTcpClient(this)),
	mCurrentState(Idle),
	mPowerLimit(qQNaN()),
	mModelType(0),
	mInitCounter(0),
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
		mModbusClient, SIGNAL(socketErrorReceived(QAbstractSocket::SocketError)),
		this, SLOT(onSocketError(QAbstractSocket::SocketError)));
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

void InverterModbusUpdater::startNextAction(ModbusState state, bool checkReInit)
{
	if (checkReInit) {
		++mInitCounter;
		if (mInitCounter >= MaxInitCount) {
			state = Init;
			mInitCounter = 0;
		}
	}
	mCurrentState = state;
	quint8 unitId = static_cast<quint8>(mInverter->id().toInt());
	switch (mCurrentState) {
	case ReadModelType:
		mModbusClient->readHoldingRegisters(unitId, 215, 1);
		break;
	case ReadMaxPower:
		mModbusClient->readHoldingRegisters(unitId, 40124, 2);
		break;
	case ReadPowerLimitScale:
		mModbusClient->readHoldingRegisters(unitId, 40250, 1);
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
		quint16 pct = mPowerLimitScale;
		if (qIsFinite(mPowerLimit)) {
			pct = static_cast<quint16>(
				qBound(0, qRound(mPowerLimitScale * mPowerLimit / mInverter->maxPower()), mPowerLimitScale));
		}
		if (pct < mPowerLimitScale) {
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
	case WaitForModelType:
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
	mTimer->setInterval((mInverter->isConnected() && mCurrentState != WaitForModelType) ? 1000 : 30000);
	mTimer->start();
}

void InverterModbusUpdater::resetValues()
{
	// Do not reset ac power itself, it will be replaced later when the common inverter data is
	// retrieved using the http api.
	mInverter->setMaxPower(qQNaN());
	mInverter->setMinPowerLimit(qQNaN());
	mInverter->setPowerLimit(qQNaN());
}

void InverterModbusUpdater::setModelType(int type)
{
	if (mModelType == type)
		return;
	mModelType = type;
	QLOG_INFO() << "Modbus TCP mode:" << mModelType;
}

void InverterModbusUpdater::onReadCompleted(quint8 unitId, QList<quint16> values)
{
	if (unitId != mInverter->id().toInt())
		return;
	ModbusState nextState = mCurrentState;
	bool checkReInit = false;
	switch (mCurrentState) {
	case ReadModelType:
		if (values.size() == 1) {
			setModelType(values[0]);
			if (values[0] == 2) {
				nextState = ReadMaxPower;
			} else {
				resetValues();
				nextState = WaitForModelType;
			}
		}
		break;
	case ReadMaxPower:
		if (values.size() == 2 && (values[0] != 0 || values[1] != 0)) {
			double maxPower = getScaledValue(values, 0, false);
			mInverter->setMaxPower(maxPower);
			mInverter->setMinPowerLimit(maxPower / 10);
			nextState = ReadPowerLimitScale;
		}
		break;
	case ReadPowerLimitScale:
		if (values.size() == 1) {
			mPowerLimitScale = 100;
			for (qint16 scale = static_cast<qint16>(values[0]); scale < 0; ++scale)
				mPowerLimitScale *= 10;
			nextState = Start;
		}
		break;
	case ReadCurrentPower:
		// Both value set to zero seems to indicate a busy state. If the power is really zero,
		// values[0] will be zero and values[1] non-zero.
		if (values.size() == 2 && (values[0] != 0 || values[1] != 0)) {
			mInverter->meanPowerInfo()->setPower(getScaledValue(values, 0, true));
			if (mWritePowerLimitRequested) {
				nextState = WritePowerLimit;
			} else {
				nextState = Idle;
				checkReInit = true;
			}
		}
		break;
	case ReadPowerLimit:
		if (values.size() == 5) {
			if (mPowerLimitScale >= MinPowerLimitScale) {
				if (values[4] == 1)
					mInverter->setPowerLimit((values[0] * mInverter->maxPower()) / mPowerLimitScale);
				else
					mInverter->setPowerLimit(mInverter->maxPower());
			} else {
				/// Make the power limit invalid, so the users of the value (eg. hub4control) know
				/// that setting the value is not supported.
				mInverter->setPowerLimit(qQNaN());
			}
			nextState = ReadCurrentPower;
		}
		break;
	default:
		Q_ASSERT(false);
		nextState = Init;
		break;
	}
	startNextAction(nextState, checkReInit);
}

void InverterModbusUpdater::onWriteCompleted(quint8 unitId, quint16 startReg, quint16 regCount)
{
	Q_UNUSED(startReg)
	Q_UNUSED(regCount)
	if (unitId != mInverter->id().toInt())
		return;
	mWritePowerLimitRequested = false;
	startNextAction(Start, true);
}

void InverterModbusUpdater::onError(quint8 functionCode, quint8 unitId, quint8 exception)
{
	QLOG_TRACE() << "Modbus TCP error" << mCurrentState << functionCode << unitId << exception;
	startIdleTimer();
}

void InverterModbusUpdater::onSocketError(QAbstractSocket::SocketError error)
{
	Q_UNUSED(error)
	setModelType(0);
	resetValues();
	if (mTimer->isActive())
		return;
	mTimer->setInterval(30000);
	mTimer->start();
}

void InverterModbusUpdater::onPowerLimitRequested(double value)
{
	if (mPowerLimitScale < MinPowerLimitScale)
		return;
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
		startNextAction(mCurrentState == Idle || mCurrentState == WaitForModelType ? Init : mCurrentState);
	else
		mModbusClient->connectToServer(mInverter->hostName());
}

// static
double InverterModbusUpdater::getScaledValue(const QList<quint16> &values, int offset,
											 bool isSigned)
{
	double v = isSigned ?
		static_cast<double>(static_cast<qint16>(values[offset])) :
		static_cast<double>(values[offset]);
	qint16 s = static_cast<qint16>(values[offset + 1]);
	for (;s > 0; --s)
		v *= 10;
	for (;s < 0; ++s)
		v /= 10;
	return v;
}

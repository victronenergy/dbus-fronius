#include <qnumeric.h>
#include <QsLog.h>
#include <QTimer>
#include <velib/vecan/products.h>
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
// This value used to be bigger to prevent old Fronius firmware from running (the resolution of
// the power limiter was 1%. New Versions support precision of 0.01%. However, since a change in
// the algorithm in hub4control, 1% should only work.
static const int PowerLimitScale = 100;

QList<SunspecUpdater*> SunspecUpdater::mUpdaters;

SunspecUpdater::SunspecUpdater(Inverter *inverter, InverterSettings *settings, QObject *parent):
	QObject(parent),
	mInverter(inverter),
	mSettings(settings),
	mModbusClient(new ModbusTcpClient(this)),
	mTimer(new QTimer(this)),
	mPowerLimitTimer(new QTimer(this)),
	mDataProcessor(new DataProcessor(inverter, settings, this)),
	mCurrentState(Idle),
	mPowerLimitPct(1.0),
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
	mPowerLimitTimer->setSingleShot(true);
	mPowerLimitTimer->setInterval(60000);
	connect(mPowerLimitTimer, SIGNAL(timeout()), this, SLOT(onPowerLimitExpired()));
	connect(mSettings, SIGNAL(phaseChanged()), this, SLOT(onPhaseChanged()));

	mUpdaters.append(this);
}

SunspecUpdater::~SunspecUpdater()
{
	// If the updater is being deleted, the connection is lost. Remove the
	// updater from static member mUpdaters.
	mUpdaters.removeAll(this);
}

void SunspecUpdater::startNextAction(ModbusState state)
{
	mCurrentState = state;
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	switch (mCurrentState) {
	case ReadPowerAndVoltage:
		if (deviceInfo.retrievalMode == ProtocolSunSpecFloat)
			readHoldingRegisters(deviceInfo.inverterModelOffset, 62);
		else
			readHoldingRegisters(deviceInfo.inverterModelOffset, 52);
		break;
	case WritePowerLimit:
	{
		QVector<quint16> values;
		if (mPowerLimitPct < 1.0) {
			quint16 pct = static_cast<quint16>(qRound(mPowerLimitPct * deviceInfo.powerLimitScale));
			values.append(pct);
			values.append(0); // unused
			values.append(PowerLimitTimeout);
			values.append(0); // unused
			values.append(1); // enabled power throttle mode
			writeMultipleHoldingRegisters(deviceInfo.immediateControlOffset + 5, values);
			mInverter->setPowerLimit(mPowerLimitPct * deviceInfo.maxPower);
			mPowerLimitTimer->start();
		} else {
			values.append(0);
			writeMultipleHoldingRegisters(deviceInfo.immediateControlOffset + 9, values);
			mInverter->setPowerLimit(deviceInfo.maxPower);
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
	case SunspecOff:
		froniusState = 0;
		break;
	case SunspecSleeping:
	case SunspecShutdown:
	case SunspecStandby:
		froniusState = 8;
		break;
	case SunspecStarting:
		froniusState = 3;
		break;
	case SunspecMppt:
		froniusState = 11;
		break;
	case SunspecThrottled:
		froniusState = 12;
		break;
	case SunspecFault:
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

		// Let the others know the connection could not be recovered.
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

		// Allow the inverter object to decide if this frame is useable. This
		// allows filtering bad frames on a per-inverter level. We use this
		// for Fronius inverters that send a frame consisting of all zeros,
		// with Status=7, Vendor State=10. By breaking out of the switch,
		// the register will be fetched again immediately. See fronius_inverter.cpp
		// for the implementation.
		if (!mInverter->validateSunspecMonitorFrame(values))
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
			double power = getFloat(values, 22);
			if (qIsFinite(power)) {
				CommonInverterData cid;
				cid.acCurrent = getFloat(values, 2);
				cid.acPower = power;
				// sunspec does not provide a voltage for the system as a whole. This does not
				// make a lot of sense. Since previous versions of dbus-fronius published this
				// value (retrieved via the Solar API) we use the value from phase 1.
				cid.acVoltage = getFloat(values, 16);
				cid.totalEnergy = getFloat(values, 32);
				mDataProcessor->process(cid);

				if (deviceInfo.phaseCount > 1) {
					ThreePhasesInverterData tpid;
					tpid.acCurrentPhase1 = getFloat(values, 4);
					tpid.acCurrentPhase2 = getFloat(values, 6);
					tpid.acCurrentPhase3 = getFloat(values, 8);
					tpid.acVoltagePhase1 = getFloat(values, 16);
					tpid.acVoltagePhase2 = getFloat(values, 18);
					tpid.acVoltagePhase3 = getFloat(values, 20);
					mDataProcessor->process(tpid);
				} else if (mSettings->phase() == MultiPhase) {
					// A single phase inverter used as a Multiphase
					// generator. This only makes sense in a split-phase
					// system. Typical in North America, and fully
					// supported by Fronius.
					updateSplitPhase(cid.acPower/2, cid.totalEnergy/2);
				}
			}
			setInverterState(values[48]);
		} else {
			if (values.size() != 52)
				break;
			// In older versions of the Fronius firmware, power value and its scaling were sometimes
			// 0 even when it was obvious that the value should have been different. It seemed to
			// be indicating some kind of error situation.
			double power = getScaledValue(values, 14, 1, 15, true);
			if (qIsFinite(power)) {
				CommonInverterData cid;
				cid.acCurrent = getScaledValue(values, 2, 1, 6, false);
				cid.acPower = power;
				// sunspec does not provide a voltage for the system as a whole. This does not
				// make a lot of sense. Since previous versions of dbus-fronius published this
				// value (retrieved via the Solar API) we use the value from phase 1.
				cid.acVoltage = getScaledValue(values, 10, 1, 13, false);
				cid.totalEnergy = getScaledValue(values, 24, 2, 26, false);
				mDataProcessor->process(cid);

				if (deviceInfo.phaseCount > 1) {
					ThreePhasesInverterData tpid;
					tpid.acCurrentPhase1 = getScaledValue(values, 3, 1, 6, false);
					tpid.acCurrentPhase2 = getScaledValue(values, 4, 1, 6, false);
					tpid.acCurrentPhase3 = getScaledValue(values, 5, 1, 6, false);
					tpid.acVoltagePhase1 = getScaledValue(values, 10, 1, 13, false);
					tpid.acVoltagePhase2 = getScaledValue(values, 11, 1, 13, false);
					tpid.acVoltagePhase3 = getScaledValue(values, 12, 1, 13, false);
					mDataProcessor->process(tpid);
				} else if (mSettings->phase() == MultiPhase) {
					// A single phase inverter used as a Multiphase
					// generator. This only makes sense in a split-phase
					// system. Typical in North America, and fully
					// supported by Fronius.
					updateSplitPhase(cid.acPower/2, cid.totalEnergy/2);
				}
			}
			setInverterState(values[38]);
		}
		nextState = mWritePowerLimitRequested ? WritePowerLimit : Idle;
		break;
	}
	default:
		Q_ASSERT(false);
		nextState = ReadPowerAndVoltage;
		break;
	}
	startNextAction(nextState);
}

void SunspecUpdater::onWriteCompleted()
{
	ModbusReply *reply = static_cast<ModbusReply *>(sender());
	reply->deleteLater();
	mWritePowerLimitRequested = false;
	startNextAction(ReadPowerAndVoltage);
}

void SunspecUpdater::onPowerLimitRequested(double value)
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	double powerLimitScale = deviceInfo.powerLimitScale;
	if (powerLimitScale < PowerLimitScale)
		return;
	// An invalid power limit means that power limiting is not supported. So we ignore the request.
	if (!qIsFinite(mInverter->powerLimit()))
		return;
	mPowerLimitPct = qBound(0.0, value / deviceInfo.maxPower, 1.0);
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
	startNextAction(ReadPowerAndVoltage);
}

void SunspecUpdater::onDisconnected()
{
	mCurrentState = ReadPowerAndVoltage;
	handleError();
}

void SunspecUpdater::onTimer()
{
	Q_ASSERT(!mTimer->isActive());
	if (mModbusClient->isConnected())
		startNextAction(mCurrentState == Idle ? ReadPowerAndVoltage : mCurrentState);
	else
		mModbusClient->connectToServer(mInverter->hostName());
}

void SunspecUpdater::onPowerLimitExpired()
{
	onPowerLimitRequested(mInverter->deviceInfo().maxPower);
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


void SunspecUpdater::updateSplitPhase(double power, double energy)
{
	PowerInfo *l1 = mInverter->getPowerInfo(PhaseL1);
	PowerInfo *l2 = mInverter->getPowerInfo(PhaseL2);
	l1->setPower(power);
	l2->setPower(power);
	l1->setTotalEnergy(energy);
	l2->setTotalEnergy(energy);
}

bool SunspecUpdater::hasConnectionTo(QString host, int id)
{
	foreach (SunspecUpdater *u, mUpdaters) {
		if ((host == u->mInverter->hostName()) && (id == u->mInverter->networkId())) {
			return true;
		}
	}
	return false;
}

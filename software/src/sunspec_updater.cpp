#include <qnumeric.h>
#include <QTimer>
#include "products.h"
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

QList<SunspecUpdater*> SunspecUpdater::mUpdaters;

SunspecUpdater::SunspecUpdater(BaseLimiter *limiter, Inverter *inverter, InverterSettings *settings, QObject *parent):
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
	mWritePowerLimitRequested(false),
	mLimiter(limiter)
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
		readPowerAndVoltage();
		break;
	case WritePowerLimit:
	{
		if (writePowerLimit(mPowerLimitPct)) {
			mInverter->setPowerLimit(mPowerLimitPct * deviceInfo.maxPower);
			mPowerLimitTimer->start();
		} else {
			startNextAction(ReadPowerAndVoltage);
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

		if (!parsePowerAndVoltage(values)) {
			nextState = Idle;
			break;
		}

		nextState = mWritePowerLimitRequested ? WritePowerLimit : Idle;
		mWritePowerLimitRequested = false;
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
	startNextAction(ReadPowerAndVoltage);
}

void SunspecUpdater::onPowerLimitRequested(double value)
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
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
	if (mLimiter) {
		// Make sure no signals survive from last time
		disconnect(mLimiter, SIGNAL(initialised(bool)));
		connect(mLimiter, SIGNAL(initialised(bool)), this, SLOT(onLimiterInitialised(bool)));
		mLimiter->onConnected(mModbusClient);
	} else {
		startNextAction(ReadPowerAndVoltage);
	}
}

void SunspecUpdater::onLimiterInitialised(bool success)
{
	// If it has sunspec model 123 or 704, or some other limiter
	// successfully initialised, enable the power limiter and
	// initialise it to maxPower.
	if (success) {
		const DeviceInfo &deviceInfo = mInverter->deviceInfo();
		if (deviceInfo.deviceType != 0 || // fronius
			deviceInfo.productId == VE_PROD_ID_PV_INVERTER_ABB) {
			mInverter->setPowerLimit(deviceInfo.maxPower);
			mSettings->setLimiterSupported(LimiterForcedEnabled);
		} else {
			mSettings->setLimiterSupported(LimiterEnabled);
			mInverter->setPowerLimit(
				mSettings->enableLimiter() ? deviceInfo.maxPower : qQNaN());
			connect(mSettings, SIGNAL(enableLimiterChanged()), this, SLOT(onEnableLimiterChanged()));
		}
	} else {
		mSettings->setLimiterSupported(LimiterDisabled);
		mInverter->setPowerLimit(qQNaN());
	}

	startNextAction(ReadPowerAndVoltage);
}

void SunspecUpdater::onEnableLimiterChanged()
{
	if (mSettings->enableLimiter()) {
		mInverter->setPowerLimit(mInverter->deviceInfo().maxPower);
	} else {
		resetPowerLimit();
		mInverter->setPowerLimit(qQNaN());
		mPowerLimitTimer->stop(); // Cancel any pending timeouts
	}
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
	// Cancel limiter by resetting WMaxLim_Ena to zero.  Depending on the
	// PV-inverter and its configuration, this will either cause it to go to
	// full power, or to go to zero.
	resetPowerLimit();
	mInverter->setPowerLimit(mInverter->deviceInfo().maxPower);
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

void SunspecUpdater::readPowerAndVoltage()
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	if (deviceInfo.retrievalMode == ProtocolSunSpecFloat)
		readHoldingRegisters(deviceInfo.inverterModelOffset, 62);
	else
		readHoldingRegisters(deviceInfo.inverterModelOffset, 52);
}

bool SunspecUpdater::writePowerLimit(double powerLimitPct)
{
	if (!mLimiter)
		return false;
	ModbusReply *reply = mLimiter->writePowerLimit(powerLimitPct);
	connect(reply, SIGNAL(finished()), this, SLOT(onWriteCompleted()));
	return true;
}

bool SunspecUpdater::resetPowerLimit()
{
	if (!mLimiter)
		return false;

	ModbusReply *reply = mLimiter->resetPowerLimit();
	if (reply == 0)
		return false;

	connect(reply, SIGNAL(finished()), this, SLOT(onWriteCompleted()));
	return true;
}

bool SunspecUpdater::parsePowerAndVoltage(QVector<quint16> values)
{
	int modelId = values[0];
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	ProtocolType retrievalMode = modelId > 103 ? ProtocolSunSpecFloat : ProtocolSunSpecIntSf;
	int phaseCount = modelId % 10;
	if (retrievalMode != deviceInfo.retrievalMode || phaseCount != deviceInfo.phaseCount) {
		emit inverterModelChanged();
		return false; // go to idle
	}
	if (deviceInfo.retrievalMode == ProtocolSunSpecFloat) {
		if (values.size() != 62)
			return false;
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
			return false;
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
	return true;
}

// Extended classes relating to Fronius specific updating
// ======================================================
// Fronius inverters send a null payload during certain solar net timeouts. We
// want to filter for those.
static const QVector<quint16> FroniusNullFrame = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7 };

FroniusSunspecUpdater::FroniusSunspecUpdater(BaseLimiter *limiter, Inverter *inverter, InverterSettings *settings, QObject *parent):
	SunspecUpdater(limiter, inverter, settings, parent)
{
}

bool FroniusSunspecUpdater::parsePowerAndVoltage(QVector<quint16> values)
{
	// Filter data for Fronius inverters that send a frame consisting of all
	// zeros, with Status=7. By returning true, the register will be fetched
	// again immediately.
	if (inverter()->deviceInfo().retrievalMode == ProtocolSunSpecIntSf &&
			values.mid(2, 37) == FroniusNullFrame) {
		qInfo() << "Fronius Null-frame detected" << values;
		return true;
	}

	return SunspecUpdater::parsePowerAndVoltage(values);
}


// Extended classes for 700-series models, for Sunspec > 2018
// ==========================================================
Sunspec2018Updater::Sunspec2018Updater(BaseLimiter *limiter, Inverter *inverter, InverterSettings *settings, QObject *parent):
	SunspecUpdater(limiter, inverter, settings, parent)
{
}

void Sunspec2018Updater::readPowerAndVoltage()
{
	// Read 121 values. The model is 153 long, too long for a single modbus
	// request, This is enough to get everything we care about.
	readHoldingRegisters(inverter()->deviceInfo().inverterModelOffset, 121);
}

bool Sunspec2018Updater::parsePowerAndVoltage(QVector<quint16> values)
{
	if (values.size() != 121)
		return false;

	CommonInverterData cid;
	cid.acPower = getScaledValue(values, 10, 1, 116, true);
	cid.acCurrent = getScaledValue(values, 14, 1, 113, true);
	cid.acVoltage = getScaledValue(values, 16, 1, 114, false);

	cid.totalEnergy = getScaledValue(values, 19, 4, 120, false);
	processor()->process(cid);

	if (inverter()->deviceInfo().phaseCount > 1) {
		ThreePhasesInverterData tpid;
		tpid.acCurrentPhase1 = getScaledValue(values, 45, 1, 113, true);
		tpid.acCurrentPhase2 = getScaledValue(values, 68, 1, 113, true);
		tpid.acCurrentPhase3 = getScaledValue(values, 91, 1, 113, true);

		tpid.acVoltagePhase1 = getScaledValue(values, 47, 1, 114, false);
		tpid.acVoltagePhase2 = getScaledValue(values, 70, 1, 114, false);
		tpid.acVoltagePhase3 = getScaledValue(values, 93, 1, 114, false);
		processor()->process(tpid);
	} else if (settings()->phase() == MultiPhase) {
		// A single phase inverter across phases, in North America.
		updateSplitPhase(cid.acPower/2, cid.totalEnergy/2);
	}

	// +1 because 2018 enum is literally off by one from the earlier spec
	setInverterState(values[4] + 1);
	return true;
}

BaseLimiter::BaseLimiter(Inverter *parent) :
	QObject(parent),
	mInverter(parent)
{
}

void BaseLimiter::onConnected(ModbusTcpClient *client)
{
	mClient = client;
}

// Limiter for model 123 (basic sunspec limiter)
SunspecLimiter::SunspecLimiter(Inverter *parent) :
	BaseLimiter(parent)
{
}

void SunspecLimiter::onConnected(ModbusTcpClient *client)
{
	BaseLimiter::onConnected(client);

	// If model 123 exists, this will be non-zero.
	emit initialised(mInverter->deviceInfo().powerLimitScale > 0);
}

ModbusReply *SunspecLimiter::writePowerLimit(double powerLimitPct)
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();

	QVector<quint16> values;
	quint16 pct = static_cast<quint16>(qRound(powerLimitPct * deviceInfo.powerLimitScale));
	values.append(pct);
	values.append(0); // unused
	values.append(PowerLimitTimeout);
	values.append(0); // unused
	values.append(1); // enabled power throttle mode
	return mClient->writeMultipleHoldingRegisters(deviceInfo.networkId, deviceInfo.immediateControlOffset + 5, values);
}

ModbusReply *SunspecLimiter::resetPowerLimit()
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	return mClient->writeMultipleHoldingRegisters(deviceInfo.networkId, deviceInfo.immediateControlOffset + 9, QVector<quint16>() << 0);
}

// Limiter for model 704 (2018 sunspec limiter)
Sunspec2018Limiter::Sunspec2018Limiter(Inverter *parent) :
	BaseLimiter(parent)
{
}

void Sunspec2018Limiter::onConnected(ModbusTcpClient *client)
{
	BaseLimiter::onConnected(client);

	// If model 704 exists, this will be non-zero.
	emit initialised(mInverter->deviceInfo().powerLimitScale > 0);
}

ModbusReply *Sunspec2018Limiter::writePowerLimit(double powerLimitPct)
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();

	QVector<quint16> values;
	quint16 pct = static_cast<quint16>(qRound(powerLimitPct * deviceInfo.powerLimitScale));
	values.append(1); // WMaxLimPctEna
	values.append(pct); // WMaxLimPct
	values.append(0); // WMaxLimPctRvrt, revert to 0%
	values.append(1); // WMaxLimPctEnaRvrt, enable reverting to 0%
	values.append(PowerLimitTimeout); // WMaxLimPctRvrtTms
	return mClient->writeMultipleHoldingRegisters(deviceInfo.networkId, deviceInfo.immediateControlOffset + 14, values);
}

ModbusReply *Sunspec2018Limiter::resetPowerLimit()
{
	const DeviceInfo &deviceInfo = mInverter->deviceInfo();
	return mClient->writeMultipleHoldingRegisters(deviceInfo.networkId, deviceInfo.immediateControlOffset + 14, QVector<quint16>() << 0);
}

#include <limits>
#include <QsLog.h>
#include <QTimer>
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "inverter_updater.h"
#include "power_info.h"

static const int UpdateInterval = 5000;

InverterUpdater::InverterUpdater(Inverter *inverter, InverterSettings *settings,
								 QObject *parent):
	QObject(parent),
	mInverter(inverter),
	mSettings(settings),
	mSolarApi(new FroniusSolarApi(inverter->hostName(), inverter->port(), this)),
	mInitialized(false),
	mRetryCount(0)
{
	Q_ASSERT(inverter != 0);
	Q_ASSERT(mSettings != 0);
	connect(
		mSolarApi, SIGNAL(commonDataFound(const CommonInverterData &)),
		this, SLOT(onCommonDataFound(const CommonInverterData &)));
	connect(
		mSolarApi, SIGNAL(threePhasesDataFound(const ThreePhasesInverterData &)),
		this, SLOT(onThreePhasesDataFound(const ThreePhasesInverterData &)));
	connect(
		mSettings, SIGNAL(phaseChanged()),
		this, SLOT(onPhaseChanged()));
	onStartRetrieval();
}

Inverter *InverterUpdater::inverter()
{
	return mInverter;
}

InverterSettings *InverterUpdater::settings()
{
	return mSettings;
}

void InverterUpdater::onStartRetrieval()
{
	mSolarApi->getCommonDataAsync(mInverter->id());
}

void InverterUpdater::onCommonDataFound(const CommonInverterData &data)
{
	switch (data.error)
	{
	case SolarApiReply::NoError:
	{
		PowerInfo *pi = mInverter->meanPowerInfo();
		pi->setCurrent(data.acCurrent);
		pi->setVoltage(data.acVoltage);
		pi->setPower(data.acPower);
		PowerInfo *li = 0;
		switch (mSettings->phase()) {
		case InverterSettings::L1:
			li = mInverter->l1PowerInfo();
			break;
		case InverterSettings::L2:
			li = mInverter->l2PowerInfo();
			break;
		case InverterSettings::L3:
			li = mInverter->l3PowerInfo();
			break;
		case InverterSettings::AllPhases:
			// Do nothing. l1/l2/l3 data will be set in handling of
			// getThreePhasesInverterData.
			break;
		default:
			Q_ASSERT(false);
			break;
		}
		if (li != 0) {
			li->setCurrent(data.acCurrent);
			li->setVoltage(data.acVoltage);
			li->setPower(data.acPower);
		}
		if (!mInitialized || mInverter->supports3Phases())
		{
			mRetryCount = 0;
			mSolarApi->getThreePhasesInverterDataAsync(mInverter->id());
		} else {
			mInverter->setIsConnected(true);
			scheduleRetrieval();
		}
	}
		break;
	case SolarApiReply::NetworkError:
		++mRetryCount;
		if (mRetryCount == 5)
		{
			mInverter->setIsConnected(false);
			mRetryCount = 0;
		}
		scheduleRetrieval();
		break;
	case SolarApiReply::ApiError:
		// Inverter does not support common data retrieval?
		///	@todo EV call setInitialized?
		mInverter->setIsConnected(false);
		mInverter->setSupports3Phases(false);
		if (mSettings->phase() == InverterSettings::AllPhases) {
			QLOG_WARN() << "Reset inverter settings from All Phases to L1."
						   "Should not happen, because Fronius devices are"
						   "either single phased to 3 phased.";
			mSettings->setPhase(InverterSettings::L1);
		}
		break;
	}
}

void InverterUpdater::onThreePhasesDataFound(const ThreePhasesInverterData &data)
{
	switch (data.error)
	{
	case SolarApiReply::NoError:
	{
		PowerInfo *l1 = mInverter->l1PowerInfo();
		l1->setCurrent(data.acCurrentPhase1);
		l1->setVoltage(data.acVoltagePhase1);
		l1->setPower(std::numeric_limits<double>::quiet_NaN());

		PowerInfo *l2 = mInverter->l2PowerInfo();
		l2->setCurrent(data.acCurrentPhase2);
		l2->setVoltage(data.acVoltagePhase2);
		l2->setPower(std::numeric_limits<double>::quiet_NaN());

		PowerInfo *l3 = mInverter->l3PowerInfo();
		l3->setCurrent(data.acCurrentPhase3);
		l3->setVoltage(data.acVoltagePhase3);
		l3->setPower(std::numeric_limits<double>::quiet_NaN());

		mInverter->setIsConnected(true);
		/// @todo EV remote the Supports3Phases property because it duplicates
		/// the InverterSettings::phase property
		mInverter->setSupports3Phases(true);
		mSettings->setPhase(InverterSettings::AllPhases);
		mRetryCount = 0;
		setInitialized();
	}
		break;
	case SolarApiReply::NetworkError:
		++mRetryCount;
		if (mRetryCount == 5)
		{
			mInverter->setIsConnected(false);
			mRetryCount = 0;
		}
		break;
	case SolarApiReply::ApiError:
		// Inverter does not support 3 phases?
		mInverter->setIsConnected(true);
		setInitialized();
		break;
	}
	scheduleRetrieval();
}

void InverterUpdater::onPhaseChanged()
{
	if (mSettings->phase() == InverterSettings::AllPhases)
		return;
	mInverter->l1PowerInfo()->resetValues();
	mInverter->l2PowerInfo()->resetValues();
	mInverter->l3PowerInfo()->resetValues();
}

void InverterUpdater::scheduleRetrieval()
{
	QTimer::singleShot(UpdateInterval, this, SLOT(onStartRetrieval()));
}

void InverterUpdater::setInitialized()
{
	if (!mInitialized)
	{
		emit initialized();
		mInitialized = true;
	}
}

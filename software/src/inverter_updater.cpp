#include <QsLog.h>
#include <QTimer>
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "inverter_updater.h"
#include "power_info.h"

static const int UpdateInterval = 5000;
static const int UpdateSettingsInterval = 10 * 60 * 1000;

InverterUpdater::InverterUpdater(Inverter *inverter, InverterSettings *settings,
								 QObject *parent):
	QObject(parent),
	mInverter(inverter),
	mSettings(settings),
	mSolarApi(new FroniusSolarApi(inverter->hostName(), inverter->port(), this)),
	mSettingsTimer(new QTimer(this)),
	mProcessor(inverter, settings),
	mInitialized(false),
	mRetryCount(0)
{
	Q_ASSERT(inverter != 0);
	Q_ASSERT(mSettings != 0);
	mSettingsTimer->setInterval(UpdateSettingsInterval);
	mSettingsTimer->start();
	connect(
		mSolarApi, SIGNAL(commonDataFound(const CommonInverterData &)),
		this, SLOT(onCommonDataFound(const CommonInverterData &)));
	connect(
		mSolarApi, SIGNAL(threePhasesDataFound(const ThreePhasesInverterData &)),
		this, SLOT(onThreePhasesDataFound(const ThreePhasesInverterData &)));
	connect(
		mSettings, SIGNAL(phaseChanged()),
		this, SLOT(onPhaseChanged()));
	connect(
		mSettingsTimer, SIGNAL(timeout()),
		this, SLOT(onSettingsTimer()));
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
		mProcessor.process(data, mInitialized);
		if (!mInitialized || mSettings->phase() == ThreePhases)
		{
			mRetryCount = 0;
			mSolarApi->getThreePhasesInverterDataAsync(mInverter->id());
		} else {
			mInverter->setIsConnected(true);
			scheduleRetrieval();
		}
		QString msg;
		if (data.statusCode >= 0 && data.statusCode <= 6) {
			msg = tr("Startup");
		} else {
			switch (data.statusCode) {
			case 7:
				msg = tr("Running");
				break;
			case 8:
				msg	= tr("Standby");
				break;
			case 9:
				msg = tr("Boot loading");
				break;
			case 10:
				msg = tr("Error");
				break;
			default:
				msg	= tr("Unknown");
				break;
			}
		}
		mInverter->setStatus(QString("%1 (%2/%3)").
			  arg(msg).
			  arg(data.statusCode).
			  arg(data.errorCode));
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
		if (mSettings->phase() == ThreePhases) {
			QLOG_WARN() << "Reset inverter settings from All Phases to L1."
						   "Should not happen, because Fronius devices are"
						   "either single phased to 3 phased.";
			mSettings->setPhase(PhaseL1);
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
		mProcessor.process(data);
		mInverter->setIsConnected(true);
		mSettings->setPhase(ThreePhases);
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
	if (mSettings->phase() == ThreePhases)
		return;
	mInverter->l1PowerInfo()->resetValues();
	mInverter->l2PowerInfo()->resetValues();
	mInverter->l3PowerInfo()->resetValues();
}

void InverterUpdater::onSettingsTimer()
{
	mProcessor.updateEnergySettings();
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

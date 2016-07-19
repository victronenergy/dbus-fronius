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
	Q_ASSERT(settings != 0);
	Q_ASSERT(inverter->uniqueId() == settings->uniqueId());
	Q_ASSERT(inverter->deviceType() == settings->deviceType());
	Q_ASSERT((inverter->phaseCount() > 1) == (settings->phase() == MultiPhase));
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
	connect(
		mInverter, SIGNAL(hostNameChanged()),
		this, SLOT(onConnectionDataChanged()));
	connect(
		mInverter, SIGNAL(portChanged()),
		this, SLOT(onConnectionDataChanged()));
	mSettingsTimer->setInterval(UpdateSettingsInterval);
	mSettingsTimer->start();
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
		mProcessor.process(data);
		mRetryCount = 0;
		if (mInverter->phaseCount() > 1) {
			mSolarApi->getThreePhasesInverterDataAsync(mInverter->id());
		} else {
			mInverter->setIsConnected(true);
			setInitialized();
			scheduleRetrieval();
		}
		mInverter->setStatusCode(data.statusCode);
		mInverter->setErrorCode(data.errorCode);
		break;
	case SolarApiReply::NetworkError:
		QLOG_DEBUG() << "[Solar API] Network error: " << data.errorMessage;
		handleError();
		scheduleRetrieval();
		break;
	case SolarApiReply::ApiError:
		QLOG_DEBUG() << "[Solar API] CommonInverterData retrieval error:" << data.errorMessage;
		handleError();
		scheduleRetrieval();
		break;
	default:
		QLOG_DEBUG() << "[Solar API] Unknown error" << data.error << data.errorMessage;
		break;
	}
}

void InverterUpdater::onThreePhasesDataFound(const ThreePhasesInverterData &data)
{
	switch (data.error)
	{
	case SolarApiReply::NoError:
		Q_ASSERT(mInverter->phaseCount() > 1 && mSettings->phase() == MultiPhase);
		mProcessor.process(data);
		mRetryCount = 0;
		mInverter->setIsConnected(true);
		setInitialized();
		break;
	case SolarApiReply::NetworkError:
		QLOG_DEBUG() << "[Solar API] Network error: " << data.errorMessage;
		handleError();
		break;
	case SolarApiReply::ApiError:
		QLOG_DEBUG() << "[Solar API] Fronius 3Phase inverter data retrieval error:"
					 << data.errorMessage;
		handleError();
		break;
	default:
		QLOG_DEBUG() << "[Solar API] Unknown error" << data.error << data.errorMessage;
		break;
	}
	scheduleRetrieval();
}

void InverterUpdater::onPhaseChanged()
{
	if (mInverter->phaseCount() > 1)
		return;
	mInverter->l1PowerInfo()->resetValues();
	mInverter->l2PowerInfo()->resetValues();
	mInverter->l3PowerInfo()->resetValues();
}

void InverterUpdater::onSettingsTimer()
{
	mProcessor.updateEnergySettings();
}

void InverterUpdater::onConnectionDataChanged()
{
	mSolarApi->setHostName(mInverter->hostName());
	mSolarApi->setPort(mInverter->port());
}

void InverterUpdater::scheduleRetrieval()
{
	QTimer::singleShot(UpdateInterval, this, SLOT(onStartRetrieval()));
}

void InverterUpdater::setInitialized()
{
	if (!mInitialized) {
		mInitialized = true;
		emit initialized();
	}
}

void InverterUpdater::handleError()
{
	++mRetryCount;
	if (mRetryCount == 5) {
		mInverter->setIsConnected(false);
		mInverter->resetValues();
		mRetryCount = 0;
	}
}

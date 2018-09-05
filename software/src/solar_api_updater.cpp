#include <QsLog.h>
#include <QTimer>
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "solar_api_updater.h"
#include "power_info.h"

static const int UpdateInterval = 5000;
static const int UpdateSettingsInterval = 10 * 60 * 1000;

SolarApiUpdater::SolarApiUpdater(Inverter *inverter, InverterSettings *settings, QObject *parent):
	QObject(parent),
	mInverter(inverter),
	mSettings(settings),
	mSolarApi(new FroniusSolarApi(inverter->hostName(), inverter->port(), this)),
	mSettingsTimer(new QTimer(this)),
	mProcessor(inverter, settings, this),
	mInitialized(false),
	mRetryCount(0)
{
	Q_ASSERT(inverter != 0);
	Q_ASSERT(settings != 0);
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

Inverter *SolarApiUpdater::inverter()
{
	return mInverter;
}

InverterSettings *SolarApiUpdater::settings()
{
	return mSettings;
}

void SolarApiUpdater::onStartRetrieval()
{
	mSolarApi->getCommonDataAsync(mInverter->deviceInfo().networkId);
}

void SolarApiUpdater::onCommonDataFound(const CommonInverterData &data)
{
	switch (data.error)
	{
	case SolarApiReply::NoError:
	{
		mProcessor.process(data);
		mRetryCount = 0;
		const DeviceInfo &deviceInfo = mInverter->deviceInfo();
		if (deviceInfo.phaseCount > 1) {
			mSolarApi->getThreePhasesInverterDataAsync(deviceInfo.networkId);
		} else {
			setInitialized();
			scheduleRetrieval();
		}
		mInverter->setStatusCode(data.statusCode);
		mInverter->setErrorCode(data.errorCode);
		break;
	}
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

void SolarApiUpdater::onThreePhasesDataFound(const ThreePhasesInverterData &data)
{
	switch (data.error)
	{
	case SolarApiReply::NoError:
		mProcessor.process(data);
		mRetryCount = 0;
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

void SolarApiUpdater::onPhaseChanged()
{
	if (mInverter->deviceInfo().phaseCount > 1)
		return;
	mInverter->l1PowerInfo()->resetValues();
	mInverter->l2PowerInfo()->resetValues();
	mInverter->l3PowerInfo()->resetValues();
}

void SolarApiUpdater::onSettingsTimer()
{
	mProcessor.updateEnergySettings();
}

void SolarApiUpdater::onConnectionDataChanged()
{
	mSolarApi->setHostName(mInverter->hostName());
	mSolarApi->setPort(mInverter->port());
}

void SolarApiUpdater::scheduleRetrieval()
{
	QTimer::singleShot(UpdateInterval, this, SLOT(onStartRetrieval()));
}

void SolarApiUpdater::setInitialized()
{
	if (!mInitialized) {
		mInitialized = true;
		emit initialized();
	}
}

void SolarApiUpdater::handleError()
{
	++mRetryCount;
	if (mRetryCount == 5) {
		emit connectionLost();
		mRetryCount = 0;
	}
}

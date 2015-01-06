#include <QTimer>
#include "froniussolar_api.h"
#include "inverter_updater.h"
#include "inverter.h"
#include "power_info.h"

static const int UpdateInterval = 5000;

InverterUpdater::InverterUpdater(Inverter *inverter, QObject *parent):
	QObject(parent),
	mInverter(inverter),
	mSolarApi(new FroniusSolarApi(inverter->hostName(), inverter->port(), this)),
	mInitialized(false),
	mRetryCount(0)
{
	Q_ASSERT(inverter != 0);
	connect(
		mSolarApi, SIGNAL(commonDataFound(const CommonInverterData &)),
		this, SLOT(onCommonDataFound(const CommonInverterData &)));
	connect(
		mSolarApi, SIGNAL(threePhasesDataFound(const ThreePhasesInverterData &)),
		this, SLOT(onThreePhasesDataFound(const ThreePhasesInverterData &)));
	onStartRetrieval();
}

Inverter *InverterUpdater::inverter()
{
	return mInverter;
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

		PowerInfo *l2 = mInverter->l2PowerInfo();
		l2->setCurrent(data.acCurrentPhase2);
		l2->setVoltage(data.acVoltagePhase2);

		PowerInfo *l3 = mInverter->l3PowerInfo();
		l3->setCurrent(data.acCurrentPhase3);
		l3->setVoltage(data.acVoltagePhase3);
		mInverter->setIsConnected(true);
		mInverter->setSupports3Phases(true);
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

#include <cassert>
#include <QDebug>
#include <QTimer>
#include "froniussolar_api.h"
#include "inverter_updater.h"
#include "inverter.h"
#include "power_info.h"

InverterUpdater::InverterUpdater(Inverter *inverter, QObject *parent):
	QObject(parent),
	mInverter(inverter),
	mSolarApi(new FroniusSolarApi(inverter->hostName(), inverter->port(), this)),
	mInitialized(false),
	mRetryCount(0)
{
	assert(inverter != 0);
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
	// qDebug() << __FUNCTION__;
	mRetryCount = 0;
	mSolarApi->getCommonDataAsync(mInverter->id());
}

void InverterUpdater::onCommonDataFound(const CommonInverterData &data)
{
	// qDebug() << __FUNCTION__;
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
			mInverter->setIsConnected(0);
		}
		scheduleRetrieval();
		break;
	case SolarApiReply::ApiError:
		// Inverter does not support common data retrieval?
		///	@todo EV call setInitialized?
		mInverter->setIsConnected(0);
		mInverter->setSupports3Phases(false);
		break;
	}
}

void InverterUpdater::onThreePhasesDataFound(const ThreePhasesInverterData &data)
{
	// qDebug() << __FUNCTION__ << data.error << data.errorMessage;
	switch (data.error)
	{
	case SolarApiReply::NoError:
	{
		PowerInfo *l1 = mInverter->l1PowerInfo();
		l1->setCurrent(data.iacL1);
		l1->setVoltage(data.uacL1);

		PowerInfo *l2 = mInverter->l2PowerInfo();
		l2->setCurrent(data.iacL2);
		l2->setVoltage(data.uacL2);

		PowerInfo *l3 = mInverter->l3PowerInfo();
		l3->setCurrent(data.iacL3);
		l3->setVoltage(data.uacL3);
		mInverter->setIsConnected(1);
		mInverter->setSupports3Phases(true);
		setInitialized();
	}
		break;
	case SolarApiReply::NetworkError:
		++mRetryCount;
		if (mRetryCount == 5)
		{
			mInverter->setIsConnected(0);
		}
		break;
	case SolarApiReply::ApiError:
		// Inverter does not support 3 phases?
		mInverter->setIsConnected(1);
		setInitialized();
		break;
	}
	scheduleRetrieval();
}

void InverterUpdater::scheduleRetrieval()
{
	static int cnt = 2000;
	QTimer::singleShot(cnt, this, SLOT(onStartRetrieval()));
	// cnt += 1000;
}

void InverterUpdater::setInitialized()
{
	// qDebug() << __FUNCTION__;
	if (!mInitialized)
	{
		emit initialized();
		mInitialized = true;
	}
}

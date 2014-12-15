#include "froniussolar_api_test.h"
#include <QDebug>

FroniusSolarApiTest::FroniusSolarApiTest(QString hostName, int port,
										 QObject *parent) :
	QObject(parent),
	mApi(new FroniusSolarApi(hostName, port, this))
{
	connect(
		mApi, SIGNAL(converterInfoFound(InverterListData)),
		this, SLOT(converterInfoFound(InverterListData)));
	connect(
		mApi, SIGNAL(commonDataFound(CommonInverterData)),
		this, SLOT(commonDataFound(CommonInverterData)));
	connect(
		mApi, SIGNAL(cumulationDataFound(CumulationInverterData)),
		this, SLOT(cumulationDataFound(CumulationInverterData)));
	connect(
		mApi, SIGNAL(threePhasesDataFound(ThreePhasesInverterData)),
		this, SLOT(threePhasesDataFound(ThreePhasesInverterData)));
}

void FroniusSolarApiTest::start()
{
	qDebug() << __FUNCTION__;
	mApi->getConverterInfoAsync();
}

void FroniusSolarApiTest::converterInfoFound(const InverterListData &data)
{
	qDebug() << __FUNCTION__ << data.error << data.errorMessage;
	qDebug() << data.inverters.size();
	for (QList<InverterInfo>::const_iterator it = data.inverters.begin();
		 it != data.inverters.end(); ++it)
	{
		qDebug() << it->id << it->uniqueId << it->customName;
		mApi->getCommonDataAsync(it->id);
		mApi->getCumulationDataAsync();
		mApi->getThreePhasesInverterDataAsync(it->id);
	}
}

void FroniusSolarApiTest::commonDataFound(const CommonInverterData &data)
{
	qDebug() << __FUNCTION__ << data.error << data.errorMessage;
	qDebug() << data.deviceId;
	qDebug() << data.acPower;
}


void FroniusSolarApiTest::cumulationDataFound(const CumulationInverterData &data)
{
	qDebug() << __FUNCTION__ << data.error << data.errorMessage;
	qDebug() << data.acPower;
}

void FroniusSolarApiTest::threePhasesDataFound(const ThreePhasesInverterData &data)
{
	qDebug() << __FUNCTION__ << data.error << data.errorMessage;
	qDebug() << data.deviceId;
	qDebug() << data.acCurrentPhase1;
}

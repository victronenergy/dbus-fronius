#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include "froniussolar_api.h"
#include "fronius_solar_api_test.h"

QProcess *FroniusSolarApiTestFixture::mProcess = 0;

FroniusSolarApiTestFixture::FroniusSolarApiTestFixture(QObject *parent) :
	QObject(parent),
	mApi("localhost", 8080)
{
	connect(&mApi, SIGNAL(converterInfoFound(InverterListData)),
			this, SLOT(onConverterInfoFound(InverterListData)));
	connect(&mApi, SIGNAL(cumulationDataFound(CumulationInverterData)),
			this, SLOT(onCumulationDataFound(CumulationInverterData)));
	connect(&mApi, SIGNAL(commonDataFound(CommonInverterData)),
			this, SLOT(onCommonDataFound(CommonInverterData)));
	connect(&mApi, SIGNAL(commonDataFound(CommonInverterData)),
			this, SLOT(onCommonDataFound(CommonInverterData)));
	connect(&mApi, SIGNAL(threePhasesDataFound(ThreePhasesInverterData)),
			this, SLOT(onThreePhasesDataFound(ThreePhasesInverterData)));
	connect(&mApi, SIGNAL(systemDataFound(CumulationInverterData)),
			this, SLOT(onCumulationDataFound(CumulationInverterData)));
}

void FroniusSolarApiTestFixture::onConverterInfoFound(const InverterListData &data)
{
	mInverterListData.reset(new InverterListData(data));
}

void FroniusSolarApiTestFixture::onCumulationDataFound(const CumulationInverterData &data)
{
	mCumulationData.reset(new CumulationInverterData(data));
}

void FroniusSolarApiTestFixture::onCommonDataFound(const CommonInverterData &data)
{
	mCommonData.reset(new CommonInverterData(data));
}

void FroniusSolarApiTestFixture::onThreePhasesDataFound(const ThreePhasesInverterData &data)
{
	m3PData.reset(new ThreePhasesInverterData(data));
}

void FroniusSolarApiTestFixture::SetUpTestCase()
{
	mProcess = new QProcess();
	// We assume that the location of the current executable is located in
	// a sub directory of the project root. We use this path to reconstruct
	// the path of the fronius simulator script.
	// We also assume that a python interpreter is present (version 2.7+ or
	// 3.0+).
	QFileInfo fi(QCoreApplication::arguments()[0]);
	fi.makeAbsolute();
	fi = QFileInfo(fi.dir().canonicalPath());
	QString p = fi.path() + "/test/src/fronius_sim/app.py";

	QStringList arguments;
	arguments << p;
	mProcess->start("python", arguments);
	if (!mProcess->waitForStarted())
		throw std::logic_error("Could not start python interpreter");
	// Give the python interpreter some time to load the script.
	qWait(400);
}

void FroniusSolarApiTestFixture::TearDownTestCase()
{
	mProcess->kill();
	delete mProcess;
	mProcess = 0;
}

TEST_F(FroniusSolarApiTestFixture, getConverterInfo)
{
	mApi.getConverterInfoAsync();
	waitForCompletion(mInverterListData);

	EXPECT_EQ(mInverterListData->error, SolarApiReply::NoError);
	EXPECT_TRUE(mInverterListData->errorMessage.isEmpty());
	EXPECT_TRUE(mInverterListData->inverters.size() > 0);
	const InverterInfo &ii = mInverterListData->inverters.first();
	EXPECT_EQ(ii.id, QString("1"));
	EXPECT_EQ(ii.uniqueId, QString("1234"));
	EXPECT_EQ(ii.customName, QString("SouthWest"));
	EXPECT_EQ(ii.deviceType, 192);
	/// @todo EV Bad test: even if the reply does not contain an error code,
	/// ii.errorCode will be 0 (default value).
	EXPECT_EQ(ii.errorCode, 0);
	EXPECT_EQ(ii.statusCode, 7);
}

TEST_F(FroniusSolarApiTestFixture, getCumulationData)
{
	mApi.getCumulationDataAsync("2");
	waitForCompletion(mCumulationData);

	EXPECT_EQ(mCumulationData->error, SolarApiReply::NoError);
	EXPECT_TRUE(mCumulationData->errorMessage.isEmpty());
	EXPECT_EQ(mCumulationData->acPower, 3373.0);
	EXPECT_EQ(mCumulationData->dayEnergy, 8000.0);
	EXPECT_EQ(mCumulationData->yearEnergy, 44000.0);
	EXPECT_EQ(mCumulationData->totalEnergy, 45000.0);
}

TEST_F(FroniusSolarApiTestFixture, getCommonData)
{
	mApi.getCommonDataAsync("2");
	waitForCompletion(mCommonData);

	EXPECT_EQ(mCommonData->error, SolarApiReply::NoError);
	EXPECT_TRUE(mCommonData->errorMessage.isEmpty());
	EXPECT_EQ(mCommonData->acFrequency, 50.0);
	EXPECT_EQ(mCommonData->dayEnergy, 8000.0);
	EXPECT_EQ(mCommonData->yearEnergy, 44000.0);
	EXPECT_EQ(mCommonData->totalEnergy, 45000.0);
}

TEST_F(FroniusSolarApiTestFixture, getThreePhasesInverterData)
{
	mApi.getThreePhasesInverterDataAsync("1");
	waitForCompletion(m3PData);

	EXPECT_EQ(m3PData->error, SolarApiReply::NoError);
	EXPECT_TRUE(m3PData->errorMessage.isEmpty());
}

TEST_F(FroniusSolarApiTestFixture, getThreePhasesInverterDataSinglePhase)
{
	mApi.getThreePhasesInverterDataAsync("2");
	waitForCompletion(m3PData);

	EXPECT_EQ(m3PData->error, SolarApiReply::ApiError);
	EXPECT_FALSE(m3PData->errorMessage.isEmpty());
}

TEST_F(FroniusSolarApiTestFixture, getSystemData)
{
	mApi.getSystemDataAsync();
	waitForCompletion(mCumulationData);

	EXPECT_EQ(mCumulationData->error, SolarApiReply::NoError);
	EXPECT_TRUE(mCumulationData->errorMessage.isEmpty());
	EXPECT_EQ(mCumulationData->acPower, 3373.0);
	EXPECT_EQ(mCumulationData->dayEnergy, 8000.0);
	EXPECT_EQ(mCumulationData->yearEnergy, 44000.0);
	EXPECT_EQ(mCumulationData->totalEnergy, 45000.0);
}

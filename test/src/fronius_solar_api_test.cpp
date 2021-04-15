#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include "froniussolar_api.h"
#include "fronius_solar_api_test.h"

QProcess *FroniusSolarApiTest::mProcess = 0;

FroniusSolarApiTest::FroniusSolarApiTest(QObject *parent) :
	QObject(parent),
	mApi("localhost", 8080, 15000)
{
	connect(&mApi, SIGNAL(converterInfoFound(InverterListData)),
			this, SLOT(onConverterInfoFound(InverterListData)));
	connect(&mApi, SIGNAL(commonDataFound(CommonInverterData)),
			this, SLOT(onCommonDataFound(CommonInverterData)));
	connect(&mApi, SIGNAL(commonDataFound(CommonInverterData)),
			this, SLOT(onCommonDataFound(CommonInverterData)));
	connect(&mApi, SIGNAL(threePhasesDataFound(ThreePhasesInverterData)),
			this, SLOT(onThreePhasesDataFound(ThreePhasesInverterData)));
}

void FroniusSolarApiTest::onConverterInfoFound(const InverterListData &data)
{
	mInverterListData.reset(new InverterListData(data));
}

void FroniusSolarApiTest::onCommonDataFound(const CommonInverterData &data)
{
	mCommonData.reset(new CommonInverterData(data));
}

void FroniusSolarApiTest::onThreePhasesDataFound(const ThreePhasesInverterData &data)
{
	m3PData.reset(new ThreePhasesInverterData(data));
}

void FroniusSolarApiTest::SetUpTestCase()
{
	mProcess = new QProcess();
	// We use this PRJ_DIR macro set by the project to reconstruct the path of the fronius
	// simulator script.We also assume that a python interpreter is present (version 2.7+ or 3.0+).
	QStringList arguments;
	arguments << PRJ_DIR "/src/fronius_sim/app.py";
	// If the ccgx sdk is installed, the python interpreter in the sdk will be
	// used when starting python without using its full path. This version does
	// not contain all modules needed by our simulation script, so we assume
	// a full python install is present and installed in /usr/bin
	mProcess->start("/usr/bin/python3", arguments);
	if (!mProcess->waitForStarted())
		throw std::logic_error("Could not start python interpreter");
	// Give the python interpreter some time to load the script.
	qWait(1000);
}

void FroniusSolarApiTest::TearDownTestCase()
{
	mProcess->kill();
	delete mProcess;
	mProcess = 0;
}

TEST_F(FroniusSolarApiTest, getConverterInfo)
{
	mApi.getConverterInfoAsync();
	waitForCompletion(mInverterListData);

	EXPECT_EQ(SolarApiReply::NoError, mInverterListData->error);
	EXPECT_TRUE(mInverterListData->errorMessage.isEmpty());
	ASSERT_TRUE(mInverterListData->inverters.size() > 0);
	const InverterInfo &ii = mInverterListData->inverters.first();
	EXPECT_EQ(1, ii.id);
	EXPECT_EQ("1234", ii.uniqueId);
	EXPECT_EQ(QString("SouthWest"), ii.customName);
	EXPECT_EQ(232, ii.deviceType);
	/// @todo EV Bad test: even if the reply does not contain an error code,
	/// ii.errorCode will be 0 (default value).
	EXPECT_EQ(0, ii.errorCode);
	EXPECT_EQ(7, ii.statusCode);
}

TEST_F(FroniusSolarApiTest, getCommonData)
{
	mApi.getCommonDataAsync(2);
	waitForCompletion(mCommonData);

	EXPECT_EQ(SolarApiReply::NoError, mCommonData->error);
	EXPECT_TRUE(mCommonData->errorMessage.isEmpty());
	EXPECT_EQ(50.0, mCommonData->acFrequency);
	EXPECT_EQ(8000.0, mCommonData->dayEnergy);
	EXPECT_EQ(44000.0, mCommonData->yearEnergy);
	EXPECT_GE(mCommonData->totalEnergy, 0);
}

TEST_F(FroniusSolarApiTest, getThreePhasesInverterData)
{
	mApi.getThreePhasesInverterDataAsync(1);
	waitForCompletion(m3PData);

	EXPECT_EQ(SolarApiReply::NoError, m3PData->error);
	EXPECT_TRUE(m3PData->errorMessage.isEmpty());
}

TEST_F(FroniusSolarApiTest, getThreePhasesInverterDataSinglePhase)
{
	mApi.getThreePhasesInverterDataAsync(2);
	waitForCompletion(m3PData);

	EXPECT_EQ(SolarApiReply::ApiError, m3PData->error);
	EXPECT_FALSE(m3PData->errorMessage.isEmpty());
}

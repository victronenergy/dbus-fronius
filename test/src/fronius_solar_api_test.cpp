#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTest>
#include "froniussolar_api.h"
#include "fronius_solar_api_test.h"

FroniusSolarApiTest::FroniusSolarApiTest(QObject *parent) :
	QObject(parent),
	mProcess(new QProcess(this)),
	mApi("localhost", 8080)
{
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
	QVERIFY(mProcess->waitForStarted());
	// Give the python interpreter some time to load the script.
	QTest::qWait(400);
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

FroniusSolarApiTest::~FroniusSolarApiTest()
{
	mProcess->kill();
}

void FroniusSolarApiTest::getConverterInfo()
{
	mApi.getConverterInfoAsync();

	waitForCompletion(mInverterListData);

	QCOMPARE(mInverterListData->error, SolarApiReply::NoError);
	QVERIFY(mInverterListData->errorMessage.isEmpty());
	QVERIFY(mInverterListData->inverters.size() > 0);
	const InverterInfo &ii = mInverterListData->inverters.first();
	QCOMPARE(ii.id, QString("1"));
	QCOMPARE(ii.uniqueId, QString("1234"));
	QCOMPARE(ii.customName, QString("SouthWest"));
	QCOMPARE(ii.deviceType, 192);
	/// @todo EV Bad test: even if the reply does not contain an error code,
	/// ii.errorCode will be 0 (default value).
	QCOMPARE(ii.errorCode, 0);
	QCOMPARE(ii.statusCode, 7);
}

void FroniusSolarApiTest::getCumulationData()
{
	mApi.getCumulationDataAsync("2");
	waitForCompletion(mCumulationData);

	QCOMPARE(mCumulationData->error, SolarApiReply::NoError);
	QVERIFY(mCumulationData->errorMessage.isEmpty());
	QCOMPARE(mCumulationData->acPower, 3373.0);
	QCOMPARE(mCumulationData->dayEnergy, 8000.0);
	QCOMPARE(mCumulationData->yearEnergy, 44000.0);
	QCOMPARE(mCumulationData->totalEnergy, 45000.0);
}

void FroniusSolarApiTest::getCommonData()
{
	mApi.getCommonDataAsync("2");
	waitForCompletion(mCommonData);

	QCOMPARE(mCommonData->error, SolarApiReply::NoError);
	QVERIFY(mCommonData->errorMessage.isEmpty());
	QCOMPARE(mCommonData->acFrequency, 50.0);
	QCOMPARE(mCommonData->dayEnergy, 8000.0);
	QCOMPARE(mCommonData->yearEnergy, 44000.0);
	QCOMPARE(mCommonData->totalEnergy, 45000.0);
}

void FroniusSolarApiTest::getThreePhasesInverterData()
{
	mApi.getThreePhasesInverterDataAsync("1");
	waitForCompletion(m3PData);

	QCOMPARE(m3PData->error, SolarApiReply::NoError);
	QVERIFY(m3PData->errorMessage.isEmpty());
}

void FroniusSolarApiTest::getThreePhasesInverterDataSinglePhase()
{
	mApi.getThreePhasesInverterDataAsync("2");
	waitForCompletion(m3PData);

	QCOMPARE(m3PData->error, SolarApiReply::ApiError);
	QVERIFY(!m3PData->errorMessage.isEmpty());
}

void FroniusSolarApiTest::getSystemData()
{
	mApi.getSystemDataAsync();
	waitForCompletion(mCumulationData);

	QCOMPARE(mCumulationData->error, SolarApiReply::NoError);
	QVERIFY(mCumulationData->errorMessage.isEmpty());
	QCOMPARE(mCumulationData->acPower, 3373.0);
	QCOMPARE(mCumulationData->dayEnergy, 8000.0);
	QCOMPARE(mCumulationData->yearEnergy, 44000.0);
	QCOMPARE(mCumulationData->totalEnergy, 45000.0);
}

void FroniusSolarApiTest::onConverterInfoFound(const InverterListData &data)
{
	mInverterListData.reset(new InverterListData(data));
}

void FroniusSolarApiTest::onCumulationDataFound(const CumulationInverterData &data)
{
	mCumulationData.reset(new CumulationInverterData(data));
}

void FroniusSolarApiTest::onCommonDataFound(const CommonInverterData &data)
{
	mCommonData.reset(new CommonInverterData(data));
}

void FroniusSolarApiTest::onThreePhasesDataFound(const ThreePhasesInverterData &data)
{
	m3PData.reset(new ThreePhasesInverterData(data));
}

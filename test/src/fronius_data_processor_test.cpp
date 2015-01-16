#include <cmath>
#include "fronius_data_processor.h"
#include "fronius_data_processor_test.h"
#include "froniussolar_api.h"
#include "inverter.h"
#include "inverter_settings.h"
#include "power_info.h"

#define EXPECT_NAN(x) EXPECT_TRUE(std::isnan(x))

TEST_F(FroniusDataProcessorTest, L1PhaseUpdate)
{
	CommonInverterData data;
	data.acPower = 512;
	data.acVoltage = 225;
	data.acCurrent = 2.3;
	data.acFrequency = 60;
	data.totalEnergy = 34596.9; // Fronius gives us energy in Wh we want kWh.
	mProcessor->process(data);

	EXPECT_FLOAT_EQ(512, mInverter->meanPowerInfo()->power());
	EXPECT_FLOAT_EQ(2.3, mInverter->meanPowerInfo()->current());
	EXPECT_FLOAT_EQ(225, mInverter->meanPowerInfo()->voltage());
	EXPECT_FLOAT_EQ(34.5969, mInverter->meanPowerInfo()->totalEnergy());

	EXPECT_FLOAT_EQ(512, mInverter->l1PowerInfo()->power());
	EXPECT_FLOAT_EQ(2.3, mInverter->l1PowerInfo()->current());
	EXPECT_FLOAT_EQ(225, mInverter->l1PowerInfo()->voltage());
	EXPECT_FLOAT_EQ(34.5969, mInverter->l1PowerInfo()->totalEnergy());
}

TEST_F(FroniusDataProcessorTest, L2PhaseUpdate)
{
	mSettings->setPhase(PhaseL2);

	CommonInverterData data;
	data.acPower = 338.2;
	data.acVoltage = 225.8;
	data.acCurrent = 1.4;
	data.acFrequency = 58;
	data.totalEnergy = 12383.9; // Fronius gives us energy in Wh we want kWh.
	mProcessor->process(data);

	EXPECT_FLOAT_EQ(338.2, mInverter->meanPowerInfo()->power());
	EXPECT_FLOAT_EQ(225.8, mInverter->meanPowerInfo()->voltage());
	EXPECT_FLOAT_EQ(1.4, mInverter->meanPowerInfo()->current());
	EXPECT_FLOAT_EQ(12.3839, mInverter->meanPowerInfo()->totalEnergy());

	EXPECT_FLOAT_EQ(338.2, mInverter->l2PowerInfo()->power());
	EXPECT_FLOAT_EQ(225.8, mInverter->l2PowerInfo()->voltage());
	EXPECT_FLOAT_EQ(1.4, mInverter->l2PowerInfo()->current());
	EXPECT_FLOAT_EQ(12.3839, mInverter->l2PowerInfo()->totalEnergy());
}

TEST_F(FroniusDataProcessorTest, L3PhaseUpdate)
{
	mSettings->setPhase(PhaseL3);

	CommonInverterData data;
	data.acPower = 445.7;
	data.acVoltage = 232.8;
	data.acCurrent = 1.93;
	data.acFrequency = 59.5;
	data.totalEnergy = 4321.9; // Fronius gives us energy in Wh we want kWh.
	mProcessor->process(data);

	EXPECT_FLOAT_EQ(445.7, mInverter->meanPowerInfo()->power());
	EXPECT_FLOAT_EQ(1.93, mInverter->meanPowerInfo()->current());
	EXPECT_FLOAT_EQ(232.8, mInverter->meanPowerInfo()->voltage());
	EXPECT_FLOAT_EQ(4.3219, mInverter->meanPowerInfo()->totalEnergy());

	EXPECT_FLOAT_EQ(445.7, mInverter->l3PowerInfo()->power());
	EXPECT_FLOAT_EQ(1.93, mInverter->l3PowerInfo()->current());
	EXPECT_FLOAT_EQ(232.8, mInverter->l3PowerInfo()->voltage());
	EXPECT_FLOAT_EQ(4.3219, mInverter->l3PowerInfo()->totalEnergy());
}

TEST_F(FroniusDataProcessorTest, AllPhaseCommonUpdate)
{
	mSettings->setPhase(ThreePhases);

	CommonInverterData data;
	data.acPower = 445.7;
	data.acVoltage = 232.8;
	data.acCurrent = 1.93;
	data.acFrequency = 59.5;
	data.totalEnergy = 4321.9; // Fronius gives us energy in Wh we want kWh.
	mProcessor->process(data);

	EXPECT_FLOAT_EQ(445.7, mInverter->meanPowerInfo()->power());
	EXPECT_FLOAT_EQ(1.93, mInverter->meanPowerInfo()->current());
	EXPECT_FLOAT_EQ(232.8, mInverter->meanPowerInfo()->voltage());
	EXPECT_FLOAT_EQ(4.3219, mInverter->meanPowerInfo()->totalEnergy());

	EXPECT_NAN(mInverter->l1PowerInfo()->power());
	EXPECT_NAN(mInverter->l1PowerInfo()->current());
	EXPECT_NAN(mInverter->l1PowerInfo()->voltage());
	EXPECT_NAN(mInverter->l1PowerInfo()->totalEnergy());
	EXPECT_NAN(mInverter->l2PowerInfo()->power());
	EXPECT_NAN(mInverter->l3PowerInfo()->power());
}

TEST_F(FroniusDataProcessorTest, ThreePhaseInitial)
{
	mSettings->setPhase(ThreePhases);

	CommonInverterData data;
	data.acPower = 445.7;
	data.acVoltage = 232.8;
	data.acCurrent = 1.93;
	data.acFrequency = 59.5;
	data.totalEnergy = 4321.9; // Fronius gives us energy in Wh we want kWh.
	mProcessor->process(data);

	EXPECT_FLOAT_EQ(445.7, mInverter->meanPowerInfo()->power());
	EXPECT_FLOAT_EQ(1.93, mInverter->meanPowerInfo()->current());
	EXPECT_FLOAT_EQ(232.8, mInverter->meanPowerInfo()->voltage());
	EXPECT_FLOAT_EQ(4.3219, mInverter->meanPowerInfo()->totalEnergy());

	ThreePhasesInverterData tpd;
	tpd.acCurrentPhase1 = 0.61;
	tpd.acVoltagePhase1 = 229.8;
	tpd.acCurrentPhase2 = 0.57;
	tpd.acVoltagePhase2 = 231.2;
	tpd.acCurrentPhase3 = 0.63;
	tpd.acVoltagePhase3 = 227.3;
	mProcessor->process(tpd);

	double vi1 = 0.61 * 229.8;
	double vi2 = 0.57 * 231.2;
	double vi3 = 0.63 * 227.3;
	double vit = vi1 + vi2 + vi3;

	EXPECT_FLOAT_EQ(445.7 * vi1 / vit, mInverter->l1PowerInfo()->power());
	EXPECT_FLOAT_EQ(0.61, mInverter->l1PowerInfo()->current());
	EXPECT_FLOAT_EQ(229.8, mInverter->l1PowerInfo()->voltage());
	EXPECT_NAN(mInverter->l1PowerInfo()->totalEnergy());

	EXPECT_FLOAT_EQ(445.7 * vi2 / vit, mInverter->l2PowerInfo()->power());
	EXPECT_FLOAT_EQ(0.57, mInverter->l2PowerInfo()->current());
	EXPECT_FLOAT_EQ(231.2, mInverter->l2PowerInfo()->voltage());
	EXPECT_NAN(mInverter->l2PowerInfo()->totalEnergy());

	EXPECT_FLOAT_EQ(445.7 * vi3 / vit, mInverter->l3PowerInfo()->power());
	EXPECT_FLOAT_EQ(0.63, mInverter->l3PowerInfo()->current());
	EXPECT_FLOAT_EQ(227.3, mInverter->l3PowerInfo()->voltage());
	EXPECT_NAN(mInverter->l3PowerInfo()->totalEnergy());
}

TEST_F(FroniusDataProcessorTest, ThreePhaseTwice)
{
	mSettings->setPhase(ThreePhases);

	CommonInverterData data;
	data.acPower = 445.7;
	data.acVoltage = 232.8;
	data.acCurrent = 1.93;
	data.acFrequency = 59.5;
	data.totalEnergy = 4321.9;
	mProcessor->process(data);

	ThreePhasesInverterData tpd;
	tpd.acCurrentPhase1 = 0.61;
	tpd.acVoltagePhase1 = 229.8;
	tpd.acCurrentPhase2 = 0.57;
	tpd.acVoltagePhase2 = 231.2;
	tpd.acCurrentPhase3 = 0.63;
	tpd.acVoltagePhase3 = 227.3;
	mProcessor->process(tpd);

	data.totalEnergy = 4331.9;
	mProcessor->process(data);
	mProcessor->process(tpd);

	PowerInfo *pt = mInverter->meanPowerInfo();
	PowerInfo *p1 = mInverter->l1PowerInfo();
	PowerInfo *p2 = mInverter->l2PowerInfo();
	PowerInfo *p3 = mInverter->l3PowerInfo();
	double et = pt->totalEnergy();
	double e1 = p1->totalEnergy();
	double e2 = p2->totalEnergy();
	double e3 = p3->totalEnergy();

	EXPECT_FLOAT_EQ(4.3319, et);
	EXPECT_FLOAT_EQ(et, e1 + e2 + e3);
	EXPECT_FLOAT_EQ(1.00014, e1 / e2);
}

TEST_F(FroniusDataProcessorTest, ThreePhaseMultiple)
{
	mSettings->setPhase(ThreePhases);

	CommonInverterData data;
	data.acPower = 445.7;
	data.acVoltage = 232.8;
	data.acCurrent = 1.93;
	data.acFrequency = 59.5;
	data.totalEnergy = 0;
	mProcessor->process(data);

	ThreePhasesInverterData tpd;
	tpd.acCurrentPhase1 = 0.61;
	tpd.acVoltagePhase1 = 229.8;
	tpd.acCurrentPhase2 = 0.57;
	tpd.acVoltagePhase2 = 231.2;
	tpd.acCurrentPhase3 = 0.63;
	tpd.acVoltagePhase3 = 227.3;
	mProcessor->process(tpd);

	PowerInfo *pt = mInverter->meanPowerInfo();
	PowerInfo *p1 = mInverter->l1PowerInfo();
	PowerInfo *p2 = mInverter->l2PowerInfo();
	PowerInfo *p3 = mInverter->l3PowerInfo();

	for (int i=0; i<10000; ++i) {
		// Voltage * Current -> Power (approx) in Watt
		// We assume samplerate of 0.2 Hz (once every 5 seconds) and
		// convert energy from J to Wh (devision by 3600).
		data.totalEnergy += data.acVoltage * data.acCurrent * 5 / 3600;
		mProcessor->process(data);
		mProcessor->process(tpd);

		double et = pt->totalEnergy();
		double e1 = p1->totalEnergy();
		double e2 = p2->totalEnergy();
		double e3 = p3->totalEnergy();

		EXPECT_FLOAT_EQ(data.totalEnergy / 1000, et);
		if (i > 0) {
			EXPECT_FLOAT_EQ(et, e1 + e2 + e3);
			EXPECT_LT(std::fabs(3 * e1 / et - 1), 0.05);
			EXPECT_LT(std::fabs(3 * e2 / et - 1), 0.05);
			EXPECT_LT(std::fabs(3 * e3 / et - 1), 0.05);
		}
	}
}

void FroniusDataProcessorTest::SetUp()
{
	mInverter.reset(new Inverter("127.0.0.1", 80, "1", "1212b", "cn"));
	mSettings.reset(new InverterSettings(mInverter->uniqueId()));
	mProcessor.reset(new FroniusDataProcessor(mInverter.data(),
											  mSettings.data()));
}

void FroniusDataProcessorTest::TearDown()
{
	mProcessor.reset();
	mSettings.reset();
	mInverter.reset();
}

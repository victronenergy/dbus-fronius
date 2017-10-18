#ifndef FRONIUSDATAPROCESSORTEST_H
#define FRONIUSDATAPROCESSORTEST_H

#include <QScopedPointer>
#include <gtest/gtest.h>
#include "defines.h"

class Inverter;
class InverterSettings;
class FroniusDataProcessor;
class VeQItemProducer;

class FroniusDataProcessorTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	void setUpProcessor(InverterPhase phase);

	QScopedPointer<Inverter> mInverter;
	QScopedPointer<InverterSettings> mSettings;
	QScopedPointer<FroniusDataProcessor> mProcessor;
	QScopedPointer<VeQItemProducer> mItemProducer;
	QScopedPointer<VeQItemProducer> mItemSubscriber;
};

#endif // FRONIUSDATAPROCESSORTEST_H

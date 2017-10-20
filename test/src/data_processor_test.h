#ifndef FRONIUSDATAPROCESSORTEST_H
#define FRONIUSDATAPROCESSORTEST_H

#include <QScopedPointer>
#include <gtest/gtest.h>
#include "defines.h"

class Inverter;
class InverterSettings;
class DataProcessor;
class VeQItemProducer;

class DataProcessorTest : public testing::Test
{
protected:
	virtual void SetUp();

	virtual void TearDown();

	void setUpProcessor(InverterPhase phase);

	QScopedPointer<Inverter> mInverter;
	QScopedPointer<InverterSettings> mSettings;
	QScopedPointer<DataProcessor> mProcessor;
	QScopedPointer<VeQItemProducer> mItemProducer;
	QScopedPointer<VeQItemProducer> mItemSubscriber;
};

#endif // FRONIUSDATAPROCESSORTEST_H

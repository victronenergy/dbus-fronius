#ifndef FRONIUSSOLARAPITEST_H
#define FRONIUSSOLARAPITEST_H

#include <gtest/gtest.h>
#include <QObject>
#include <QScopedPointer>
#include "froniussolar_api.h"
#include "test_helper.h"

class QProcess;

class FroniusSolarApiTestFixture : public QObject, public testing::Test
{
	Q_OBJECT
public:
	explicit FroniusSolarApiTestFixture(QObject *parent = 0);

public slots:
	void onConverterInfoFound(const InverterListData &data);

	void onCumulationDataFound(const CumulationInverterData &data);

	void onCommonDataFound(const CommonInverterData &data);

	void onThreePhasesDataFound(const ThreePhasesInverterData &data);

protected:
	virtual void SetUp() {}

	virtual void TearDown() {}

	/*! Per-test-case set-up.
	 * Called before the first test in this test case.
	 * Can be omitted if not needed.
	 */
	static void SetUpTestCase();

	/*!
	 * Per-test-case tear-down.
	 * Called after the last test in this test case.
	 * Can be omitted if not needed.
	 */
	static void TearDownTestCase();

	/*!
	 * Waits and processes events until `ptr` has been filled.
	 * We use this funtion to wait for the result of webrequests created by
	 * FroniusSolarApi.
	 */
	template<class T>
	void waitForCompletion(QScopedPointer<T> &ptr) {
		ptr.reset();
		while (ptr.isNull()) {
			qWait(10);
		}
	}

	FroniusSolarApi mApi;
	QScopedPointer<InverterListData> mInverterListData;
	QScopedPointer<CumulationInverterData> mCumulationData;
	QScopedPointer<CommonInverterData> mCommonData;
	QScopedPointer<ThreePhasesInverterData> m3PData;

private:
	static QProcess *mProcess;
};

#endif // FRONIUSSOLARAPITEST_H

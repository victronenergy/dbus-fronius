#ifndef FRONIUSSOLARAPITEST_H
#define FRONIUSSOLARAPITEST_H

#include <QObject>
#include <QScopedPointer>
#include <QTest>
#include "froniussolar_api.h"

class QProcess;

class FroniusSolarApiTest : public QObject
{
	Q_OBJECT
public:
	explicit FroniusSolarApiTest(QObject *parent = 0);

	virtual ~FroniusSolarApiTest();

private slots:
	void getConverterInfo();

	void getCumulationData();

	void getCommonData();

	void getThreePhasesInverterData();

	void getThreePhasesInverterDataSinglePhase();

	void getSystemData();

	void onConverterInfoFound(const InverterListData &data);

	void onCumulationDataFound(const CumulationInverterData &data);

	void onCommonDataFound(const CommonInverterData &data);

	void onThreePhasesDataFound(const ThreePhasesInverterData &data);

private:
	/*!
	 * Waits and processes events until `ptr` has been filled.
	 * We use this funtion to wait for the result of webrequests created by
	 * FroniusSolarApi.
	 */
	template<class T>
	void waitForCompletion(QScopedPointer<T> &ptr) {
		ptr.reset();
		while (ptr.isNull()) {
			QTest::qWait(10);
		}
	}

	QProcess *mProcess;
	QScopedPointer<InverterListData> mInverterListData;
	QScopedPointer<CumulationInverterData> mCumulationData;
	QScopedPointer<CommonInverterData> mCommonData;
	QScopedPointer<ThreePhasesInverterData> m3PData;
	FroniusSolarApi mApi;
};

#endif // FRONIUSSOLARAPITEST_H

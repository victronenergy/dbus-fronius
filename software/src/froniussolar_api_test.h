#ifndef FRONIUSSOLAR_API_TEST_H
#define FRONIUSSOLAR_API_TEST_H

#include "froniussolar_api.h"
#include <QObject>
#include <QList>

class FroniusSolarApiTest : public QObject
{
	Q_OBJECT
public:
	FroniusSolarApiTest(QString hostName, int port, QObject *parent);

	void start();

private slots:
	void converterInfoFound(const InverterInfoData &data);

	void commonDataFound(const CommonInverterData &data);

	void cumulationDataFound(const CumulationInverterData &data);

	void threePhasesDataFound(const ThreePhasesInverterData &data);

private:
	FroniusSolarApi *mApi;
};

#endif // FRONIUSSOLAR_API_TEST_H

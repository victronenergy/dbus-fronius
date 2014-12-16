#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QStringList>
#include <QTimer>
#include "froniussolar_api.h"
#include "json/json.h"

QNetworkAccessManager *FroniusSolarApi::mNam;

FroniusSolarApi::FroniusSolarApi(QString hostName, int port, QObject *parent) :
	QObject(parent),
	mHostName(hostName),
	mPort(port),
	mTimeoutTimer(new QTimer(this))
{
	if (mNam == 0)
		mNam = new QNetworkAccessManager();
	mTimeoutTimer->setInterval(5000);
	connect(mTimeoutTimer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
}

QString FroniusSolarApi::hostName() const
{
	return mHostName;
}

int FroniusSolarApi::port() const
{
	return mPort;
}

void FroniusSolarApi::getConverterInfoAsync()
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterInfo.cgi");
	sendGetRequest(url, "getInverterInfo");
}

void FroniusSolarApi::getCumulationDataAsync()
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterRealtimeData.cgi");
	url.addQueryItem("Scope", "System");
	sendGetRequest(url, "getCumulationData");
}

void FroniusSolarApi::getCommonDataAsync(const QString &deviceId)
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterRealtimeData.cgi");
	url.addQueryItem("Scope", "Device");
	url.addQueryItem("DeviceId", deviceId);
	url.addQueryItem("DataCollection", "CommonInverterData");
	sendGetRequest(url, "getCommonData");
}

void FroniusSolarApi::getThreePhasesInverterDataAsync(const QString &deviceId)
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterRealtimeData.cgi");
	url.addQueryItem("Scope", "Device");
	url.addQueryItem("DeviceId", deviceId);
	url.addQueryItem("DataCollection", "3PInverterData");
	sendGetRequest(url, "getThreePhasesInverterData");
}

void FroniusSolarApi::onReply()
{
	QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
	Q_ASSERT(reply != 0);
	mTimeoutTimer->stop();
	mReply = 0;
	SolarApiReply::Error error = SolarApiReply::NoError;
	QString errorMessage;
	if (reply->error() != QNetworkReply::NoError) {
		error = SolarApiReply::NetworkError;
		errorMessage = reply->errorString();
		qDebug() << "Network error:" << errorMessage << reply->url();
	}
	QString id = reply->request().attribute(QNetworkRequest::User).toString();
	QVariantMap result = parseReply(reply);
	int errorCode = getByPath(result, "Head/Status/Code").toInt();
	if (errorCode != 0)
	{
		error = SolarApiReply::ApiError;
		errorMessage = getByPath(result, "Head/Status/Reason").toString();
		qDebug() << "API error:" << errorMessage;
	}
	if (id == "getInverterInfo") {
		InverterListData data;
		data.error = error;
		data.errorMessage = errorMessage;
		if (error == SolarApiReply::NoError) {
			QVariantMap devices = getByPath(result, "Body/Data").toMap();
			for (QVariantMap::Iterator it = devices.begin();
				 it != devices.end();
				 ++it) {
				InverterInfo ii;
				ii.id = it.key();
				QVariantMap di = it.value().toMap();
				ii.uniqueId = di["UniqueID"].toString();
				ii.customName = di["CustomName"].toString();
				ii.errorCode = di["ErrorCode"].toInt();
				ii.statusCode = di["StatusCode"].toInt();
				data.inverters.push_back(ii);
			}
		}
		emit converterInfoFound(data);
	} else if (id == "getCommonData") {
		CommonInverterData data;
		data.error = error;
		data.errorMessage = errorMessage;
		if (error == SolarApiReply::NoError) {
			data.deviceId = getByPath(result, "Head/RequestArguments/DeviceId").
					toString();
			QVariant d = getByPath(result, "Body/Data");
			data.acPower = getByPath(d, "PAC/Value").toInt();
			data.acCurrent = getByPath(d, "IAC/Value").toDouble();
			data.acVoltage = getByPath(d, "UAC/Value").toDouble();
			data.acFrequency = getByPath(d, "FAC/Value").toDouble();
			data.dcCurrent = getByPath(d, "IDC/Value").toDouble();
			data.acVoltage = getByPath(d, "UAC/Value").toDouble();
			data.dayEnergy = getByPath(d, "DAY_ENERGY/Value").toInt();
			data.yearEnergy = getByPath(d, "YEAR_ENERGY/Value").toInt();
			data.totalEnergy = getByPath(d, "TOTAL_ENERGY/Value").toInt();
		}
		emit commonDataFound(data);
	} else if (id == "getThreePhasesInverterData") {
		ThreePhasesInverterData data;
		data.error = error;
		data.errorMessage = errorMessage;
		if (error == SolarApiReply::NoError) {
			data.deviceId = getByPath(result, "Head/RequestArguments/DeviceId").
					toString();
			QVariant d = getByPath(result, "Body/Data");
			data.acCurrentPhase1 = getByPath(d, "IAC_L1/Value").toDouble();
			data.acVoltagePhase1 = getByPath(d, "UAC_L1/Value").toDouble();
			data.acCurrentPhase2 = getByPath(d, "IAC_L2/Value").toDouble();
			data.acVoltagePhase2 = getByPath(d, "UAC_L2/Value").toDouble();
			data.acCurrentPhase3 = getByPath(d, "IAC_L3/Value").toDouble();
			data.acVoltagePhase3 = getByPath(d, "UAC_L3/Value").toDouble();
		}
		emit threePhasesDataFound(data);
	} else if (id == "getCumulationData") {
		CumulationInverterData data;
		data.error = error;
		data.errorMessage = errorMessage;
		if (error == SolarApiReply::NoError) {
			QVariant d = getByPath(result, "Body/Data");
			data.acPower = getByPath(d, "PAC/Value").toInt();
			data.dayEnergy = getByPath(d, "DAY_ENERGY/Value").toInt();
			data.yearEnergy = getByPath(d, "YEAR_ENERGY/Value").toInt();
			data.totalEnergy = getByPath(d, "TOTAL_ENERGY/Value").toInt();
		}
		emit cumulationDataFound(data);
	}
	reply->deleteLater();
}

void FroniusSolarApi::OnTimeout()
{
	qDebug() << __FUNCTION__;
	if (mReply != 0) {
		mReply->abort();
		mReply = 0;
	}
}

void FroniusSolarApi::sendGetRequest(QUrl url, QString id)
{
	url.setScheme("http");
	url.setPort(mPort);
	url.setHost(mHostName);
	QNetworkRequest r(url);
	// Add the ID to the request so we can emit the correct signal when the
	// request has been handled.
	r.setAttribute(QNetworkRequest::User, id);
	mReply = mNam->get(r);
	QObject::connect(mReply, SIGNAL(finished()), this, SLOT(onReply()));
	mTimeoutTimer->start();
}

QVariantMap FroniusSolarApi::parseReply(QNetworkReply *reply)
{
	Q_ASSERT(reply != 0);
	QByteArray bytes = reply->readAll();
	QString result(QString::fromLocal8Bit(bytes));
	return JSON::instance().parse(result).toMap();
}

QVariant FroniusSolarApi::getByPath(const QVariant &variant,
									const QString &path)
{
	QVariant m = variant;
	QStringList spl = path.split('/');
	for (QStringList::Iterator it = spl.begin(); it != spl.end(); ++it)
	{
		m = m.toMap()[*it];
	}
	return m;
}

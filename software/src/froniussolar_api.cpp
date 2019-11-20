#include <QtGlobal>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QsLog.h>
#include <QStringList>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QUrlQuery>
#endif

#include "froniussolar_api.h"
#include "json/json.h"

FroniusSolarApi::FroniusSolarApi(const QString &hostName, int port,
								 QObject *parent) :
	QObject(parent),
	mReply(0),
	mHostName(hostName),
	mPort(port),
	mTimeoutTimer(new QTimer(this))
{
	mTimeoutTimer->setInterval(5000);
	connect(mTimeoutTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

QString FroniusSolarApi::hostName() const
{
	return mHostName;
}

void FroniusSolarApi::setHostName(const QString &h)
{
	if (mHostName == h)
		return;
	mHostName = h;
}

int FroniusSolarApi::port() const
{
	return mPort;
}

void FroniusSolarApi::setPort(int port)
{
	if (mPort == port)
		return;
	mPort = port;
}

const QUrl FroniusSolarApi::baseUrl(const QString &path)
{
	QUrl url;
	url.setHost(mHostName);
	url.setPort(mPort);
	url.setScheme("http");
	url.setPath(path);
	return url;
}

void FroniusSolarApi::getConverterInfoAsync()
{
	QUrl url = baseUrl("/solar_api/v1/GetInverterInfo.cgi");
	sendGetRequest(url, "getInverterInfo");
}

void FroniusSolarApi::getCumulationDataAsync(int deviceId)
{
	QUrl url = baseUrl("/solar_api/v1/GetInverterRealtimeData.cgi");

	#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	QUrlQuery query;
	query.addQueryItem("Scope", "Device");
	query.addQueryItem("DeviceId", QString::number(deviceId));
	query.addQueryItem("DataCollection", "CumulationInverterData");
	url.setQuery(query);
	#else
	url.addQueryItem("Scope", "Device");
	url.addQueryItem("DeviceId", QString::number(deviceId));
	url.addQueryItem("DataCollection", "CumulationInverterData");
	#endif
	sendGetRequest(url, "getCumulationData");
}

void FroniusSolarApi::getCommonDataAsync(int deviceId)
{
	QUrl url = baseUrl("/solar_api/v1/GetInverterRealtimeData.cgi");

	#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	QUrlQuery query;
	query.addQueryItem("Scope", "Device");
	query.addQueryItem("DeviceId", QString::number(deviceId));
	query.addQueryItem("DataCollection", "CommonInverterData");
	url.setQuery(query);
	#else
	url.addQueryItem("Scope", "Device");
	url.addQueryItem("DeviceId", QString::number(deviceId));
	url.addQueryItem("DataCollection", "CommonInverterData");
	#endif
	sendGetRequest(url, "getCommonData");
}

void FroniusSolarApi::getThreePhasesInverterDataAsync(int deviceId)
{
	QUrl url = baseUrl("/solar_api/v1/GetInverterRealtimeData.cgi");

	#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	QUrlQuery query;
	query.addQueryItem("Scope", "Device");
	query.addQueryItem("DeviceId", QString::number(deviceId));
	query.addQueryItem("DataCollection", "3PInverterData");
	url.setQuery(query);
	#else
	url.addQueryItem("Scope", "Device");
	url.addQueryItem("DeviceId", QString::number(deviceId));
	url.addQueryItem("DataCollection", "3PInverterData");
	#endif
	sendGetRequest(url, "getThreePhasesInverterData");
}

void FroniusSolarApi::getSystemDataAsync()
{
	QUrl url = baseUrl("/solar_api/v1/GetInverterRealtimeData.cgi");

	#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	QUrlQuery query;
	query.addQueryItem("Scope", "System");
	url.setQuery(query);
	#else
	url.addQueryItem("Scope", "System");
	#endif
	sendGetRequest(url, "getSystemData");
}

void FroniusSolarApi::onDone()
{
	mTimeoutTimer->stop();
	processRequest(mReply);
	mReply->deleteLater();
}

void FroniusSolarApi::onTimeout()
{
	// For some reason, this signal can fire even with an inactive timer.
	if (mTimeoutTimer->isActive())
		mReply->abort();
}

void FroniusSolarApi::processConverterInfo(QNetworkReply *reply)
{
	InverterListData data;
	QVariantMap map;
	processReply(reply, data, map);
	QVariantMap devices = getByPath(map, "Body/Data").toMap();
	for (QVariantMap::Iterator it = devices.begin();
		 it != devices.end();
		 ++it) {
		InverterInfo ii;
		ii.id = it.key().toInt();
		QVariantMap di = it.value().toMap();
		ii.deviceType = di["DT"].toInt();
		ii.uniqueId = di["UniqueID"].toString();
		ii.customName = di["CustomName"].toString();
		ii.errorCode = di["ErrorCode"].toInt();
		ii.statusCode = di["StatusCode"].toInt();
		data.inverters.push_back(ii);
	}
	emit converterInfoFound(data);
}

void FroniusSolarApi::processCumulationData(QNetworkReply *reply)
{
	CumulationInverterData data;
	QVariantMap map;
	processReply(reply, data, map);
	QVariant d = getByPath(map, "Body/Data");
	data.acPower = getByPath(d, "PAC/Value").toDouble();
	data.dayEnergy = getByPath(d, "DAY_ENERGY/Value").toDouble();
	data.yearEnergy = getByPath(d, "YEAR_ENERGY/Value").toDouble();
	data.totalEnergy = getByPath(d, "TOTAL_ENERGY/Value").toDouble();
	emit cumulationDataFound(data);
}

void FroniusSolarApi::processCommonData(QNetworkReply *reply)
{
	CommonInverterData data;
	QVariantMap map;
	processReply(reply, data, map);
	data.deviceId = getByPath(map, "Head/RequestArguments/DeviceId").toString();
	QVariant d = getByPath(map, "Body/Data");
	data.acPower = getByPath(d, "PAC/Value").toDouble();
	data.acCurrent = getByPath(d, "IAC/Value").toDouble();
	data.acVoltage = getByPath(d, "UAC/Value").toDouble();
	data.acFrequency = getByPath(d, "FAC/Value").toDouble();
	data.dcCurrent = getByPath(d, "IDC/Value").toDouble();
	data.dcVoltage = getByPath(d, "UDC/Value").toDouble();
	data.dayEnergy = getByPath(d, "DAY_ENERGY/Value").toDouble();
	data.yearEnergy = getByPath(d, "YEAR_ENERGY/Value").toDouble();
	data.totalEnergy = getByPath(d, "TOTAL_ENERGY/Value").toDouble();
	data.statusCode = getByPath(d, "DeviceStatus/StatusCode").toInt();
	data.errorCode = getByPath(d, "DeviceStatus/ErrorCode").toInt();
	emit commonDataFound(data);
}

void FroniusSolarApi::processThreePhasesData(QNetworkReply *reply)
{
	ThreePhasesInverterData data;
	QVariantMap map;
	processReply(reply, data, map);
	data.deviceId = getByPath(map, "Head/RequestArguments/DeviceId").toString();
	QVariant d = getByPath(map, "Body/Data");
	data.acCurrentPhase1 = getByPath(d, "IAC_L1/Value").toDouble();
	data.acVoltagePhase1 = getByPath(d, "UAC_L1/Value").toDouble();
	data.acCurrentPhase2 = getByPath(d, "IAC_L2/Value").toDouble();
	data.acVoltagePhase2 = getByPath(d, "UAC_L2/Value").toDouble();
	data.acCurrentPhase3 = getByPath(d, "IAC_L3/Value").toDouble();
	data.acVoltagePhase3 = getByPath(d, "UAC_L3/Value").toDouble();
	emit threePhasesDataFound(data);
}

void FroniusSolarApi::sendGetRequest(const QUrl &request, const QString &id)
{
	Q_ASSERT(mRequestType.isEmpty());
	mReply = networkManager().get(QNetworkRequest(request));
	connect(mReply, SIGNAL(finished()), this, SLOT(onDone()));
	mRequestType = id;
	mTimeoutTimer->start();
}

void FroniusSolarApi::processRequest(QNetworkReply *reply)
{
	if (mRequestType == "getInverterInfo") {
		processConverterInfo(reply);
	} else if (mRequestType == "getCommonData") {
		processCommonData(reply);
	} else if (mRequestType == "getThreePhasesInverterData") {
		processThreePhasesData(reply);
	} else if (mRequestType == "getCumulationData") {
		processCumulationData(reply);
	} else if (mRequestType == "getSystemData") {
		processCumulationData(reply);
	}
}

void FroniusSolarApi::processReply(QNetworkReply *reply,
								   SolarApiReply &apiReply,
								   QVariantMap &map)
{
	mRequestType.clear();
	// Some error will be logged with QLOG_DEBUG because they occur often during
	// a device scan and would fill the log with a lot of useless information.
	if (reply->error() != QNetworkReply::NoError) {
		apiReply.error = SolarApiReply::NetworkError;
		apiReply.errorMessage = reply->errorString();;
		QLOG_DEBUG() << "Network error:" << apiReply.errorMessage << mHostName;
		return;
	}
	QByteArray bytes = reply->readAll();
	QString result(QString::fromLocal8Bit(bytes));
	reply->close();
	QLOG_TRACE() << result;
	if (!result.isEmpty()) {
		map = JSON::instance().parse(result).toMap();
	}
	QVariantMap status = getByPath(map, "Head/Status").toMap();
	if (!status.contains("Code")) {
		apiReply.error = SolarApiReply::NetworkError;
		apiReply.errorMessage = "Reply message has no status "
								"(we're probably talking to a device "
								"that does not support the Fronius Solar API)";
		QLOG_DEBUG() << "Network error:" << apiReply.errorMessage << mHostName;
		return;
	}
	int errorCode = status["Code"].toInt();
	if (errorCode != 0)
	{
		apiReply.error = SolarApiReply::ApiError;
		apiReply.errorMessage = status["Reason"].toString();
		QLOG_DEBUG() << "Fronius solar API error:" << apiReply.errorMessage;
		return;
	}
	apiReply.error = SolarApiReply::NoError;
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

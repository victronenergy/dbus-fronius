#include <qhttp.h>
#include <QJsonDocument>
#include <QsLog.h>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include "froniussolar_api.h"

FroniusSolarApi::FroniusSolarApi(const QString &hostName, int port,
								 QObject *parent) :
	QObject(parent),
	mHttp(0),
	mHostName(hostName),
	mPort(port),
	mTimeoutTimer(new QTimer(this))
{
	mTimeoutTimer->setInterval(5000);
	connect(mTimeoutTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	updateHttpClient();
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
	updateHttpClient();
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
	updateHttpClient();
}

void FroniusSolarApi::getConverterInfoAsync()
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterInfo.cgi");
	sendGetRequest(url, "getInverterInfo");
}

void FroniusSolarApi::getCumulationDataAsync(int deviceId)
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterRealtimeData.cgi");
	QUrlQuery query;
	query.addQueryItem("Scope", "Device");
	query.addQueryItem("DeviceId", QString::number(deviceId));
	query.addQueryItem("DataCollection", "CumulationInverterData");
	url.setQuery(query);
	sendGetRequest(url, "getCumulationData");
}

void FroniusSolarApi::getCommonDataAsync(int deviceId)
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterRealtimeData.cgi");
	QUrlQuery query;
	query.addQueryItem("Scope", "Device");
	query.addQueryItem("DeviceId", QString::number(deviceId));
	query.addQueryItem("DataCollection", "CommonInverterData");
	url.setQuery(query);
	sendGetRequest(url, "getCommonData");
}

void FroniusSolarApi::getThreePhasesInverterDataAsync(int deviceId)
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterRealtimeData.cgi");
	QUrlQuery query;
	query.addQueryItem("Scope", "Device");
	query.addQueryItem("DeviceId", QString::number(deviceId));
	query.addQueryItem("DataCollection", "3PInverterData");
	url.setQuery(query);
	sendGetRequest(url, "getThreePhasesInverterData");
}

void FroniusSolarApi::getSystemDataAsync()
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterRealtimeData.cgi");
	QUrlQuery query;
	query.addQueryItem("Scope", "System");
	url.setQuery(query);
	sendGetRequest(url, "getSystemData");
}

void FroniusSolarApi::onDone(bool error)
{
	processRequest(error ? mHttp->errorString() : QString());
}

void FroniusSolarApi::onTimeout()
{
	// During call to abort onRequestFinished will be called.
	mHttp->abort();
}

void FroniusSolarApi::processConverterInfo(const QString &networkError)
{
	InverterListData data;
	QJsonObject map;
	processReply(networkError, data, map);
	QJsonObject devices = getChild(map, "Body/Data");
	for (QJsonObject::Iterator it = devices.begin();
		 it != devices.end();
		 ++it) {
		InverterInfo ii;
		ii.id = it.key().toInt();
		QJsonObject di = it.value().toObject();
		ii.deviceType = di["DT"].toInt();
		ii.uniqueId = di["UniqueID"].toString();
		ii.customName = di["CustomName"].toString();
		ii.errorCode = di["ErrorCode"].toInt();
		ii.statusCode = di["StatusCode"].toInt();
		data.inverters.push_back(ii);
	}
	emit converterInfoFound(data);
}

void FroniusSolarApi::processCumulationData(const QString &networkError)
{
	CumulationInverterData data;
	QJsonObject map;
	processReply(networkError, data, map);
	QJsonObject d = getChild(map, "Body/Data");
	data.acPower = getByPath(d, "PAC/Value").toDouble();
	data.dayEnergy = getByPath(d, "DAY_ENERGY/Value").toDouble();
	data.yearEnergy = getByPath(d, "YEAR_ENERGY/Value").toDouble();
	data.totalEnergy = getByPath(d, "TOTAL_ENERGY/Value").toDouble();
	emit cumulationDataFound(data);
}

void FroniusSolarApi::processCommonData(const QString &networkError)
{
	CommonInverterData data;
	QJsonObject map;
	processReply(networkError, data, map);
	data.deviceId = getByPath(map, "Head/RequestArguments/DeviceId").toString();
	QJsonObject d = getChild(map, "Body/Data");
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

void FroniusSolarApi::processThreePhasesData(const QString &networkError)
{
	ThreePhasesInverterData data;
	QJsonObject map;
	processReply(networkError, data, map);
	data.deviceId = getByPath(map, "Head/RequestArguments/DeviceId").toString();
	QJsonObject d = getChild(map, "Body/Data");
	data.acCurrentPhase1 = getByPath(d, "IAC_L1/Value").toDouble();
	data.acVoltagePhase1 = getByPath(d, "UAC_L1/Value").toDouble();
	data.acCurrentPhase2 = getByPath(d, "IAC_L2/Value").toDouble();
	data.acVoltagePhase2 = getByPath(d, "UAC_L2/Value").toDouble();
	data.acCurrentPhase3 = getByPath(d, "IAC_L3/Value").toDouble();
	data.acVoltagePhase3 = getByPath(d, "UAC_L3/Value").toDouble();
	emit threePhasesDataFound(data);
}

void FroniusSolarApi::processSystemData(const QString &networkError)
{
	CumulationInverterData data;
	QJsonObject map;
	processReply(networkError, data, map);
	QJsonObject d = getChild(map, "Body/Data");
	data.acPower = getByPath(d, "PAC/Value").toDouble();
	data.dayEnergy = getByPath(d, "DAY_ENERGY/Value").toDouble();
	data.yearEnergy = getByPath(d, "YEAR_ENERGY/Value").toDouble();
	data.totalEnergy = getByPath(d, "TOTAL_ENERGY/Value").toDouble();
	emit systemDataFound(data);
}

void FroniusSolarApi::sendGetRequest(const QUrl &request, const QString &id)
{
	Q_ASSERT(mRequestType.isEmpty());
	mHttp->get(request.toString());
	mRequestType = id;
	mTimeoutTimer->start();
}

void FroniusSolarApi::processRequest(const QString &networkError)
{
	if (mRequestType == "getInverterInfo") {
		processConverterInfo(networkError);
	} else if (mRequestType == "getCommonData") {
		processCommonData(networkError);
	} else if (mRequestType == "getThreePhasesInverterData") {
		processThreePhasesData(networkError);
	} else if (mRequestType == "getCumulationData") {
		processCumulationData(networkError);
	} else if (mRequestType == "getSystemData") {
		processCumulationData(networkError);
	}
}

void FroniusSolarApi::processReply(const QString &networkError,
								   SolarApiReply &apiReply,
								   QJsonObject &map)
{
	mRequestType.clear();
	mTimeoutTimer->stop();
	// Some error will be logged with QLOG_DEBUG because they occur often during
	// a device scan and would fill the log with a lot of useless information.
	if (!networkError.isEmpty()) {
		apiReply.error = SolarApiReply::NetworkError;
		apiReply.errorMessage = mHttp->errorString();
		QLOG_DEBUG() << "Network error:" << apiReply.errorMessage << mHostName;
		return;
	}
	QByteArray bytes = mHttp->readAll();
	if (!bytes.isEmpty()) {
		QLOG_TRACE() << QString::fromLocal8Bit(bytes);
		QJsonDocument doc = QJsonDocument::fromJson(bytes);
		map = doc.object();
	}
	QJsonObject status = getChild(map, "Head/Status");
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

void FroniusSolarApi::updateHttpClient()
{
	delete mHttp;
	mHttp = new QHttp(mHostName, QHttp::ConnectionModeHttp, mPort, this);
	connect(mHttp, SIGNAL(done(bool)),
			this, SLOT(onDone(bool)));
}

QJsonValue FroniusSolarApi::getByPath(const QJsonObject &obj, const QString &path)
{
	int i = path.lastIndexOf('/');
	if (i == -1)
		return obj[path];
	QJsonObject o = getChild(obj, path.left(i));
	return o[path.mid(i + 1)];
}

QJsonObject FroniusSolarApi::getChild(const QJsonObject &obj, const QString &path)
{
	QJsonObject o = obj;
	QStringList spl = path.split('/');
	for (QStringList::Iterator it = spl.begin(); it != spl.end(); ++it)
		o = o[*it].toObject();
	return o;
}

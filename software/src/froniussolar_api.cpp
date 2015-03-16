#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QsLog.h>
#include <QStringList>
#include <QTimer>
#include "froniussolar_api.h"
#include "json/json.h"

QWeakPointer<QNetworkAccessManager> FroniusSolarApi::mStaticNam;

FroniusSolarApi::FroniusSolarApi(QString hostName, int port, QObject *parent) :
	QObject(parent),
	mNam(mStaticNam.toStrongRef()),
	mHostName(hostName),
	mPort(port),
	mReply(0),
	mTimeoutTimer(new QTimer(this))
{
	if (mNam == 0) {
		mNam = QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager());
		mStaticNam = mNam.toWeakRef();
	}
	mTimeoutTimer->setInterval(5000);
	connect(mTimeoutTimer, SIGNAL(timeout()), this, SLOT(OnTimeout()));
}

FroniusSolarApi::~FroniusSolarApi()
{
	// If we do not have this function, the default destructor will call the
	// destructor of mNam while QNetworkManager is undefined.
}

QString FroniusSolarApi::hostName() const
{
	return mHostName;
}

void FroniusSolarApi::setHostName(const QString &h)
{
	mHostName = h;
}

int FroniusSolarApi::port() const
{
	return mPort;
}

void FroniusSolarApi::setPort(int port)
{
	mPort = port;
}

void FroniusSolarApi::getConverterInfoAsync()
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterInfo.cgi");
	sendGetRequest(url, "getInverterInfo");
}

void FroniusSolarApi::getCumulationDataAsync(const QString &deviceId)
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterRealtimeData.cgi");
	url.addQueryItem("Scope", "Device");
	url.addQueryItem("DeviceId", deviceId);
	url.addQueryItem("DataCollection", "CumulationInverterData");
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

void FroniusSolarApi::getSystemDataAsync()
{
	QUrl url;
	url.setPath("/solar_api/v1/GetInverterRealtimeData.cgi");
	url.addQueryItem("Scope", "System");
	sendGetRequest(url, "getSystemData");
}

void FroniusSolarApi::onReply()
{
	mTimeoutTimer->stop();
	mReply = 0;
	QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
	Q_ASSERT(reply != 0);
	QString id = reply->request().attribute(QNetworkRequest::User).toString();
	if (id == "getInverterInfo") {
		processConverterInfo(reply);
	} else if (id == "getCommonData") {
		processCommonData(reply);
	} else if (id == "getThreePhasesInverterData") {
		processThreePhasesData(reply);
	} else if (id == "getCumulationData") {
		processCumulationData(reply);
	} else if (id == "getSystemData") {
		processCumulationData(reply);
	}
	reply->deleteLater();
}

void FroniusSolarApi::OnTimeout()
{
	if (mReply != 0) {
		mReply->abort();
		mReply = 0;
	}
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
		ii.id = it.key();
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
	data.deviceId = getByPath(map, "Head/RequestArguments/DeviceId").
					toString();
	QVariant d = getByPath(map, "Body/Data");
	data.acCurrentPhase1 = getByPath(d, "IAC_L1/Value").toDouble();
	data.acVoltagePhase1 = getByPath(d, "UAC_L1/Value").toDouble();
	data.acCurrentPhase2 = getByPath(d, "IAC_L2/Value").toDouble();
	data.acVoltagePhase2 = getByPath(d, "UAC_L2/Value").toDouble();
	data.acCurrentPhase3 = getByPath(d, "IAC_L3/Value").toDouble();
	data.acVoltagePhase3 = getByPath(d, "UAC_L3/Value").toDouble();
	emit threePhasesDataFound(data);
}

void FroniusSolarApi::processSystemData(QNetworkReply *reply)
{
	CumulationInverterData data;
	QVariantMap map;
	processReply(reply, data, map);
	QVariant d = getByPath(map, "Body/Data");
	data.acPower = getByPath(d, "PAC/Value").toDouble();
	data.dayEnergy = getByPath(d, "DAY_ENERGY/Value").toDouble();
	data.yearEnergy = getByPath(d, "YEAR_ENERGY/Value").toDouble();
	data.totalEnergy = getByPath(d, "TOTAL_ENERGY/Value").toDouble();
	emit systemDataFound(data);
}

void FroniusSolarApi::sendGetRequest(QUrl url, const QString &id)
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

void FroniusSolarApi::processReply(QNetworkReply *reply, SolarApiReply &apiReply,
								 QVariantMap &map)
{
	// Some error will be logged with QLOG_DEBUG because they occur often during
	// a device scan and would fill the log with a lot of useless information.
	if (reply->error() != QNetworkReply::NoError) {
		apiReply.error = SolarApiReply::NetworkError;
		apiReply.errorMessage = reply->errorString();
		QLOG_DEBUG() << "Network error:" << apiReply.errorMessage << reply->url();
		return;
	}
	QByteArray bytes = reply->readAll();
	QString result(QString::fromLocal8Bit(bytes));
	if (result.size() > 0) {
		QLOG_TRACE() << result;
		map = JSON::instance().parse(result).toMap();
	}
	QVariantMap status = getByPath(map, "Head/Status").toMap();
	if (!status.contains("Code")) {
		apiReply.error = SolarApiReply::NetworkError;
		apiReply.errorMessage = "Reply message has no status "
								"(we're probably talking to a device "
								"that does not support the Fronius Solar API)";
		QLOG_DEBUG() << "Network error:" << apiReply.errorMessage << reply->url();
		return;
	}
	int errorCode = status["Code"].toInt();
	if (errorCode != 0)
	{
		apiReply.error = SolarApiReply::ApiError;
		apiReply.errorMessage = status["Reason"].toString();
		QLOG_ERROR() << "Fronius solar API error:" << apiReply.errorMessage;
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

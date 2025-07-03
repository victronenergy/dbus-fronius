#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QJsonDocument>
#include <QUrlQuery>
#include "qhttp/qhttp.h"
#else
#include <QHttp>
#include "json/json.h"
#endif

#include <QUrl>
#include <QStringList>
#include <QTimer>
#include "logging.h"

#include "froniussolar_api.h"

FroniusSolarApi::FroniusSolarApi(const QString &hostName, int port, int timeout,
								 QObject *parent) :
	QObject(parent),
	mHttp(0),
	mHostName(hostName),
	mPort(port),
	mTimeoutTimer(new QTimer(this))
{
	mTimeoutTimer->setInterval(timeout);
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

const QUrl FroniusSolarApi::baseUrl(const QString &path)
{
	QUrl url;
	url.setPath(path);
	return url;
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
	QUrl url = baseUrl("/solar_api/v1/GetInverterInfo.cgi");
	sendGetRequest(url, "getInverterInfo");
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

void FroniusSolarApi::getDeviceInfoAsync()
{
	QUrl url = baseUrl("/solar_api/v1/GetActiveDeviceInfo.cgi");
	#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
	QUrlQuery query;
	query.addQueryItem("DeviceClass", "Inverter");
	url.setQuery(query);
	#else
	url.addQueryItem("DeviceClass", "Inverter");
	#endif
	sendGetRequest(url, "getDeviceInfo");
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
	QVariantMap map;
	QMap<int, int> unprogrammed;

	processReply(networkError, data, map);
	QVariantMap devices = getByPath(map, "Body/Data").toMap();
	for (QVariantMap::Iterator it = devices.begin();
		 it != devices.end();
		 ++it) {
		InverterInfo ii;
		ii.id = it.key().toInt();
		QVariantMap di = it.value().toMap();
		ii.deviceType = di["DT"].toInt();
		ii.uniqueId = di["UniqueID"].toString();

		// Workaround for Fronius inverters that have unprogrammed UniqueIDs,
		// which show up as 0xFFFFFF. If the uniqueId is 16777215, and this
		// is not the first one, append the key to the uniqueId.
		// QMap objects always iterate in key order, so this will always
		// be parsed in the same order. In the unlikely event that there are 10
		// inverters on a DataManager, then 10 may sort before 2, but we don't
		// care as long as it is consistent each time. Settings for the
		// inverter will be stored under this key, so consistency is important.
		// The uniqueId is later combined with the deviceType, so this only
		// needs to be unique per DT.
		if (ii.uniqueId == "16777215") {
			if (unprogrammed.value(ii.deviceType) > 0)
				ii.uniqueId.append("_").append(it.key());
			unprogrammed[ii.deviceType]++;
		}

		data.inverters.push_back(ii);
	}
	emit converterInfoFound(data);
}

void FroniusSolarApi::processCommonData(const QString &networkError)
{
	CommonInverterData data;
	QVariantMap map;
	processReply(networkError, data, map);
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

void FroniusSolarApi::processThreePhasesData(const QString &networkError)
{
	ThreePhasesInverterData data;
	QVariantMap map;
	processReply(networkError, data, map);
	data.deviceId = getByPath(map, "Head/RequestArguments/DeviceId").toString();
	QVariant d = getByPath(map, "Body/Data");
	data.acCurrentPhase1 = getByPath(d, "IAC_L1/Value").toDouble();
	data.acVoltagePhase1 = getByPath(d, "UAC_L1/Value").toDouble(&data.valid);
	data.acCurrentPhase2 = getByPath(d, "IAC_L2/Value").toDouble();
	data.acVoltagePhase2 = getByPath(d, "UAC_L2/Value").toDouble();
	data.acCurrentPhase3 = getByPath(d, "IAC_L3/Value").toDouble();
	data.acVoltagePhase3 = getByPath(d, "UAC_L3/Value").toDouble();
	emit threePhasesDataFound(data);
}

void FroniusSolarApi::processDeviceInfo(const QString &networkError)
{
	QVariantMap map;
	DeviceInfoData data;
	processReply(networkError, data, map);
	QVariantMap devices = getByPath(map, "Body/Data").toMap();
	for (QVariantMap::Iterator it = devices.begin(); it != devices.end(); ++it) {
		QVariantMap di = it.value().toMap();
		data.serialInfo[it.key().toInt()] = di["Serial"].toString();
	}
	emit deviceInfoFound(data);
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
	} else if (mRequestType == "getDeviceInfo") {
		processDeviceInfo(networkError);
	}
}

void FroniusSolarApi::processReply(const QString &networkError,
								   SolarApiReply &apiReply,
								   QVariantMap &map)
{
	mRequestType.clear();
	mTimeoutTimer->stop();
	// Some error will be logged with qDebug because they occur often during
	// a device scan and would fill the log with a lot of useless information.
	if (!networkError.isEmpty()) {
		apiReply.error = SolarApiReply::NetworkError;
		apiReply.errorMessage = mHttp->errorString();
		qDebug() << "Network error:" << apiReply.errorMessage << mHostName;
		return;
	}
	QByteArray bytes = mHttp->readAll();
	// CCGX does not receive reply from subsequent requests if we don't do this.
	mHttp->close();
	qDebug() << QString::fromLocal8Bit(bytes);
	map = parseJson(bytes);

	QVariantMap status = getByPath(map, "Head/Status").toMap();
	if (!status.contains("Code")) {
		apiReply.error = SolarApiReply::NetworkError;
		apiReply.errorMessage = "Reply message has no status "
								"(we're probably talking to a device "
								"that does not support the Fronius Solar API)";
		qDebug() << "Network error:" << apiReply.errorMessage << mHostName;
		return;
	}
	int errorCode = status["Code"].toInt();
	if (errorCode != 0)
	{
		apiReply.error = SolarApiReply::ApiError;
		apiReply.errorMessage = status["Reason"].toString();
		qDebug() << "Fronius solar API error:" << apiReply.errorMessage;
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

QVariantMap FroniusSolarApi::parseJson(const QByteArray bytes)
{
		#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
		// QT5/6 version
		QJsonDocument doc = QJsonDocument::fromJson(bytes);
		if (doc.isNull())
			return QVariantMap();

		return doc.toVariant().toMap();
		#else
		// QT4 version
		QString result(QString::fromLocal8Bit(bytes));
		if (!result.isEmpty())
		{
			return JSON::instance().parse(result).toMap();
		} else {
			return QVariantMap();
		}
		#endif
}

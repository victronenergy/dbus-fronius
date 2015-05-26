#ifndef FRONIUSSOLAR_API_H
#define FRONIUSSOLAR_API_H

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QList>
#include <QString>
#include <QUrl>

class QHttp;
class QTimer;

/*!
 * @brief Base class for all data packages returned by FroniusSolarApi.
 */
struct SolarApiReply
{
	enum Error
	{
		NoError = 0,
		NetworkError = 1,
		ApiError = 2
	};

	Error error;
	QString errorMessage;
};

struct InverterInfo
{
	/*!
	 * @brief The id of the inverter.
	 * This id is unique only within the DATCOM chain is used to address
	 * inverters within this chain. Use this ID in the FroniusSolarApi where
	 * a deviceId is required.
	 */
	int id;
	/*!
	 * @brief uniqueId Unique ID of the inverter (serial number?)
	 */
	QString uniqueId;
	/*!
	 * @brief customName Name of the inverted as assigned by the user.
	 * May be empty.
	 */
	QString customName;
	/*!
	 * @brief deviceType Type of the device. This value is not documented,
	 * but inverters seem to have value 192. Also seen 123 on an 3 phase
	 * inverter.
	 */
	int deviceType;
	/*!
	 * @brief errorCode
	 * -1	No _valid_ error code
	 *  0	OK
	 */
	int errorCode;
	/*!
	 * @brief statusCode
	 * 0-6	Startup
	 * 7	Running
	 * 8	Standby
	 * 9	Boot loading
	 * 10	Error
	 */
	int statusCode;
};

struct InverterListData : public SolarApiReply
{
	/*!
	 * @brief The list of inverters available to the data manager card.
	 */
	QList<InverterInfo> inverters;
};

struct CumulationInverterData : public SolarApiReply
{
	double acPower;
	/*!
	 * @brief Energy generated on current day
	 */
	double dayEnergy;
	/*!
	 * @brief Energy generated in current year
	 */
	double yearEnergy;
	/*!
	 * @brief Energy generated overall
	 */
	double totalEnergy;
};

struct CommonInverterData : public SolarApiReply
{
	QString deviceId;
	double acPower;
	double acCurrent;
	double acVoltage;
	double acFrequency;
	double dcCurrent;
	double dcVoltage;
	/*!
	 * @brief Energy generated on current day
	 */
	double dayEnergy;
	/*!
	 * @brief Energy generated in current year
	 */
	double yearEnergy;
	/*!
	 * @brief Energy generated overall
	 */
	double totalEnergy;
	int statusCode;
	int errorCode;
};

struct ThreePhasesInverterData : public SolarApiReply
{
	QString deviceId;
	double acCurrentPhase1;
	double acVoltagePhase1;
	double acCurrentPhase2;
	double acVoltagePhase2;
	double acCurrentPhase3;
	double acVoltagePhase3;
};

/*!
 * @brief Implements the Fronius solar API.
 * This is the API running on the data manager extension cards which may be
 * installed in Fronius converters.
 * A single data manager card can report information from multiple inverters,
 * if they are chained using the DATCOM interface.
 */
class FroniusSolarApi : public QObject
{
	Q_OBJECT
public:
	FroniusSolarApi(const QString &hostName, int port, QObject *parent = 0);

	QString hostName() const;

	void setHostName(const QString &h);

	int port() const;

	void setPort(int port);

	/*!
	 * @brief retrieves the list of inverters from the data manager specified
	 * by the hostName and port parameters passed to the constructor.
	 * This function is asynchronous and will return immediatlye.
	 * The converterInfoFound signal will be emitted when the API call has been
	 * handled, even if an error has occured.
	 */
	void getConverterInfoAsync();

	/*!
	 * @brief returns cumulated data from all connected inverters.
	 */
	void getCumulationDataAsync(int deviceId);

	/*!
	 * @brief retrieves common data from the specified inverter. Common data
	 * is available from all inverters (with or without 3 phases).
	 * @param deviceId The ID of the inverter. This should be the content of
	 * the `id` field in the InverterInfo object recieved after calling the
	 * getConverterInfoAsync function.
	 * The commonDataFound signal will be emitted when the API call has been
	 * handled, even if an error has occured.
	 */
	void getCommonDataAsync(int deviceId);

	/*!
	 * @brief retrieves values from 3 phase inverters.
	 * @param deviceId The ID of the inverter. This should be the content of
	 * the 'id' field in the InverterInfo object recieved after calling the
	 * getConverterInfoAsync function.
	 * The threePhasesDataFound signal will be emitted when the API call has
	 * been handled, even if an error has occured.
	 */
	void getThreePhasesInverterDataAsync(int deviceId);

	void getSystemDataAsync();

signals:
	/*!
	 * @brief emitted when getConverterInfo request has been completed.
	 * @param data payload
	 */
	void converterInfoFound(const InverterListData &data);

	/*!
	 * @brief emitted when getCumulationData request has been completed.
	 * @param data payload
	 */
	void cumulationDataFound(const CumulationInverterData &data);

	/*!
	 * @brief emitted when getCommonData request has been completed.
	 * @param data payload
	 */
	void commonDataFound(const CommonInverterData &data);

	/*!
	 * @brief emitted when getThreePhasesInverterData request has been
	 * completed.
	 * @param data payload
	 */
	void threePhasesDataFound(const ThreePhasesInverterData &data);

	void systemDataFound(const CumulationInverterData &data);

private slots:
	void onDone(bool error);

	void onTimeout();

private:
	void sendGetRequest(const QUrl &request, const QString &id);

	void processRequest(const QString &networkError);

	void processConverterInfo(const QString &networkError);

	void processCumulationData(const QString &networkError);

	void processCommonData(const QString &networkError);

	void processThreePhasesData(const QString &networkError);

	void processSystemData(const QString &networkError);

	void processReply(const QString &networkError, SolarApiReply &apiReply,
					  QJsonObject &map);

	void updateHttpClient();

	/*!
	 * @brief Retrieves a nested value from the specified map.
	 * @param obj
	 * @param path A list of id's separated by slashes ('/').
	 * @return The nested value or an empty variant of the value could not be
	 * found.
	 */
	static QJsonValue getByPath(const QJsonObject &obj, const QString &path);

	static QJsonObject getChild(const QJsonObject &obj, const QString &path);

	QHttp *mHttp;
	QString mHostName;
	int mPort;
	QString mRequestType;
	QTimer *mTimeoutTimer;
};

#endif // FRONIUSSOLAR_API_H

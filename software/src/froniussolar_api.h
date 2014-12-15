#ifndef FRONIUSSOLAR_API_H
#define FRONIUSSOLAR_API_H

#include <QObject>
#include <QList>
#include <QString>
#include <QUrl>
#include <QVariantMap>

class QNetworkAccessManager;
class QNetworkReply;
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

struct InverterInfo : public SolarApiReply
{
	/*!
	 * @brief The id of the inverter.
	 * This id is unique only within the DATCOM chain is used to address
	 * inverters within this chain. Use this ID in the FroniusSolarApi where
	 * a deviceId is required.
	 */
	QString id;
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
	explicit FroniusSolarApi(QString hostName, int port, QObject *parent = 0);

	QString hostName() const;

	int port() const;

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
	void getCumulationDataAsync();

	/*!
	 * @brief retrieves common data from the specified inverter. Common data
	 * is available from all inverters (with or without 3 phases).
	 * @param deviceId The ID of the inverter. This should be the content of
	 * the `id` field in the InverterInfo object recieved after calling the
	 * getConverterInfoAsync function.
	 * The commonDataFound signal will be emitted when the API call has been
	 * handled, even if an error has occured.
	 */
	void getCommonDataAsync(const QString &deviceId);

	/*!
	 * @brief retrieves values from 3 phase inverters.
	 * @param deviceId The ID of the inverter. This should be the content of
	 * the 'id' field in the InverterInfo object recieved after calling the
	 * getConverterInfoAsync function.
	 * The threePhasesDataFound signal will be emitted when the API call has
	 * been handled, even if an error has occured.
	 */
	void getThreePhasesInverterDataAsync(const QString &deviceId);

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

private slots:
	void onReply();

	void OnTimeout();

private:
	void sendGetRequest(QUrl url, QString id);

	QVariantMap parseReply(QNetworkReply *reply);

	/*!
	 * @brief Retrieves a nested value from the specified map.
	 * @param map
	 * @param path A list of id's separated by slashes ('/').
	 * @return The nested value or an empty variant of the value could not be
	 * found.
	 */
	static QVariant getByPath(const QVariant &map, const QString &path);

	static QNetworkAccessManager *mNam;
	QString mHostName;
	int mPort;
	QNetworkReply *mReply;
	QTimer *mTimeoutTimer;
};

#endif // FRONIUSSOLAR_API_H

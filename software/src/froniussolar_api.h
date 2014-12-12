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
 * \brief Base class for all data packages returned by FroniusSolarApi.
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
	 * \brief The id of the inverter.
	 * This id is unique only within the DATCOM chain is used to address
	 * inverters within this chain. Use this ID in the FroniusSolarApi where
	 * a deviceId is required.
	 */
	QString id;
	/*!
	 * \brief uniqueId Unique ID of the inverter (serial number?)
	 */
	QString uniqueId;
	/*!
	 * \brief customName Name of the inverted as assigned by the user.
	 * May be empty.
	 */
	QString customName;
	/*!
	 * \brief deviceType Type of the device. This value is not documented,
	 * but inverters seem to have value 192. Also seen 123 on an 3 phase
	 * inverter.
	 */
	int deviceType;
	/*!
	 * \brief errorCode
	 * -1	No _valid_ error code
	 *  0	OK
	 */
	int errorCode;
	/*!
	 * \brief statusCode
	 * 0-6	Startup
	 * 7	Running
	 * 8	Standby
	 * 9	Boot loading
	 * 10	Error
	 */
	int statusCode;
};

struct InverterInfoData : public SolarApiReply
{
	QList<InverterInfo> inverters;
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
	 * \brief Energy generated on current day
	 */
	double dayEnergy;
	/*!
	 * \brief Energy generated in current year
	 */
	double yearEnergy;
	/*!
	 * \brief Energy generated overall
	 */
	double totalEnergy;
};

struct CumulationInverterData : public SolarApiReply
{
	QString deviceId;
	double pac;
	double dayEnergy;
	double yearEnergy;
	double totalEnergy;
};

struct ThreePhasesInverterData : public SolarApiReply
{
	QString deviceId;
	double iacL1;
	double uacL1;
	double iacL2;
	double uacL2;
	double iacL3;
	double uacL3;
};

/*!
 * \brief Implements the Fronius solar API.
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
	 * \brief retrieves the list of inverters from the data manager specified
	 * by the hostName and port parameters passed to the constructor.
	 * This function is asynchronous and will return immediatlye.
	 * The converterInfoFound signal will be emitted when the API call has been
	 * handled, even if an error has occured.
	 */
	void getConverterInfoAsync();

	/*!
	 * \brief returns cumulated data from all connected inverters.
	 */
	void getCumulationDataAsync();

	/*!
	 * \brief retrieves 'common' data from the specified inverter. Common data
	 * is available from all inverters (with or without 3 phases).
	 * \param deviceId The ID of the inverter. This should be the content of
	 * the 'id' field in the InverterInfo object recieved after calling the
	 * getConverterInfoAsync function.
	 */
	void getCommonDataAsync(const QString &deviceId);

	void getThreePhasesInverterDataAsync(const QString &deviceId);

signals:
	void converterInfoFound(const InverterInfoData &data);

	void commonDataFound(const CommonInverterData &data);

	void cumulationDataFound(const CumulationInverterData &data);

	void threePhasesDataFound(const ThreePhasesInverterData &data);

private slots:
	void onReply();

	void OnTimeout();

private:
	void sendGetRequest(QUrl url, QString id);

	QVariantMap parseReply(QNetworkReply *reply);

	static QVariant getByPath(const QVariant &map, QString path);

	QString mHostName;
	int mPort;
	static QNetworkAccessManager *mNam;
	QNetworkReply *mReply;
	QTimer *mTimeoutTimer;
};

#endif // FRONIUSSOLAR_API_H

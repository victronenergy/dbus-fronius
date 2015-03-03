#ifndef INVERTER_H
#define INVERTER_H

#include <QObject>
#include "defines.h"

class PowerInfo;
struct FroniusDeviceInfo;

class Inverter : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool isConnected READ isConnected WRITE setIsConnected NOTIFY isConnectedChanged)
	Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
	Q_PROPERTY(int statusCode READ statusCode WRITE setStatusCode NOTIFY statusCodeChanged)
	Q_PROPERTY(int errorCode READ errorCode WRITE setErrorCode NOTIFY errorCodeChanged)
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(int deviceType READ deviceType)
	Q_PROPERTY(QString uniqueId READ uniqueId)
	Q_PROPERTY(QString customName READ customName)
	Q_PROPERTY(QString hostName READ hostName WRITE setHostName NOTIFY hostNameChanged)
	Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
	Q_PROPERTY(int phaseCount READ phaseCount)
	Q_PROPERTY(QString productName READ productName)
public:
	Inverter(const QString &hostName, int port, const QString &id,
			 int deviceType, const QString &uniqueId,
			 const QString &customName, QObject *parent = 0);

	bool isConnected() const;

	void setIsConnected(bool v);

	/*!
	 * Human readable status text based on `errorCode` and `statusCode`.
	 */
	QString status() const;

	void setStatus(const QString &c);

	/*!
	 * Error code as returned by the fronius inverter
	 */
	int errorCode() const;

	void setErrorCode(int code);

	/*!
	 * Status as returned by the fronius inverter
	 * - 0-6: Startup
	 * - 7: Running
	 * - 8: Standby
	 * - 9: Boot loading
	 * - 10: Error
	 */
	int statusCode() const;

	void setStatusCode(int code);

	QString id() const;

	int deviceType() const;

	QString uniqueId() const;

	QString customName() const;

	QString hostName() const;

	void setHostName(const QString &h);

	int port() const;

	void setPort(int p);

	int phaseCount() const;

	QString productName() const;

	PowerInfo *meanPowerInfo();

	PowerInfo *l1PowerInfo();

	PowerInfo *l2PowerInfo();

	PowerInfo *l3PowerInfo();

	PowerInfo *getPowerInfo(InverterPhase phase);

	/*!
	 * @brief Reset all measured values to NaN
	 */
	void resetValues();

signals:
	void isConnectedChanged();

	void statusChanged();

	void hostNameChanged();

	void portChanged();

	void errorCodeChanged();

	void statusCodeChanged();

private:
	bool mIsConnected;
	QString mStatus;
	int mErrorCode;
	int mStatusCode;
	QString mHostName;
	int mPort;
	QString mId;
	int mDeviceType;
	QString mUniqueId;
	QString mCustomName;
	const FroniusDeviceInfo *mDeviceInfo;
	PowerInfo *mMeanPowerInfo;
	PowerInfo *mL1PowerInfo;
	PowerInfo *mL2PowerInfo;
	PowerInfo *mL3PowerInfo;
};

#endif // INVERTER_H

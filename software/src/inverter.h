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
	Q_PROPERTY(int deviceInstance READ deviceInstance WRITE setDeviceInstance NOTIFY deviceInstanceChanged)
	Q_PROPERTY(double powerLimit READ powerLimit WRITE setPowerLimit NOTIFY powerLimitChanged)
	Q_PROPERTY(double minPowerLimit READ minPowerLimit WRITE setMinPowerLimit NOTIFY minPowerLimitChanged)
	Q_PROPERTY(double powerLimitStepSize READ powerLimitStepSize WRITE setPowerLimitStepSize NOTIFY powerLimitStepSizeChanged)
	Q_PROPERTY(double maxPower READ maxPower WRITE setMaxPower NOTIFY maxPowerChanged)
public:
	Inverter(const QString &hostName, int port, const QString &id,
			 int deviceType, const QString &uniqueId,
			 const QString &customName, QObject *parent = 0);

	bool isConnected() const;

	void setIsConnected(bool v);

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

	int deviceInstance() const;

	void setDeviceInstance(int instance);

	PowerInfo *meanPowerInfo();

	PowerInfo *l1PowerInfo();

	PowerInfo *l2PowerInfo();

	PowerInfo *l3PowerInfo();

	PowerInfo *getPowerInfo(InverterPhase phase);

	double powerLimit() const
	{
		return mPowerLimit;
	}

	void setPowerLimit(double p);

	void setRequestedPowerLimit(double p);

	double powerLimitStepSize() const
	{
		return mPowerLimitStepSize;
	}

	void setPowerLimitStepSize(double p);

	double minPowerLimit() const
	{
		return mMinPowerLimit;
	}

	void setMinPowerLimit(double p);

	double maxPower() const
	{
		return mMaxPower;
	}

	void setMaxPower(double p);

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

	void deviceInstanceChanged();

	void powerLimitChanged();

	void powerLimitRequested(double value);

	void powerLimitStepSizeChanged();

	void minPowerLimitChanged();

	void maxPowerChanged();

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
	int mDeviceInstance;
	const FroniusDeviceInfo *mDeviceInfo;
	PowerInfo *mMeanPowerInfo;
	PowerInfo *mL1PowerInfo;
	PowerInfo *mL2PowerInfo;
	PowerInfo *mL3PowerInfo;
	double mPowerLimit;
	double mPowerLimitStepSize;
	double mMinPowerLimit;
	double mMaxPower;
};

#endif // INVERTER_H

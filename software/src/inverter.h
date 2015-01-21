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
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(int deviceType READ deviceType)
	Q_PROPERTY(QString uniqueId READ uniqueId)
	Q_PROPERTY(QString customName READ customName)
	Q_PROPERTY(QString hostName READ hostName WRITE setHostName NOTIFY hostNameChanged)
	Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
	Q_PROPERTY(bool supports3Phases READ supports3Phases)
	Q_PROPERTY(QString productName READ productName)
public:
	Inverter(const QString &hostName, int port, const QString &id,
			 int deviceType, const QString &uniqueId,
			 const QString &customName, QObject *parent = 0);

	bool isConnected() const;

	void setIsConnected(bool v);

	QString status() const;

	void setStatus(const QString &c);

	QString id() const;

	int deviceType() const;

	QString uniqueId() const;

	QString customName() const;

	QString hostName() const;

	void setHostName(const QString &h);

	int port() const;

	void setPort(int p);

	bool supports3Phases() const;

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

private:
	bool mIsConnected;
	QString mStatus;
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

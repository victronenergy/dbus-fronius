#ifndef INVERTER_H
#define INVERTER_H

#include <QObject>
#include "defines.h"

class PowerInfo;

class Inverter : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool isConnected READ isConnected WRITE setIsConnected NOTIFY isConnectedChanged)
	Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString uniqueId READ uniqueId)
	Q_PROPERTY(QString customName READ customName)
	Q_PROPERTY(QString hostName READ hostName)
	Q_PROPERTY(QString port READ port)
public:
	Inverter(const QString &hostName, int port, const QString &id,
			 const QString &uniqueId, const QString &customName,
			 QObject *parent = 0);

	bool isConnected() const;

	void setIsConnected(bool v);

	QString status() const;

	void setStatus(const QString &c);

	QString id() const;

	QString uniqueId() const;

	QString customName() const;

	QString hostName() const;

	int port() const;

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

	void supports3PhasesChanged();

	void statusChanged();

private:
	bool mIsConnected;
	QString mStatus;
	QString mHostName;
	int mPort;
	QString mId;
	QString mUniqueId;
	QString mCustomName;
	PowerInfo *mMeanPowerInfo;
	PowerInfo *mL1PowerInfo;
	PowerInfo *mL2PowerInfo;
	PowerInfo *mL3PowerInfo;
};

#endif // INVERTER_H

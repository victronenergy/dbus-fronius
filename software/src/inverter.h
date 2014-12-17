#ifndef INVERTER_H
#define INVERTER_H

#include <QObject>

class PowerInfo;

class Inverter : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int isConnected READ isConnected WRITE setIsConnected NOTIFY isConnectedChanged)
	Q_PROPERTY(bool supports3Phases READ supports3Phases WRITE setSupports3Phases NOTIFY supports3PhasesChanged)
	Q_PROPERTY(QString id READ id)
	Q_PROPERTY(QString uniqueId READ uniqueId)
	Q_PROPERTY(QString hostName READ hostName)
	Q_PROPERTY(QString port READ port)
public:
	Inverter(const QString &hostName, int port, const QString &id,
			 const QString &uniqueId, QObject *parent = 0);

	int isConnected() const;

	void setIsConnected(int v);

	bool supports3Phases() const;

	void setSupports3Phases(bool p);

	QString id() const;

	QString uniqueId() const;

	QString hostName() const;

	int port() const;

	PowerInfo *meanPowerInfo();

	PowerInfo *l1PowerInfo();

	PowerInfo *l2PowerInfo();

	PowerInfo *l3PowerInfo();

signals:
	void isConnectedChanged();

	void supports3PhasesChanged();

private:
	int mIsConnected;
	bool mSupports3Phases;
	QString mHostName;
	int mPort;
	QString mId;
	QString mUniqueId;
	PowerInfo *mMeanPowerInfo;
	PowerInfo *mL1PowerInfo;
	PowerInfo *mL2PowerInfo;
	PowerInfo *mL3PowerInfo;
};

#endif // INVERTER_H

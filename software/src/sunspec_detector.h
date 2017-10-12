#ifndef SUNSPEC_DETECTOR_H
#define SUNSPEC_DETECTOR_H

#include <QAbstractSocket>
#include "abstract_detector.h"
#include "defines.h"

class ModbusReply;
class ModbusTcpClient;

class SunspecDetector : public AbstractDetector
{
	Q_OBJECT
public:
	SunspecDetector(QObject *parent = 0);

	SunspecDetector(quint8 unitId, QObject *parent = 0);

	virtual DetectorReply *start(const QString &hostName);

	quint8 unitId() const
	{
		return mUnitId;
	}

	void setUnitId(quint8 unitId)
	{
		mUnitId = unitId;
	}

private slots:
	void onConnected();

	void onDisconnected();

	void onFinished();

private:
	class Reply : public DetectorReply
	{
	public:
		Reply(QObject *parent = 0);

		virtual ~Reply();

		virtual QString hostName() const
		{
			return di.hostName;
		}

		void setResult()
		{
			emit deviceFound(di);
		}

		void setFinished()
		{
			emit finished();
		}

		enum State {
			SunSpecHeader,
			ModuleHeader,
			ModuleContent
		};

		DeviceInfo di;
		ModbusTcpClient *client;
		State state;
		quint16 currentRegister;
	};

	void startNextRequest(Reply *di, quint16 regCount);

	void setDone(Reply *di);

	QHash<ModbusTcpClient *, Reply *> mClientToReply;
	QHash<ModbusReply *, Reply *> mModbusReplyToReply;
	quint8 mUnitId;
};

#endif // SUNSPEC_DETECTOR_H

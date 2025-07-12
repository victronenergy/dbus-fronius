#ifndef SUNSPEC_DETECTOR_H
#define SUNSPEC_DETECTOR_H

#include <QAbstractSocket>
#include <QHash>
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

	DetectorReply *start(const QString &hostName, int timeout) override;
	DetectorReply *start(const QString &hostName, int timeout, quint8 unitId);

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

		QString hostName() const override
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
		quint16 currentModel;
		quint16 nextModelRegister;
	};

	void requestNextHeader(Reply *di);
	void requestNextContent(Reply *di, quint16 currentModel, quint16 nextModelRegister, quint16 regCount, quint16 offset = 0);
	void startNextRequest(Reply *di, quint16 regCount);

	void checkDone(Reply *di);
	void setDone(Reply *di);

	QHash<ModbusTcpClient *, Reply *> mClientToReply;
	QHash<ModbusReply *, Reply *> mModbusReplyToReply;
	quint8 mUnitId;
};

#endif // SUNSPEC_DETECTOR_H

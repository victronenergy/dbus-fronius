#ifndef MODBUSTCPCLIENT_H
#define MODBUSTCPCLIENT_H

#include <QAbstractSocket>
#include <QObject>
#include "modbus_client.h"
#include "modbus_reply.h"

class QTimer;

class ModbusTcpClient: public ModbusClient
{
	Q_OBJECT
public:
	static const quint16 DefaultTcpPort = 502;

	ModbusTcpClient(QObject *parent = 0);

	void connectToServer(const QString &hostName, quint16 tcpPort = DefaultTcpPort);

	bool isConnected() const;

	virtual ModbusReply *readHoldingRegisters(quint8 unitId, quint16 startReg, quint16 count);

	virtual ModbusReply *readInputRegisters(quint8 unitId, quint16 startReg, quint16 count);

	virtual ModbusReply *writeSingleHoldingRegister(quint8 unitId, quint16 reg, quint16 value);

	virtual ModbusReply *writeMultipleHoldingRegisters(quint8 unitId, quint16 startReg,
													   const QVector<quint16> &values);

	QString hostName() const;

	quint16 portName() const;

	virtual int timeout() const;

	virtual void setTimeout(int t);

signals:
	void connected();

	void disconnected();

protected:
	virtual void timerEvent(QTimerEvent *event);

private slots:
	void onConnected();

	void onReadyRead();

	void onReplyDestroyed();

	void onSocketErrorReceived(QAbstractSocket::SocketError error);

private:
	enum FunctionCode
	{
		ReadCoils						= 1,
		ReadDiscreteInputs				= 2,
		ReadHoldingRegisters			= 3,
		ReadInputRegisters				= 4,
		WriteSingleCoil					= 5,
		WriteSingleRegister				= 6,
		WriteMultipleCoils				= 15,
		WriteMultipleRegisters			= 16,
		ReadFileRecord					= 20,
		WriteFileRecord					= 21,
		MaskWriteRegister				= 22,
		ReadWriteMultipleRegisters		= 23,
		ReadFIFOQueue					= 24,
		EncapsulatedInterfaceTransport	= 43,
	};

	class Reply : public ModbusReply {
	public:
		Reply(quint16 transactionId, int timeout, QObject *parent = 0);

		quint16 transactionId() const
		{
			return mTransactionId;
		}

		using ModbusReply::setResult;

		virtual bool isFinished() const;

		virtual void timerEvent(QTimerEvent *event);

	private:
		virtual void onFinished();

		int mTimerId;
		quint16 mTransactionId;
	};

	ModbusReply *readRegisters(FunctionCode function, quint8 unitId, quint16 startReg,
							   quint16 count);

	ModbusReply *sendFrame(const QByteArray &payload);

	Reply *popReply(quint16 transactionId);

	QByteArray createFrame(FunctionCode function, quint8 unitId, quint8 count);

	void setFinished(quint16 transactionId, const QVector<quint16> &values);

	void setFinished(quint16 transactionId, int error);

	QHash<quint16, Reply *> mPendingReplies;
	QAbstractSocket *mSocket;
	int mTimeout;
	int mConnectTimerId;
	QByteArray mBuffer;
	QString mHostName;
	quint16 mTcpPort;
	quint16 mTransactionId;
};

#endif // MODBUSTCPCLIENT_H

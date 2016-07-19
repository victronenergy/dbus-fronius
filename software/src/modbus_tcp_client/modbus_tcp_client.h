#ifndef MODBUSTCPCLIENT_H
#define MODBUSTCPCLIENT_H

#include <QList>
#include <QObject>

class QTcpSocket;

class ModbusTcpClient: public QObject
{
	Q_OBJECT
public:
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

	enum ExceptionCode
	{
		NoExeption							= 0,
		IllegalFunction						= 1,
		IllegalDataAddress					= 2,
		IllegalDataValue					= 3,
		SlaveDeviceFailure					= 4,
		Acknowledge							= 5,
		SlaveDeviceBusy						= 6,
		MemoryParityError					= 7,
		GatewayPathUnavailable				= 10,
		GatewayTargetDeviceFailedToRespond	= 11
	};

	ModbusTcpClient(QObject *parent = 0);

	void connectToServer(const QString &hostName, quint16 port = 502);

	bool isConnected() const;

	void readHoldingRegisters(quint8 unitId, quint16 startReg, quint16 count);

	void readInputRegisters(quint8 unitId, quint16 startReg, quint16 count);

	void writeSingleHoldingRegister(quint8 unitId, quint16 reg, quint16 value);

	void writeMultipleHoldingRegisters(quint8 unitId, quint16 startReg,
									   const QList<quint16> &values);

signals:
	void readHoldingRegistersCompleted(quint8 unitId, QList<quint16> values);

	void readInputRegistersCompleted(quint8 unitId, QList<quint16> values);

	void writeSingleHoldingRegisterCompleted(quint8 unitId, quint16 startReg, quint16 value);

	void writeMultipleHoldingRegistersCompleted(quint8 unitId, quint16 startReg, quint16 regCount);

	void errorReceived(quint8 functionCode, quint8 unitId, quint8 exception);

	void connected();

	void disconnected();

private slots:
	void onReadyRead();

private:
	void readRegisters(FunctionCode function, quint8 unitId, quint16 startReg, quint16 count);

	QByteArray createFrame(FunctionCode function, quint8 unitId, quint8 count);

	QTcpSocket *mSocket;
	QByteArray mBuffer;
	quint16 mTransactionId;
};

inline quint8 msb(quint16 d)
{
	return d >> 8;
}

inline quint8 lsb(quint16 d)
{
	return d & 0xFF;
}

inline quint16 toUInt16(quint8 msb, quint8 lsb)
{
	return static_cast<quint16>((msb << 8) | lsb);
}

inline quint16 toUInt16(const QByteArray &a, int offset)
{
	return toUInt16(static_cast<quint8>(a[offset]), static_cast<quint8>(a[offset + 1]));
}

#endif // MODBUSTCPCLIENT_H

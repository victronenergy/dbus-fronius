#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <QByteArray>
#include <QList>
#include <QMetaType>
#include <QMutex>
#include <QObject>
extern "C" {
	#include <velib/platform/serial.h>
}
#include "crc16.h"
#include "modbus_client.h"
#include "modbus_reply.h"

class QTimer;

Q_DECLARE_METATYPE(QList<quint16>)

/*!
 * Partial implementation of the Modbus RTU protocol.
 *
 * Supported functions: `ReadHoldingRegisters`, `ReadInputRegisters`,
 * and `WriteSingleRegister`.
 *
 * Communication is implemented asynchronously. It is allowed to add multiple
 * request at once. They will be queued and sent to the device whenever it is
 * ready (ie. all previous requests have been handled).
 */
class ModbusRtuClient : public ModbusClient
{
	Q_OBJECT
public:
	ModbusRtuClient(const QString &portName, int baudrate, QObject *parent = 0);

	~ModbusRtuClient();

	virtual ModbusReply *readHoldingRegisters(quint8 unitId, quint16 startReg, quint16 count);

	virtual ModbusReply *readInputRegisters(quint8 unitId, quint16 startReg, quint16 count);

	virtual ModbusReply *writeSingleHoldingRegister(quint8 unitId, quint16 reg, quint16 value);

	virtual ModbusReply *writeMultipleHoldingRegisters(quint8 unitId, quint16 startReg,
													   const QVector<quint16> &values);

	virtual int timeout() const;

	virtual void setTimeout(int t);

signals:
	void serialEvent(const char *message);

private slots:
	void onTimeout();

	bool processPacket();

	void onReadyRead();

	void onError();

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
		Reply(QObject *parent = 0);

		using ModbusReply::setResult;

		virtual bool isFinished() const;

		QVector<quint16> values;
		FunctionCode function;
		quint8 slaveAddress;
		quint16 reg;
		quint16 count;
		bool finished;

	private:
		virtual void onFinished();
	};

	void handleByteRead(quint8 b);

	void resetStateEngine(bool toIdle);

	void processPending();

	ModbusReply *readRegisters(FunctionCode function, quint8 slaveAddress, quint16 startReg, quint16 count);

	void send(Reply *reply);

	enum ReadState {
		Idle,
		Address,
		Function,
		ByteCount,
		StartAddressMsb,
		StartAddressLsb,
		Data,
		CrcMsb,
		CrcLsb
	};

	VeSerialPort *mSerialPort;
	QTimer *mTimer;
	QList<Reply *> mPendingCommands;
	Reply *mActiveReply;

	// State engine
	ReadState mState;
	FunctionCode mFunction;
	quint8 mCount;
	quint16 mUnitId;
	quint16 mStartAddress;
	quint16 mCrc;
	Crc16 mCrcBuilder;
	bool mAddToCrc;
	QByteArray mData;
};

#endif // MODBUS_RTU_H

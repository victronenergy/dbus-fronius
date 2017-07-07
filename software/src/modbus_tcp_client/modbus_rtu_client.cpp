#include <QSocketNotifier>
#include <QTimer>
#include <QVector>
#include <unistd.h>
#include "defines.h"
#include "modbus_rtu_client.h"

ModbusRtuClient::ModbusRtuClient(const QString &portName, int baudrate, QObject *parent):
	ModbusClient(parent),
	mSerialPort(veSerialAllocate(portName.toLatin1().data())),
	mTimer(new QTimer(this)),
	mActiveReply(0)
{
	veSerialSetBaud(mSerialPort, static_cast<un32>(baudrate));
	veSerialSetKind(mSerialPort, 0); // Requires external event pump
	veSerialOpen(mSerialPort, 0);

	QSocketNotifier *readNotifier =
		new QSocketNotifier(mSerialPort->fh, QSocketNotifier::Read, this);
	connect(readNotifier, SIGNAL(activated(int)), this, SLOT(onReadyRead()));

	QSocketNotifier *errorNotifier =
		new QSocketNotifier(mSerialPort->fh, QSocketNotifier::Exception, this);
	connect(errorNotifier, SIGNAL(activated(int)), this, SLOT(onError()));

	mData.reserve(16);

	resetStateEngine(true);
	mTimer->setInterval(1000);
	mTimer->setSingleShot(true);
	connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

ModbusRtuClient::~ModbusRtuClient()
{
	veSerialClose(mSerialPort);
	VeSerialPortFree(mSerialPort);
}

ModbusReply *ModbusRtuClient::readHoldingRegisters(quint8 unitId, quint16 startReg, quint16 count)
{
	return readRegisters(ReadHoldingRegisters, unitId, startReg, count);
}

ModbusReply *ModbusRtuClient::readInputRegisters(quint8 unitId, quint16 startReg, quint16 count)
{
	return readRegisters(ReadInputRegisters, unitId, startReg, count);
}

ModbusReply *ModbusRtuClient::writeSingleHoldingRegister(quint8 unitId, quint16 reg, quint16 value)
{
	Reply *cmd = new Reply(this);
	cmd->function = WriteSingleRegister;
	cmd->slaveAddress = unitId;
	cmd->reg = reg;
	QVector<quint16> values;
	values.append(value);
	cmd->values = values;
	send(cmd);
	return cmd;
}

ModbusReply *ModbusRtuClient::writeMultipleHoldingRegisters(quint8 unitId, quint16 startReg,
															const QVector<quint16> &values)
{
	Reply *cmd = new Reply(this);
	cmd->function = WriteMultipleRegisters;
	cmd->slaveAddress = unitId;
	cmd->reg = startReg;
	cmd->values = values;
	send(cmd);
	return cmd;
}

int ModbusRtuClient::timeout() const
{
	return mTimer->interval();
}

void ModbusRtuClient::setTimeout(int t)
{
	mTimer->setInterval(t);
}

void ModbusRtuClient::onTimeout()
{
	if (mState == Idle || mActiveReply == 0)
		return;
	mActiveReply->setResult(ModbusReply::Timeout);
	resetStateEngine(true);
	processPending();
}

bool ModbusRtuClient::processPacket()
{
	if (mActiveReply == 0)
		return true;
	if (mCrc != mCrcBuilder.getValue()) {
		// mActiveReply->setResult(ModbusReply::CrcError);
		return false;
	}
	if (mUnitId != mActiveReply->slaveAddress)
		return false;
	if ((mFunction & 0x7F) != mActiveReply->function)
		return false;
	if ((mFunction & 0x80) != 0) {
		quint8 errorCode = static_cast<quint8>(mData[0]);
		mActiveReply->setResult(static_cast<ModbusReply::ExceptionCode>(errorCode));
		return true;
	}
	switch (mFunction) {
	case ReadHoldingRegisters:
	case ReadInputRegisters:
	{
		QVector<quint16> registers;
		for (int i=0; i<mData.length(); i+=2) {
			registers.append(toUInt16(mData, i));
		}
		mActiveReply->setResult(registers);
		break;
	}
	case WriteSingleRegister:
	{
		QVector<quint16> registers;
		registers.append(toUInt16(mData, 0));
		mActiveReply->setResult(registers);
		break;
	}
	case WriteMultipleRegisters:
	{
		mActiveReply->setResult(ModbusReply::NoException);
		break;
	}
	default:
		return false;
	}
	return true;
}

void ModbusRtuClient::onReadyRead()
{
	quint8 buf[64];
	bool first = true;
	for (;;) {
		ssize_t len = read(mSerialPort->fh, buf, sizeof(buf));
		if (len < 0) {
			emit serialEvent("serial read failure");
			return;
		}
		if (first && len == 0) {
			emit serialEvent("Ready for reading but read 0 bytes. Device removed?");
			return;
		}
		for (ssize_t i = 0; i<len; ++i)
			handleByteRead(buf[i]);
		if (len < static_cast<int>(sizeof(buf)))
			break;
		first = false;
	}
}

void ModbusRtuClient::onError()
{
	emit serialEvent("Serial error");
}

void ModbusRtuClient::handleByteRead(quint8 b)
{
	if (mAddToCrc)
		mCrcBuilder.add(b);
	switch (mState) {
	case Idle:
		// We received data when we were not expecting any. Ignore the data.
		break;
	case Address:
		mState = Function;
		mUnitId = b;
		break;
	case Function:
		mFunction = static_cast<FunctionCode>(b);
		if ((mFunction & 0x80) != 0) {
			// Exception
			mCount = 1;
			mState = Data;
		} else {
			switch (mFunction) {
			case ReadHoldingRegisters:
			case ReadInputRegisters:
				mState = ByteCount;
				break;
			case WriteSingleRegister:
				mState = StartAddressMsb;
				break;
			case WriteMultipleRegisters:
				mState = StartAddressMsb;
				break;
			default:
				resetStateEngine(false);
				break;
			}
		}
		break;
	case ByteCount:
		mCount = b;
		if (mCount == 0) {
			mState = CrcMsb;
			mAddToCrc = false;
		} else {
			mState = Data;
		}
		break;
	case StartAddressMsb:
		mStartAddress = static_cast<quint16>(b << 8);
		mState = StartAddressLsb;
		break;
	case StartAddressLsb:
		Q_ASSERT(mFunction == WriteSingleRegister || mFunction == WriteMultipleRegisters);
		mStartAddress |= b;
		mCount = 2;
		mState = Data;
		break;
	case Data:
		if (mCount > 0) {
			mData.append(static_cast<char>(b));
			--mCount;
		}
		if (mCount == 0) {
			mState = CrcMsb;
			mAddToCrc = false;
		}
		break;
	case CrcMsb:
		mCrc = static_cast<quint16>(b << 8);
		mState = CrcLsb;
		break;
	case CrcLsb:
		mCrc |= b;
		if (processPacket()) {
			resetStateEngine(true);
			processPending();
		} else {
			resetStateEngine(false);
		}
		break;
	}
}

void ModbusRtuClient::resetStateEngine(bool toIdle)
{
	mState = toIdle ? Idle : Address;
	mCrcBuilder.reset();
	mAddToCrc = true;
	mData.clear();
	if (toIdle)
		mTimer->stop();
}

void ModbusRtuClient::processPending()
{
	if (mPendingCommands.isEmpty()) {
		mActiveReply = 0;
		return;
	}
	Reply *cmd = mPendingCommands.first();
	mPendingCommands.removeFirst();
	send(cmd);
}

ModbusReply *ModbusRtuClient::readRegisters(FunctionCode function, quint8 slaveAddress,
											quint16 startReg, quint16 count)
{
	Reply *cmd = new Reply(this);
	cmd->function = function;
	cmd->slaveAddress = slaveAddress;
	cmd->reg = startReg;
	cmd->count = count;
	send(cmd);
	return cmd;
}

void ModbusRtuClient::send(ModbusRtuClient::Reply *reply)
{
	if (mState != Idle) {
		Q_ASSERT(mActiveReply == 0);
		mPendingCommands.append(reply);
		return;
	}
	QByteArray frame;
	frame.reserve(10);
	frame.append(static_cast<char>(reply->slaveAddress));
	frame.append(static_cast<char>(reply->function));
	switch (reply->function) {
	case ReadHoldingRegisters:
	case ReadInputRegisters:
		frame.append(static_cast<char>(msb(reply->reg)));
		frame.append(static_cast<char>(lsb(reply->reg)));
		frame.append(static_cast<char>(msb(reply->count)));
		frame.append(static_cast<char>(lsb(reply->count)));
		break;
	case WriteSingleRegister:
		Q_ASSERT(reply->values.count() == 1);
		frame.append(static_cast<char>(msb(reply->reg)));
		frame.append(static_cast<char>(lsb(reply->reg)));
		frame.append(static_cast<char>(msb(reply->values.first())));
		frame.append(static_cast<char>(lsb(reply->values.first())));
		break;
	case WriteMultipleRegisters:
	{
		Q_ASSERT(!reply->values.isEmpty());
		int count = reply->values.count();
		frame.append(static_cast<char>(msb(reply->reg)));
		frame.append(static_cast<char>(lsb(reply->reg)));
		frame.append(static_cast<char>(msb(count)));
		frame.append(static_cast<char>(lsb(count)));
		frame.append(static_cast<char>(msb(2 * count)));
		frame.append(static_cast<char>(lsb(2 * count)));
		break;
	}
	default:
		qFatal("Unsupported function");
		mTimer->start();
		return;
	}
	mActiveReply = reply;
	quint16 crc = Crc16::getValue(frame);
	frame.append(static_cast<char>(msb(crc)));
	frame.append(static_cast<char>(lsb(crc)));
	// Modbus requires a pause between sending of 3.5 times the interval needed
	// to send a character. We use 4 characters here, just in case...
	// We also assume 10 bits per caracter (8 data bits, 1 stop bit and 1 parity
	// bit). Keep in mind that overestimating the character time does not hurt
	// (a lot), but underestimating does.
	// Then number of bits devided by the the baudrate (unit: bits/second) gives
	// us the time in seconds. usleep wants time in microseconds, so we have to
	// multiply by 1 million.
	usleep((4 * 10 * 1000 * 1000) / mSerialPort->baudrate);
	veSerialPutBuf(mSerialPort, reinterpret_cast<un8 *>(frame.data()),
				   static_cast<un32>(frame.size()));
	mTimer->start();
	mState = Address;
}

ModbusRtuClient::Reply::Reply(QObject *parent):
	ModbusReply(parent),
	function(ReadHoldingRegisters),
	slaveAddress(0),
	reg(0),
	count(0),
	finished(false)
{
}

bool ModbusRtuClient::Reply::isFinished() const
{
	return finished;
}

void ModbusRtuClient::Reply::onFinished()
{
	Q_ASSERT(!finished);
	finished = true;
}

#include <QTcpSocket>
#include "modbus_tcp_client.h"

ModbusTcpClient::ModbusTcpClient(QObject *parent):
	QObject(parent),
	mSocket(new QTcpSocket(this)),
	mTransactionId(0)
{
	connect(mSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(mSocket, SIGNAL(connected()), this, SIGNAL(connected()));
	connect(mSocket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
	mSocket->socketOption(QAbstractSocket::LowDelayOption);
}

void ModbusTcpClient::readInputRegisters(quint8 unitId, quint16 startReg, quint16 count)
{
	readRegisters(ReadInputRegisters, unitId, startReg, count);
}

void ModbusTcpClient::connectToServer(const QString &hostName, quint16 port)
{
	mSocket->connectToHost(hostName, port);
}

bool ModbusTcpClient::isConnected() const
{
	return mSocket->state() == QTcpSocket::ConnectedState;
}

void ModbusTcpClient::readHoldingRegisters(quint8 unitId, quint16 startReg, quint16 count)
{
	readRegisters(ReadHoldingRegisters, unitId, startReg, count);
}

void ModbusTcpClient::writeSingleHoldingRegister(quint8 unitId, quint16 reg, quint16 value)
{
	QByteArray frame = createFrame(WriteSingleRegister, unitId, 4);
	frame.append(static_cast<char>(msb(reg)));
	frame.append(static_cast<char>(lsb(reg)));
	frame.append(static_cast<char>(msb(value)));
	frame.append(static_cast<char>(lsb(value)));
	mSocket->write(frame);
}

void ModbusTcpClient::writeMultipleHoldingRegisters(quint8 unitId, quint16 startReg,
													const QList<quint16> &values)
{
	QByteArray frame = createFrame(WriteMultipleRegisters, unitId, 4 + 2 * values.size());
	frame.append(static_cast<char>(msb(startReg)));
	frame.append(static_cast<char>(lsb(startReg)));
	frame.append(static_cast<char>(msb(values.size())));
	frame.append(static_cast<char>(lsb(values.size())));
	frame.append(static_cast<char>(values.size() * 2));
	foreach (quint16 value, values) {
		frame.append(static_cast<char>(msb(value)));
		frame.append(static_cast<char>(lsb(value)));
	}
	mSocket->write(frame);
}

void ModbusTcpClient::onReadyRead()
{
	mBuffer.append(mSocket->read(mSocket->bytesAvailable()));
	for (;;) {
		if (mBuffer.size() < 6)
			return;
		int length = toUInt16(mBuffer, 4);
		if (mBuffer.size() < 6 + length)
			return;
		Q_ASSERT(toUInt16(mBuffer, 0) == mTransactionId);
		Q_ASSERT(toUInt16(mBuffer, 2) == 0);
		quint8 unitId = static_cast<quint8>(mBuffer[6]);
		int functionCode = mBuffer[7];
		if ((functionCode & 0x80) == 0) {
			switch (functionCode) {
			case ReadHoldingRegisters:
			case ReadInputRegisters:
				if (mBuffer.size() > 10) {
					// quint8 regCount = mBuffer[8];
					QList<quint16> registers;
					for (int i=9; i<mBuffer.size(); i+=2)
						registers.append(toUInt16(mBuffer, i));
					if (functionCode == ReadHoldingRegisters)
						emit readHoldingRegistersCompleted(unitId, registers);
					else
						emit readInputRegistersCompleted(unitId, registers);
				}
				break;
			case WriteSingleRegister:
				if (mBuffer.size() > 11) {
					quint16 startReg = toUInt16(mBuffer, 8);
					quint16 value = toUInt16(mBuffer, 10);
					emit writeSingleHoldingRegisterCompleted(unitId, startReg, value);
				}
				break;
			case WriteMultipleRegisters:
				if (mBuffer.size() > 11) {
					quint16 startReg = toUInt16(mBuffer, 8);
					quint16 regCount = toUInt16(mBuffer, 10);
					emit writeMultipleHoldingRegistersCompleted(unitId, startReg, regCount);
				}
				break;
			}
		} else {
			if (mBuffer.size() > 8) {
				int error = mBuffer[8];
				emit errorReceived(functionCode & 0x7F, unitId, error);
			}
		}
		mBuffer.remove(0, 6 + length);
	}
}

void ModbusTcpClient::readRegisters(FunctionCode function, quint8 unitId, quint16 startReg,
									quint16 count)
{
	QByteArray frame = createFrame(function, unitId, 4);
	frame.append(static_cast<char>(msb(startReg)));
	frame.append(static_cast<char>(lsb(startReg)));
	frame.append(static_cast<char>(msb(count)));
	frame.append(static_cast<char>(lsb(count)));
	mSocket->write(frame);
}

QByteArray ModbusTcpClient::createFrame(ModbusTcpClient::FunctionCode function, quint8 unitId,
										quint8 count)
{
	++mTransactionId;
	QByteArray frame;
	frame.append(static_cast<char>(msb(mTransactionId)));
	frame.append(static_cast<char>(lsb(mTransactionId)));
	frame.append(static_cast<char>(0));
	frame.append(static_cast<char>(0));
	frame.append(static_cast<char>(0));
	frame.append(static_cast<char>(count + 2));
	frame.append(static_cast<char>(unitId));
	frame.append(static_cast<char>(function));
	return frame;
}


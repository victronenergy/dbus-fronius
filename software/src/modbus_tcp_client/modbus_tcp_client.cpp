#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>
#include "crc16.h"
#include "modbus_tcp_client.h"
#include "modbus_reply.h"

ModbusTcpClient::ModbusTcpClient(QObject *parent):
	ModbusClient(parent),
	mSocket(new QTcpSocket(this)),
	mTimeout(1000),
	mConnectTimerId(0),
	mTransactionId(0)
{
	mSocket->socketOption(QAbstractSocket::LowDelayOption);
	connect(mSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(mSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(mSocket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
	#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	connect(mSocket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(onSocketErrorReceived(QAbstractSocket::SocketError)));
	#else
	connect(mSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
			this, SLOT(onSocketErrorReceived(QAbstractSocket::SocketError)));
	#endif
}

void ModbusTcpClient::connectToServer(const QString &hostName, quint16 tcpPort)
{
	mHostName = hostName;
	mTcpPort = tcpPort;
	mConnectTimerId = startTimer(mTimeout);
	mSocket->connectToHost(hostName, tcpPort);
}

bool ModbusTcpClient::isConnected() const
{
	return mSocket->state() == QTcpSocket::ConnectedState;
}

ModbusReply *ModbusTcpClient::readInputRegisters(quint8 unitId, quint16 startReg, quint16 count)
{
	return readRegisters(ReadInputRegisters, unitId, startReg, count);
}

ModbusReply *ModbusTcpClient::readHoldingRegisters(quint8 unitId, quint16 startReg, quint16 count)
{
	return readRegisters(ReadHoldingRegisters, unitId, startReg, count);
}

ModbusReply *ModbusTcpClient::writeSingleHoldingRegister(quint8 unitId, quint16 reg, quint16 value)
{
	QByteArray frame = createFrame(WriteSingleRegister, unitId, 4);
	frame.append(static_cast<char>(msb(reg)));
	frame.append(static_cast<char>(lsb(reg)));
	frame.append(static_cast<char>(msb(value)));
	frame.append(static_cast<char>(lsb(value)));
	return sendFrame(frame);
}

ModbusReply *ModbusTcpClient::writeMultipleHoldingRegisters(quint8 unitId, quint16 startReg,
															const QVector<quint16> &values)
{
	QByteArray frame = createFrame(WriteMultipleRegisters, unitId, 5 + 2 * values.size());
	frame.append(static_cast<char>(msb(startReg)));
	frame.append(static_cast<char>(lsb(startReg)));
	frame.append(static_cast<char>(msb(values.size())));
	frame.append(static_cast<char>(lsb(values.size())));
	frame.append(static_cast<char>(values.size() * 2));
	foreach (quint16 value, values) {
		frame.append(static_cast<char>(msb(value)));
		frame.append(static_cast<char>(lsb(value)));
	}
	return sendFrame(frame);
}

QString ModbusTcpClient::hostName() const
{
	return mHostName;
}

quint16 ModbusTcpClient::portName() const
{
	return mTcpPort;
}

int ModbusTcpClient::timeout() const
{
	return mTimeout;
}

void ModbusTcpClient::setTimeout(int t)
{
	mTimeout = t;
}

void ModbusTcpClient::timerEvent(QTimerEvent *event)
{
	Q_UNUSED(event)
	Q_ASSERT(mConnectTimerId > 0);
	if (mConnectTimerId == 0)
		return;
	killTimer(mConnectTimerId);
	mConnectTimerId = 0;
	mSocket->disconnectFromHost();
	emit disconnected();
}

void ModbusTcpClient::onConnected()
{
	if (mConnectTimerId > 0) {
		killTimer(mConnectTimerId);
		mConnectTimerId = 0;
	}
	emit connected();
}

void ModbusTcpClient::onReadyRead()
{
	mBuffer.append(mSocket->read(mSocket->bytesAvailable()));
	for (;;) {
		if (mBuffer.size() < 6)
			return;
		int length = toUInt16(mBuffer, 4) + 6;
		if (mBuffer.size() < length)
			return;
		quint16 transactionId = toUInt16(mBuffer, 0);
		Q_ASSERT(toUInt16(mBuffer, 2) == 0);
		// quint8 unitId = static_cast<quint8>(mBuffer[6]);
		quint8 functionCode = static_cast<quint8>(mBuffer[7]);
		if ((functionCode & 0x80) == 0) {
			switch (functionCode) {
			case ReadHoldingRegisters:
			case ReadInputRegisters:
				if (mBuffer.size() > 8) {
					quint8 payloadSize = static_cast<quint8>(mBuffer[8]);
					int i0 = 9;
					int i1 = i0 + payloadSize;
					if (i1 == length) {
						QVector<quint16> values;
						for (; i0 < i1; i0 += 2)
							values.append(toUInt16(mBuffer, i0));
						setFinished(transactionId, values);
						break;
					}
				}
				setFinished(transactionId, ModbusReply::ParseError);
				break;
			case WriteSingleRegister:
				if (mBuffer.size() > 11) {
					// quint16 startReg = toUInt16(mBuffer, 8);
					quint16 value = toUInt16(mBuffer, 10);
					QVector<quint16> values;
					values.append(value);
					setFinished(transactionId, values);
					break;
				}
				setFinished(transactionId, ModbusReply::ParseError);
				break;
			case WriteMultipleRegisters:
				if (mBuffer.size() == 12) {
					// quint16 startReg = toUInt16(mBuffer, 8);
					// quint16 regCount = toUInt16(mBuffer, 10);
					setFinished(transactionId, QVector<quint16>());
					break;
				}
				setFinished(transactionId, ModbusReply::ParseError);
				break;
			}
		} else {
			if (mBuffer.size() > 8) {
				quint8 error = static_cast<quint8>(mBuffer[8]);
				setFinished(transactionId, error);
			} else {
				setFinished(transactionId, ModbusReply::ParseError);
			}
		}
		mBuffer.remove(0, length);
	}
}

void ModbusTcpClient::onReplyDestroyed()
{
	Reply *reply = static_cast<Reply *>(sender());
	mPendingReplies.remove(reply->transactionId());
}

void ModbusTcpClient::onSocketErrorReceived(QAbstractSocket::SocketError error)
{
	Q_UNUSED(error)
	foreach (Reply *reply, mPendingReplies)
		reply->setResult(ModbusReply::TcpError);
	mPendingReplies.clear();
	emit disconnected();
}

ModbusReply *ModbusTcpClient::sendFrame(const QByteArray &frame)
{
	Reply *reply = new Reply(mTransactionId, mTimeout, this);
	mPendingReplies[mTransactionId] = reply;
	mSocket->write(frame);
	connect(reply, SIGNAL(destroyed()), this, SLOT(onReplyDestroyed()));
	return reply;
}

ModbusTcpClient::Reply *ModbusTcpClient::popReply(quint16 transactionId)
{
	QHash<quint16, Reply *>::Iterator it = mPendingReplies.find(transactionId);
	if (it == mPendingReplies.end())
		return 0;
	Reply *reply = it.value();
	mPendingReplies.erase(it);
	disconnect(reply);
	return reply;
}

ModbusReply *ModbusTcpClient::readRegisters(FunctionCode function, quint8 unitId, quint16 startReg,
											quint16 count)
{
	QByteArray frame = createFrame(function, unitId, 4);
	frame.append(static_cast<char>(msb(startReg)));
	frame.append(static_cast<char>(lsb(startReg)));
	frame.append(static_cast<char>(msb(count)));
	frame.append(static_cast<char>(lsb(count)));
	return sendFrame(frame);
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

void ModbusTcpClient::setFinished(quint16 transactionId, const QVector<quint16> &values)
{
	Reply *reply = popReply(transactionId);
	if (reply != 0)
		reply->setResult(values);
}

void ModbusTcpClient::setFinished(quint16 transactionId, int error)
{
	Reply *reply = popReply(transactionId);
	if (reply != 0)
		reply->setResult(static_cast<ModbusReply::ExceptionCode>(error));
}

ModbusTcpClient::Reply::Reply(quint16 transactionId, int timeout, QObject *parent):
	ModbusReply(parent),
	mTimerId(startTimer(timeout)),
	mTransactionId(transactionId)
{
}

bool ModbusTcpClient::Reply::isFinished() const
{
	return mTimerId == 0;
}

void ModbusTcpClient::Reply::onFinished()
{
	Q_ASSERT(mTimerId > 0);
	killTimer(mTimerId);
	mTimerId = 0;
}

void ModbusTcpClient::Reply::timerEvent(QTimerEvent *event)
{
	Q_UNUSED(event)
	Q_ASSERT(mTimerId > 0);
	setResult(Timeout);
}

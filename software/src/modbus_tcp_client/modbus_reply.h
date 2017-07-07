#ifndef MODBUS_REPLY_H
#define MODBUS_REPLY_H

#include <QObject>
#include <QVector>
#include <QDebug>
#include <QTextStream>

class ModbusReply : public QObject
{
	Q_OBJECT
public:
	enum ExceptionCode
	{
		NoException							= 0,
		IllegalFunction						= 1,
		IllegalDataAddress					= 2,
		IllegalDataValue					= 3,
		SlaveDeviceFailure					= 4,
		Acknowledge							= 5,
		SlaveDeviceBusy						= 6,
		MemoryParityError					= 7,
		GatewayPathUnavailable				= 10,
		GatewayTargetDeviceFailedToRespond	= 11,
		UnsupportedFunction					= 251,
		CrcError							= 252,
		ParseError							= 253,
		Timeout								= 254,
		TcpError							= 255
	};

	Q_ENUMS(ExceptionCode)

	QVector<quint16> registers() const
	{
		return mRegisters;
	}

	ExceptionCode error() const
	{
		return mError;
	}

	virtual bool isFinished() const = 0;

	virtual QString toString() const;

signals:
	void finished();

protected:
	explicit ModbusReply(QObject *parent = 0);

	virtual void onFinished() = 0;

	void setResult(const QVector<quint16> &registers);

	void setResult(ExceptionCode error);

private:
	QVector<quint16> mRegisters;
	ExceptionCode mError;
};

inline QDebug &operator<<(QDebug &str, const ModbusReply &reply)
{
	return str << reply.toString();
}

inline QTextStream &operator<<(QTextStream &str, const ModbusReply &reply)
{
	return str << reply.toString();
}

#endif // MODBUS_REPLY_H

#ifndef MODBUS_CLIENT_H
#define MODBUS_CLIENT_H

#include <QObject>

class ModbusReply;

class ModbusClient : public QObject
{
	Q_OBJECT
public:
	explicit ModbusClient(QObject *parent = 0);

	virtual ModbusReply *readHoldingRegisters(quint8 unitId, quint16 startReg, quint16 count) = 0;

	virtual ModbusReply *readInputRegisters(quint8 unitId, quint16 startReg, quint16 count) = 0;

	virtual ModbusReply *writeSingleHoldingRegister(quint8 unitId, quint16 reg, quint16 value) = 0;

	virtual ModbusReply *writeMultipleHoldingRegisters(quint8 unitId, quint16 startReg,
													   const QVector<quint16> &values) = 0;

	virtual int timeout() const = 0;

	virtual void setTimeout(int t) = 0;
};

#endif // MODBUS_CLIENT_H

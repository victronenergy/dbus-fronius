#ifndef INVERTER_MODBUS_UPDATER_H
#define INVERTER_MODBUS_UPDATER_H

#include <QObject>

class Inverter;
class ModbusTcpClient;
class QTimer;

class InverterModbusUpdater: QObject
{
	Q_OBJECT
public:
	InverterModbusUpdater(Inverter *inverter, QObject *parent = 0);

private slots:
	void onReadCompleted(quint8 unitId, QList<quint16> values);

	void onWriteCompleted(quint8 unitId, quint16 startReg, quint16 regCount);

	void onError(quint8 functionCode, quint8 unitId, quint8 exception);

	void onPowerLimitRequested(double value);

	void onConnected();

	void onDisconnected();

	void onTimer();

private:
	enum ModbusState {
		ReadMaxPower,
		ReadPowerLimitScale,
		ReadPowerLimit,
		ReadCurrentPower,
		WritePowerLimit,
		Idle,
		Init = ReadMaxPower,
		Start = ReadPowerLimit
	};

	void startNextAction(ModbusState state);

	void startIdleTimer();

	static double getScaledValue(const QList<quint16> &values, int offset = 0);

	Inverter *mInverter;
	ModbusTcpClient *mModbusClient;
	ModbusState mCurrentState;
	double mPowerLimit;
	int mPowerLimitScale;
	bool mWritePowerLimitRequested;
	QTimer *mTimer;
};

#endif // INVERTER_MODBUS_UPDATER_H

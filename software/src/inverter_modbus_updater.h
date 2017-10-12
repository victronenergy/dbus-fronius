#ifndef INVERTER_MODBUS_UPDATER_H
#define INVERTER_MODBUS_UPDATER_H

#include <QObject>
#include <QAbstractSocket>

class Inverter;
class ModbusTcpClient;
class QTimer;

class InverterModbusUpdater: public QObject
{
	Q_OBJECT
public:
	InverterModbusUpdater(Inverter *inverter, QObject *parent = 0);

private slots:
	void onReadCompleted(quint8 unitId, QList<quint16> values);

	void onWriteCompleted(quint8 unitId, quint16 startReg, quint16 regCount);

	void onError(quint8 functionCode, quint8 unitId, quint8 exception);

	void onSocketError(QAbstractSocket::SocketError error);

	void onPowerLimitRequested(double value);

	void onConnected();

	void onDisconnected();

	void onTimer();

private:
	enum ModbusState {
		ReadModelType,
		ReadMaxPower,
		ReadPowerLimitScale,
		ReadPowerLimit,
		ReadCurrentPower,
		WritePowerLimit,
		Idle,
		Init = ReadModelType,
		Start = ReadPowerLimit
	};

	void startNextAction(ModbusState state, bool checkReInit = false);

	void startIdleTimer();

	void resetValues();

	void setModelType(int type);

	static double getScaledValue(const QList<quint16> &values, int offset, bool isSigned);

	Inverter *mInverter;
	ModbusTcpClient *mModbusClient;
	ModbusState mCurrentState;
	double mPowerLimit;
	int mPowerLimitScale;
	/// Modbus TCP mode on PV inverter disabled (0), float (1), or int+sf (2).
	/// We only support int+sf.
	int mModelType;
	int mInitCounter;
	bool mWritePowerLimitRequested;
	QTimer *mTimer;
};

#endif // INVERTER_MODBUS_UPDATER_H

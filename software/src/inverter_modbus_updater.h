#ifndef INVERTER_MODBUS_UPDATER_H
#define INVERTER_MODBUS_UPDATER_H

#include <QObject>
#include <QAbstractSocket>

class FroniusDataProcessor;
class Inverter;
class InverterSettings;
class ModbusReply;
class ModbusTcpClient;
class QTimer;

class InverterModbusUpdater: public QObject
{
	Q_OBJECT
public:
	explicit InverterModbusUpdater(Inverter *inverter, InverterSettings *settings,
								   QObject *parent = 0);

signals:
	void connectionLost();

	void inverterModelChanged();

private slots:
	void onReadCompleted();

	void onWriteCompleted();

	void onError(quint8 functionCode, quint8 unitId, quint8 exception);

	void onSocketError(QAbstractSocket::SocketError error);

	void onPowerLimitRequested(double value);

	void onConnected();

	void onDisconnected();

	void onTimer();

	void onPhaseChanged();

private:
	enum ModbusState {
		ReadPowerLimit,
		ReadPowerAndVoltage,
		WritePowerLimit,
		Idle,
		Init = ReadPowerLimitScale,
		Start = ReadPowerLimit
	};

	void connectModbusClient();

	void startNextAction(ModbusState state);

	void startIdleTimer();

	void resetValues();

	void setInverterState(int sunSpecState);

	void readHoldingRegisters(quint16 startRegister, quint16 count);

	void writeMultipleHoldingRegisters(quint16 startReg, const QVector<quint16> &values);

	bool handleModbusError(ModbusReply *reply);

	void handleError();

	Inverter *mInverter;
	InverterSettings *mSettings;
	ModbusTcpClient *mModbusClient;
	QTimer *mTimer;
	FroniusDataProcessor *mDataProcessor;
	ModbusState mCurrentState;
	double mPowerLimitPct;
	int mRetryCount;
	bool mWritePowerLimitRequested;
};

#endif // INVERTER_MODBUS_UPDATER_H

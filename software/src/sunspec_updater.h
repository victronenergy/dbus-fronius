#ifndef INVERTER_MODBUS_UPDATER_H
#define INVERTER_MODBUS_UPDATER_H

#include <QObject>
#include <QAbstractSocket>

class DataProcessor;
class Inverter;
class InverterSettings;
class ModbusReply;
class ModbusTcpClient;
class QTimer;

class SunspecUpdater: public QObject
{
	Q_OBJECT
public:
	explicit SunspecUpdater(Inverter *inverter, InverterSettings *settings, QObject *parent = 0);

signals:
	void connectionLost();

	void inverterModelChanged();

private slots:
	void onReadCompleted();

	void onWriteCompleted();

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
		Idle
	};

	void connectModbusClient();

	void startNextAction(ModbusState state);

	void startIdleTimer();

	void setInverterState(int sunSpecState);

	void readHoldingRegisters(quint16 startRegister, quint16 count);

	void writeMultipleHoldingRegisters(quint16 startReg, const QVector<quint16> &values);

	bool handleModbusError(ModbusReply *reply);

	void handleError();

	ModbusState getInitState() const;

	Inverter *mInverter;
	InverterSettings *mSettings;
	ModbusTcpClient *mModbusClient;
	QTimer *mTimer;
	DataProcessor *mDataProcessor;
	ModbusState mCurrentState;
	double mPowerLimitPct;
	int mRetryCount;
	bool mWritePowerLimitRequested;
};

#endif // INVERTER_MODBUS_UPDATER_H

#ifndef INVERTER_MODBUS_UPDATER_H
#define INVERTER_MODBUS_UPDATER_H

#include <QObject>
#include <QList>
#include <QAbstractSocket>
#include <QString>

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

	virtual ~SunspecUpdater();

	static bool hasConnectionTo(QString host, int id);

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

	enum OperatingState {
		SunspecOff = 1,
		SunspecSleeping = 2,
		SunspecStarting = 3,
		SunspecMppt = 4,
		SunspecThrottled = 5,
		SunspecShutdown = 6,
		SunspecFault = 7,
		SunspecStandby = 8
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

	void updateSplitPhase(double power, double energy);

	Inverter *mInverter;
	InverterSettings *mSettings;
	ModbusTcpClient *mModbusClient;
	QTimer *mTimer;
	DataProcessor *mDataProcessor;
	ModbusState mCurrentState;
	double mPowerLimitPct;
	int mRetryCount;
	bool mWritePowerLimitRequested;
	static QList<SunspecUpdater*> mUpdaters; // to keep track of inverters we have a connection with
};

#endif // INVERTER_MODBUS_UPDATER_H

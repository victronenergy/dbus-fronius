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
class BaseLimiter;

class SunspecUpdater: public QObject
{
	Q_OBJECT
public:
	explicit SunspecUpdater(BaseLimiter *limiter, Inverter *inverter, InverterSettings *settings, QObject *parent = 0);

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

	void onLimiterInitialised(bool success);

	void onEnableLimiterChanged();

	void onDisconnected();

	void onTimer();

	void onPowerLimitExpired();

	void onPhaseChanged();

protected:
	virtual void readPowerAndVoltage();

	virtual bool parsePowerAndVoltage(QVector<quint16> values);

	Inverter *inverter() { return mInverter; }

	InverterSettings *settings() { return mSettings; }

	DataProcessor *processor() { return mDataProcessor; }

	void readHoldingRegisters(quint16 startRegister, quint16 count);

	void updateSplitPhase(double power, double energy);

	void setInverterState(int sunSpecState);

private:
	enum ModbusState {
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

	bool writePowerLimit(double powerLimitPct);

	bool resetPowerLimit();

	void connectModbusClient();

	void startNextAction(ModbusState state);

	void startIdleTimer();

	bool handleModbusError(ModbusReply *reply);

	void handleError();

	Inverter *mInverter;
	InverterSettings *mSettings;
	ModbusTcpClient *mModbusClient;
	QTimer *mTimer;
	QTimer *mPowerLimitTimer;
	DataProcessor *mDataProcessor;
	ModbusState mCurrentState;
	double mPowerLimitPct;
	int mRetryCount;
	bool mWritePowerLimitRequested;
	static QList<SunspecUpdater*> mUpdaters; // to keep track of inverters we have a connection with
	BaseLimiter *mLimiter;
};

class FroniusSunspecUpdater : public SunspecUpdater
{
	Q_OBJECT
public:
	explicit FroniusSunspecUpdater(BaseLimiter *limiter, Inverter *inverter, InverterSettings *settings, QObject *parent = 0);
private:
	bool parsePowerAndVoltage(QVector<quint16> values) override;
};

class Sunspec2018Updater : public SunspecUpdater
{
	Q_OBJECT
public:
	explicit Sunspec2018Updater(BaseLimiter *limiter, Inverter *inverter, InverterSettings *settings, QObject *parent = 0);
private:
	void readPowerAndVoltage() override;

	bool parsePowerAndVoltage(QVector<quint16> values) override;
};

// Limiting functionality

class BaseLimiter : public QObject
{
	Q_OBJECT
public:
	explicit BaseLimiter(Inverter *parent);

	virtual void onConnected(ModbusTcpClient *client);

	virtual ModbusReply *writePowerLimit(double powerLimitPct) = 0;

	virtual ModbusReply *resetPowerLimit() = 0;

signals:
	void initialised(bool);

protected:
	Inverter *mInverter;
	ModbusTcpClient *mClient;
};

class SunspecLimiter : public BaseLimiter
{
	Q_OBJECT
public:
	explicit SunspecLimiter(Inverter *parent);

	void onConnected(ModbusTcpClient *client) override;

	ModbusReply *writePowerLimit(double powerLimitPct) override;

	ModbusReply *resetPowerLimit() override;
};

class Sunspec2018Limiter : public BaseLimiter
{
	Q_OBJECT
public:
	explicit Sunspec2018Limiter(Inverter *parent);

	void onConnected(ModbusTcpClient *client) override;

	ModbusReply *writePowerLimit(double powerLimitPct) override;

	ModbusReply *resetPowerLimit() override;
};

#endif // INVERTER_MODBUS_UPDATER_H

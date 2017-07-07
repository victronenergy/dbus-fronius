#ifndef APP_H
#define APP_H

#include <QCoreApplication>

class ModbusClient;

class App: public QCoreApplication
{
	Q_OBJECT
public:
	App(int &argc, char **argv);

	int parseOptions();

private slots:
	void onConnected();

	void onFinished();

private:
	ModbusClient *mClient;
	quint16 mRegister;
	quint8 mUnitId;
	quint16 mCount;
	bool mWrite;
	quint16 mValue;
};

#endif // APP_H

#ifndef APP_H
#define APP_H

#include <QCoreApplication>

class ModbusTcpClient;

class App: public QCoreApplication
{
	Q_OBJECT
public:
	App(int &argc, char **argv);

	int parseOptions();

private slots:
	void onConnected();

	void onReadCompleted(quint8 unitId, const QList<quint16> &values);

	void onWriteCompleted(quint8 unitId, quint16 address, quint16 value);

	void onErrorReceived(quint8 functionCode, quint8 unitId, quint8 exception);

private:
	ModbusTcpClient *mClient;
	quint16 mRegister;
	quint8 mUnitId;
	quint16 mCount;
	bool mWrite;
	quint16 mValue;
};

#endif // APP_H

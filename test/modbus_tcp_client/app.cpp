#include <QTextStream>
#include "modbus_tcp_client/modbus_rtu_client.h"
#include "modbus_tcp_client/modbus_tcp_client.h"
#include "app.h"
#include "arguments.h"

App::App(int &argc, char **argv):
	QCoreApplication(argc, argv)
{
}

int App::parseOptions()
{
	Arguments args;
	args.addArg("-s", "Server name");
	args.addArg("-p", "TCP port");
	args.addArg("-d", "Serial port");
	args.addArg("-r", "Register");
	args.addArg("-c", "Register count");
	args.addArg("-u", "Unit ID");
	args.addArg("-w", "Write to register");
	args.addArg("-o", "Timeout (ms)");
	args.addArg("-h", "Help");

	if (args.contains("h")) {
		args.help();
		return 0;
	}
	if (!args.contains("r")) {
		args.help();
		return 1;
	}
	QString server = args.value("s");
	if (server.isEmpty())
		server = "localhost";
	mRegister = static_cast<quint16>(args.value("r").toUInt());
	mUnitId = static_cast<quint8>(args.value("u").toUInt());
	mCount = static_cast<quint16>(qMax(1u, args.value("c").toUInt()));
	int timeout = args.value("o").toInt();
	if (timeout <= 0)
		timeout = 1000;
	QString writeValue = args.value("w");
	if (!writeValue.isEmpty()) {
		if (mCount > 1) {
			QTextStream out(stdout);
			out << "Writing multiple registers is not supported\n";
			return 2;
		}
		mValue = static_cast<quint16>(writeValue.toUInt());
		mWrite = true;
	}
	quint16 port = 502;
	if (args.contains("p"))
		port = static_cast<quint16>(args.value("p").toUInt());

	QString serialPort;
	if (args.contains("d"))
		serialPort = args.value("d");

	if (serialPort.isEmpty()) {
		ModbusTcpClient *client = new ModbusTcpClient(this);
		connect(client, SIGNAL(connected()), this, SLOT(onConnected()));
		client->connectToServer(server, port);
		mClient = client;
	} else {
		mClient = new ModbusRtuClient(serialPort, 9600, this);
		onConnected();
	}
	mClient->setTimeout(timeout);
	return 0;
}

void App::onConnected()
{
	ModbusReply *reply = 0;
	if (mWrite) {
		reply = mClient->writeSingleHoldingRegister(mUnitId, mRegister, mValue);
	} else {
		reply = mClient->readHoldingRegisters(mUnitId, mRegister, mCount);
	}
	connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
}

void App::onFinished()
{
	ModbusReply *reply = static_cast<ModbusReply *>(sender());
	reply->deleteLater();
	QTextStream out(stdout);
	out << *reply << endl;
	quit();
}

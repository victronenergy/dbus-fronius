#include <QTextStream>
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
	args.addArg("-r", "Register");
	args.addArg("-c", "Register count");
	args.addArg("-u", "Unit ID");
	args.addArg("-w", "Write to register");
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

	mClient = new ModbusTcpClient(this);
	mClient->connectToServer(server, port);
	connect(mClient, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(mClient, SIGNAL(readHoldingRegistersCompleted(quint8, QList<quint16>)),
			this, SLOT(onReadCompleted(quint8, QList<quint16>)));
	connect(mClient, SIGNAL(writeSingleHoldingRegisterCompleted(quint8, quint16, quint16)),
			this, SLOT(onWriteCompleted(quint8, quint16, quint16)));
	connect(mClient, SIGNAL(errorReceived(quint8, quint8, quint8)),
			this, SLOT(onErrorReceived(quint8, quint8, quint8)));
	return 0;
}

void App::onConnected()
{
	if (mWrite) {
		mClient->writeSingleHoldingRegister(mUnitId, mRegister, mValue);
	} else {
		mClient->readHoldingRegisters(mUnitId, mRegister, mCount);
	}
}

void App::onReadCompleted(quint8 unitId, const QList<quint16> &values)
{
	Q_UNUSED(unitId)
	QTextStream out(stdout);
	foreach (quint16 v, values) {
		out << v << ' ';
	}
	out << '\n';
	quit();
}

void App::onWriteCompleted(quint8 unitId, quint16 address, quint16 value)
{
	Q_UNUSED(unitId)
	Q_UNUSED(address)
	QTextStream out(stdout);
	out << value << '\n';
	quit();
}

void App::onErrorReceived(quint8 functionCode, quint8 unitId, quint8 exception)
{
	Q_UNUSED(unitId)
	QTextStream out(stdout);
	out << "Error: " << functionCode << ' ' << unitId << ' ' << exception << '\n';
	quit();
}

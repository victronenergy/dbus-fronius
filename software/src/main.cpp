#include <QCoreApplication>
#include <QsLog.h>
#include <QStringList>
#include <unistd.h>
#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>
#include "dbus_fronius.h"
#include "dbus_settings_bridge.h"
#include "version.h"

void initLogger(QsLogging::Level logLevel)
{
	QsLogging::Logger &logger = QsLogging::Logger::instance();
	QsLogging::DestinationPtr debugDestination(
			QsLogging::DestinationFactory::MakeDebugOutputDestination());
	logger.addDestination(debugDestination);
	logger.setIncludeTimestamp(false);

	QLOG_INFO() << "dbus_fronius" << "v"VERSION << "started" << "("REVISION")";
	QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
	QLOG_INFO() << "Built on" << __DATE__ << "at" << __TIME__;
	logger.setLoggingLevel(logLevel);
}

void initDBus(const QString &dbusAddress)
{
	VBusItems::setDBusAddress(dbusAddress);

	QLOG_INFO() << "Wait for local settings on DBus... ";
	VBusItem settings;
	settings.consume("com.victronenergy.settings", "/Settings/Vrmlogger/Url");
	for (;;) {
		QVariant reply = settings.getValue();
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
		if (reply.isValid())
			break;
		usleep(500000);
		QLOG_INFO() << "Waiting...";
	}
	QLOG_INFO() << "Local settings found";
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	initLogger(QsLogging::InfoLevel);

	bool expectVerbosity = false;
	bool expectDBusAddress = false;
	QString dbusAddress = "system";
	foreach (QString arg, a.arguments()) {
		if (expectVerbosity) {
			QsLogging::Logger &logger = QsLogging::Logger::instance();
			int logLevel = arg.toInt();
			if (logLevel < 0)
				logLevel = 0;
			if (logLevel >= QsLogging::FatalLevel)
				logLevel = static_cast<int>(QsLogging::FatalLevel);
			logger.setLoggingLevel(static_cast<QsLogging::Level>(logLevel));
			expectVerbosity = false;
		} else if (expectDBusAddress) {
			dbusAddress = arg;
			expectDBusAddress = false;
		}
		if (arg == "-h" || arg == "--help") {
			qDebug() << a.arguments().first();
			qDebug() << "\t-h, --help";
			qDebug() << "\t Show this message.";
			qDebug() << "\t-V, --version";
			qDebug() << "\t Show the application version.";
			qDebug() << "\t-d level, --debug level";
			qDebug() << "\t Set log level";
			qDebug() << "\t-b, --dbus";
			qDebug() << "\t dbus address or 'session' or 'system'";
			return 0;
		}
		if (arg == "-V" || arg == "--version") {
			qDebug() << VERSION << "("REVISION")";
			return 0;
		} else if (arg == "-d" || arg == "--debug") {
			expectVerbosity = true;
		} else if (arg == "-t" || arg == "--timestamp") {
			QsLogging::Logger &logger = QsLogging::Logger::instance();
			logger.setIncludeTimestamp(true);
		} else if (arg == "-b" || arg == "--dbus") {
			expectDBusAddress = true;
		}
	}

	initDBus(dbusAddress);

	DBusFronius test(&a);

	return a.exec();
}

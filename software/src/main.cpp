#include <QCoreApplication>
#include <QsLog.h>
#include <QStringList>
#include <QThread>
#include "dbus_test.h"
#include "settings.h"
#include "version.h"

void initLogger(QsLogging::Level logLevel)
{
	QsLogging::Logger &logger = QsLogging::Logger::instance();
	QsLogging::DestinationPtr debugDestination(
			QsLogging::DestinationFactory::MakeDebugOutputDestination() );
	logger.addDestination(debugDestination);

	QLOG_INFO() << "dbus_fronius" << "v"VERSION << "started" << "("REVISION")";
	QLOG_INFO() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
	QLOG_INFO() << "Built on" << __DATE__ << "at" << __TIME__;
	logger.setLoggingLevel(logLevel);
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

#if QT_NO_DEBUG
	initLogger(QsLogging::ErrorLevel);
#else
	initLogger(QsLogging::TraceLevel);
#endif

	foreach (QString arg, a.arguments()) {
		if (arg == "-h" || arg == "--help") {
			qDebug() << a.arguments().first();
			qDebug() << "\t-h, --help";
			qDebug() << "\t Show this message.";
			qDebug() << "\t-V, --version";
			qDebug() << "\t Show the application version.";
			qDebug() << "\t-v, --verbose";
			qDebug() << "\t Increase verbosity. This option may be used more than once.";
			return 0;
		}
		if (arg == "-V" || arg == "--version") {
			qDebug() << VERSION << "("REVISION")";
			return 0;
		}
		if (arg == "-v" || arg == "--verbose") {
			QsLogging::Logger &logger = QsLogging::Logger::instance();
			int logLevel = logger.loggingLevel();
			if (logLevel > 0) {
				logger.setLoggingLevel(static_cast<QsLogging::Level>(logLevel - 1));
			}
		}
	}

#if TARGET_ccgx
	// Wait for local settings to become available on the DBus
	QLOG_INFO() << "Wait for local setting on DBus...";
	VBusItem settings("com.victronenergy.settings", "/Settings", DBUS_CONNECTION);
	QVariant reply = settings.getValue();
	while (!reply.isValid()) {
		reply = settings.getValue();
		QThread.Sleep(2000);
	}
	QLOG_INFO() << "Local settings found!";
#endif

	DBusTest test(&a);

	return a.exec();
}

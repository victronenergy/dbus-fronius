#include <QCoreApplication>
#include <QDBusConnection>
#include <QsLog.h>
#include <QStringList>
#include <unistd.h>
#include <velib/qt/v_busitems.h>
#include "dbus_fronius.h"
#include "dbus_settings_bridge.h"
#include "version.h"

void initLogger(QsLogging::Level logLevel)
{
	QsLogging::Logger &logger = QsLogging::Logger::instance();
	QsLogging::DestinationPtr debugDestination(
			QsLogging::DestinationFactory::MakeDebugOutputDestination() );
	logger.addDestination(debugDestination);
	logger.setIncludeTimestamp(false);

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

#if TARGET_ccgx
	VBusItems::setConnectionType(QDBusConnection::SystemBus);
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
		} else if (arg == "-v" || arg == "--verbose") {
			QsLogging::Logger &logger = QsLogging::Logger::instance();
			int logLevel = logger.loggingLevel();
			if (logLevel > 0) {
				logger.setLoggingLevel(static_cast<QsLogging::Level>(logLevel - 1));
			}
		} else if (arg == "-t" || arg == "--timestamp") {
			QsLogging::Logger &logger = QsLogging::Logger::instance();
			logger.setIncludeTimestamp(true);
		}
	}

	QLOG_INFO() << "Wait for local setting on DBus...";
	for (;;) {
		if (DBusSettingsBridge::addDBusObjects())
			break;
		QLOG_INFO() << "Wait...";
		usleep(2000000);
	}
	QLOG_INFO() << "Local settings found!";

	DBusFronius test(&a);

	return a.exec();
}

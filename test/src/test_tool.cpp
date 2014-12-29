#include <QCoreApplication>
#include <QsLog.h>
#include "dbus_observer.h"
#include "dbus_settings.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QsLogging::Logger &logger = QsLogging::Logger::instance();
	QsLogging::DestinationPtr debugDestination(
			QsLogging::DestinationFactory::MakeDebugOutputDestination());
	logger.addDestination(debugDestination);
	logger.setIncludeTimestamp(false);
	logger.setLoggingLevel(QsLogging::TraceLevel);

	DBusObserver observer("com.victronenergy.pvinverter");
	DBusSettings settings;

	return a.exec();
}

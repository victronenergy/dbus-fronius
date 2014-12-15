#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include "dbus_test.h"
#include "froniussolar_api_test.h"
#include "dbus_inverter_guard.h"
#include "dbus_settings_guard.h"
#include "inverter_gateway.h"
#include "local_ip_address_generator.h"
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

	initLogger(QsLogging::TraceLevel);

#if TARGET_ccgx
	// Wait for local settings to become available on the DBus
	QLOG_INFO() << "Wait for local setting on DBus... ";
	VBusItem settings("com.victronenergy.settings", "/Settings", DBUS_CONNECTION);
	QVariant reply = settings.getValue();
	while (reply.isValid() == false) {
		reply = settings.getValue();
		usleep(2000000);
	}
	QLOG_INFO() << "Local settings found!";
#endif

//	Settings settings;
//	DBusSettingsGuard guard(&settings);

	DBusTest test(&a);

	return a.exec();
}

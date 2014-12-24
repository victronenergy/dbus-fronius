#include <QsLog.h>
#include <QTest>
#include "dbus_inverter_bridge_test.h"
#include "dbus_settings_bridge_test.h"
#include "fronius_solar_api_test.h"

int main(int argc, char *argv[])
{
	QsLogging::Logger &logger = QsLogging::Logger::instance();
	QsLogging::DestinationPtr debugDestination(
			QsLogging::DestinationFactory::MakeDebugOutputDestination() );
	logger.addDestination(debugDestination);
	logger.setLoggingLevel(QsLogging::WarnLevel);

	QCoreApplication app(argc, argv);
	DBusSettingsBridgeTest tc;
	QTest::qExec(&tc, argc, argv);
	DBusInverterBridgeTest bc;
	QTest::qExec(&bc, argc, argv);
	FroniusSolarApiTest fc;
	QTest::qExec(&fc, argc, argv);
}

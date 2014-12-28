#include <QCoreApplication>
#include <QsLog.h>
#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
	QsLogging::Logger &logger = QsLogging::Logger::instance();
	QsLogging::DestinationPtr debugDestination(
		QsLogging::DestinationFactory::MakeDebugOutputDestination());
	logger.addDestination(debugDestination);
	logger.setLoggingLevel(QsLogging::WarnLevel);

	QCoreApplication app(argc, argv);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

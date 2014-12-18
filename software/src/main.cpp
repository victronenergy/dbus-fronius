#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QsLog.h>
#include <QStringList>
#include <QThread>
#include <unistd.h>
#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>
#include "dbus_test.h"
#include "dbus_settings_guard.h"
#include "settings.h"
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

bool addSettings(const QString &group, const QString &name, QChar type,
				 const QDBusVariant &defaultValue) {
	QDBusConnection &connection = VBusItems::getConnection();
	QDBusMessage m = QDBusMessage::createMethodCall(
				"com.victronenergy.settings",
				"/Settings",
				"com.victronenergy.Settings",
				"AddSetting")
		<< group
		<< name
		<< QVariant::fromValue(defaultValue)
		<< QString(type)
		<< QVariant::fromValue(QDBusVariant(0))
		<< QVariant::fromValue(QDBusVariant(0));
	QDBusMessage reply = connection.call(m);
	return reply.type() == QDBusMessage::ReplyMessage;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	// This enables us to retrieve values from QT properties via the
	// QObject::property function.
	qRegisterMetaType<QList<QHostAddress> >();
	qRegisterMetaType<QHostAddress>();

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

	for (;;) {
		if (addSettings("Fronius", "AutoDetect", 'i', QDBusVariant(1)))
			break;
		usleep(2000000);
	}

	QLOG_INFO() << "Local settings found!";

	addSettings("Fronius", "IPAddresses", 's', QDBusVariant(""));
	addSettings("Fronius", "KnownIPAddresses", 's', QDBusVariant(""));
	addSettings("Fronius", "ScanProgress", 'i', QDBusVariant(0));

	DBusTest test(&a);

	return a.exec();
}

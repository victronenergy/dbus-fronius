#include <QCoreApplication>
#include <QStringList>
#include <unistd.h>
#include <velib/qt/ve_qitem.hpp>
#include <velib/qt/ve_qitems_dbus.hpp>
#include <velib/qt/ve_qitem_dbus_publisher.hpp>
#include "dbus_fronius.h"
#include "ve_service.h"
#include "logging.h"

void initDBus()
{
	// Wait for localsettings. We need this because later on we might need the call 'AddSetting'
	// on localsettings, which will cause problems if the settings are not there yet.
	VeQItem *item = VeQItems::getRoot()->itemGetOrCreate(
		"sub/com.victronenergy.settings/Settings/Vrmlogger/Url", true);
	item->getValue();

	qInfo() << "Wait for local settings on DBus... ";
	for (;;) {
		if (item->getState() == VeQItem::Synchronized)
			break;
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
		usleep(500000);
		qInfo() << "Waiting...";
	}
	qInfo() << "Local settings found";
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(VERSION);

	qInfo() << "dbus_fronius" << "v" VERSION << "started";
	qInfo() << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
	qInfo() << "Built on" << __DATE__ << "at" << __TIME__;

	bool expectDBusAddress = false;
	QString dbusAddress = "system";
	bool debug = false;
	foreach (QString arg, a.arguments()) {
		if (expectDBusAddress) {
			dbusAddress = arg;
			expectDBusAddress = false;
		}
		if (arg == "-h" || arg == "--help") {
			qInfo() << a.arguments().first();
			qInfo() << "\t-h, --help";
			qInfo() << "\t Show this message.";
			qInfo() << "\t-V, --version";
			qInfo() << "\t Show the application version.";
			qInfo() << "\t-d, --debug";
			qInfo() << "\t Enable debug logging";
			qInfo() << "\t-b, --dbus";
			qInfo() << "\t dbus address or 'session' or 'system'";
			return 0;
		}
		if (arg == "-V" || arg == "--version") {
			qDebug() << VERSION;
			return 0;
		} else if (arg == "-d" || arg == "--debug") {
			debug = true;
		} else if (arg == "-b" || arg == "--dbus") {
			expectDBusAddress = true;
		}
	}

	initLogging(debug);

	VeQItemDbusProducer producer(VeQItems::getRoot(), "sub", true, false);
	producer.setAutoCreateItems(false);
	producer.open(dbusAddress);

	VeProducer dbusExportProducer(VeQItems::getRoot(), "pub");
	VeQItemDbusPublisher publisher(dbusExportProducer.services());
	publisher.open(dbusAddress);

	initDBus();

	DBusFronius test(&a);

	return a.exec();
}

#include <QCoreApplication>
#include <QStringList>
#include <QLoggingCategory>
#include <QtLogging>
#include <unistd.h>
#include <veutil/qt/ve_qitem.hpp>
#include <veutil/qt/ve_qitems_dbus.hpp>
#include <veutil/qt/ve_qitem_exported_dbus_services.hpp>
#include "dbus_fronius.h"
#include "ve_service.h"

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

	QStringList args = a.arguments();
	a.setApplicationName(args.takeFirst());

	QString dbusAddress = "system";
	bool debug = false;

	while (!args.isEmpty()) {
		QString arg = args.takeFirst();

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
			if (!args.isEmpty())
				dbusAddress = args.takeFirst();
		}
	}

	QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, debug);
	qSetMessagePattern("%{type} %{message}");

	VeQItemDbusProducer producer(VeQItems::getRoot(), "sub", true, false);
	producer.setAutoCreateItems(false);
	producer.open(dbusAddress);

	VeProducer dbusExportProducer(VeQItems::getRoot(), "pub");
	VeQItemExportedDbusServices publisher(dbusExportProducer.services());
	publisher.open(dbusAddress);

	initDBus();

	DBusFronius test(&a);

	return a.exec();
}

//#include <QCoreApplication>
//#include <QDBusConnection>
//#include <velib/qt/v_busitems.h>
//#include "dbus_settings.h"
//#include "dbus_client.h"

#include <QTest>
#include "dbus_settings_bridge_test.h"

QTEST_MAIN(DbusSettingsBridgeTest)

//int main(int argc, char *argv[])
//{
//	QCoreApplication a(argc, argv);

//	VBusItems::setConnectionType(QDBusConnection::SessionBus);

//	DBusSettings settings(&a);

//	static const QString DefaultServicePrefix = "com.victronenergy.pvinverter";
//	QString servicePrefix = argc < 2 ? DefaultServicePrefix : argv[1];
//	DBusClient c(servicePrefix, &a);

//	DBusClient cs("com.victronenergy.Settings", &a);

//	return a.exec();
//}

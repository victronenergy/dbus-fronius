#include <QCoreApplication>
#include <QDebug>
#include <QDBusConnection>
#include <velib/qt/v_busitem.h>
#include "dbus_test.h"
#include "froniussolar_api_test.h"
#include "dbus_inverter_guard.h"
#include "inverter_gateway.h"
#include "local_ip_address_generator.h"
#include "version.h"

int main(int argc, char *argv[])
{
	qDebug() << "App start\n";

	QCoreApplication a(argc, argv);

//	FroniusSolarApiTest test("ziontrain.no-ip.org", 8080, &a);
//	// FroniusSolarApiTest test("192.168.4.144", 8080, &a);
//	test.start();

	DBusTest test(&a);

//	DBusTest test(&a);

	return a.exec();
}

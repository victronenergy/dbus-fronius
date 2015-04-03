#ifndef DBUSINVERTERSCANBRIDGE_H
#define DBUSINVERTERSCANBRIDGE_H

#include "dbus_bridge.h"

class InverterGateway;
class QString;
class QVariant;

/*!
 * @brief Publishes device scan information to the D-bus.
 * This class is responsible for the com.victronenergy.fronius service which
 * contains information about the IP address scanning procedure.
 * This data will not be published to com.victronenergy.settings because they
 * may change often (especially ScanProgress), so they would cause the settings
 * file to be written too often. Furthermore, this is temporary data and should
 * therefore not be stored anyway.
 */
class DBusGatewayBridge : public DBusBridge
{
	Q_OBJECT
public:
	explicit DBusGatewayBridge(InverterGateway *gateway, QObject *parent = 0);

protected:
	virtual bool toDBus(const QString &path, QVariant &value);

	virtual bool fromDBus(const QString &path, QVariant &v);
};

#endif // DBUSINVERTERSCANBRIDGE_H

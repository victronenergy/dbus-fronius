#ifndef DBUS_CLIENT_H
#define DBUS_CLIENT_H

#include <QObject>
#include <QMap>

class VBusItem;

class DBusClient : QObject
{
	Q_OBJECT
public:
	DBusClient(const QString &servicePrefix, QObject *parent = 0);

private slots:
	void onServiceRegistered(const QString &service);

	void onServiceUnregistered(const QString &service);

	void onServiceOwnerChanged(const QString &name, const QString &oldOwner,
							   const QString &newOwner);

	void onValueChanged();

private:
	void scanObjects(const QString &service, const QString &path);

	QMultiMap<QString, VBusItem *> mItems;
	QString mServicePrefix;
};

#endif // DBUS_CLIENT_H

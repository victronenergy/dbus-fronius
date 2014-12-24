#ifndef DBUS_CLIENT_H
#define DBUS_CLIENT_H

#include <QObject>
#include <QMap>

class QDBusMessage;
class VBusItem;

class DBusClient : QObject
{
	Q_OBJECT
public:
	DBusClient(const QString &servicePrefix, QObject *parent = 0);

	QVariant getValue(const QString &service, const QString &path) const;

	QString getText(const QString &service, const QString &path) const;

	bool setValue(const QString &service, const QString &path,
				  const QVariant &value);

private slots:
	void onServiceRegistered(const QString &service);

	void onServiceUnregistered(const QString &service);

	void onServiceOwnerChanged(const QString &name, const QString &oldOwner,
							   const QString &newOwner);

	void onValueChanged();

	void onIntrospectSuccess(const QDBusMessage &reply);

	void onIntrospectFailure();

private:
	void scanObjects(const QString &service, const QString &path);

	QMultiMap<QString, VBusItem *> mItems;
	QString mServicePrefix;

	QList<QString> mPendingPaths;
	QString mIntrospectPath;
	QString mIntrospectService;
};

#endif // DBUS_CLIENT_H

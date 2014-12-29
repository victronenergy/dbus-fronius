#ifndef DBUS_CLIENT_H
#define DBUS_CLIENT_H

#include <QObject>
#include <QList>
#include <QString>

class DBusServiceObserver;

class DBusObserver : QObject
{
	Q_OBJECT
public:
	DBusObserver(const QString &servicePrefix, QObject *parent = 0);

	QVariant getValue(const QString &service, const QString &path) const;

	QString getText(const QString &service, const QString &path) const;

	bool setValue(const QString &service, const QString &path,
				  const QVariant &value);

private slots:
	void onServiceRegistered(const QString &service);

	void onServiceUnregistered(const QString &service);

	void onServiceOwnerChanged(const QString &name, const QString &oldOwner,
							   const QString &newOwner);

private:
	void scanObjects(const QString &service, const QString &path);

	DBusServiceObserver *findObserver(const QString &service) const;

	QList<DBusServiceObserver *> mObservers;
	QString mServicePrefix;
};

#endif // DBUS_CLIENT_H

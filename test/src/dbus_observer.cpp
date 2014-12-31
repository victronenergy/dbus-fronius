#include <QDBusConnectionInterface>
#include <QDBusConnection>
#include <QsLog.h>
#include <velib/qt/v_busitems.h>
#include "dbus_observer.h"
#include "dbus_service_observer.h"

DBusObserver::DBusObserver(const QString &servicePrefix, QObject *parent):
	QObject(parent),
	mServicePrefix(servicePrefix)
{
	QDBusConnectionInterface *ci = VBusItems::getConnection().interface();
	connect(ci, SIGNAL(serviceRegistered(QString)),
			this, SLOT(onServiceRegistered(QString)));
	connect(ci, SIGNAL(serviceUnregistered(QString)),
			this, SLOT(onServiceUnregistered(QString)));
	connect(ci, SIGNAL(serviceOwnerChanged(QString, QString, QString)),
			this, SLOT(onServiceOwnerChanged(QString, QString, QString)));
	QDBusReply<QStringList> reply = ci->registeredServiceNames();
	QStringList services = reply.value();
	for (QStringList::iterator it = services.begin();
		 it != services.end();
		 ++it)
	{
		onServiceRegistered(*it);
	}
}

QVariant DBusObserver::getValue(const QString &service, const QString &path) const
{
	DBusServiceObserver *observer = findObserver(service);
	return observer == 0 ? QVariant() : observer->getValue(path);
}

QString DBusObserver::getText(const QString &service, const QString &path) const
{
	DBusServiceObserver *observer = findObserver(service);
	return observer == 0 ? QString() : observer->getText(path);
}

bool DBusObserver::setValue(const QString &service, const QString &path,
						  const QVariant &value)
{
	DBusServiceObserver *observer = findObserver(service);
	return observer != 0 && observer->setValue(path, value);
}

bool DBusObserver::isInitialized(const QString &service) const
{
	DBusServiceObserver *observer = findObserver(service);
	return observer != 0 && observer->initialized();
}

void DBusObserver::onServiceRegistered(const QString &service)
{
	if (!service.startsWith(mServicePrefix))
		return;
	DBusServiceObserver *observer = findObserver(service);
	if (observer != 0)
		return;
	QLOG_INFO() << "New service:" << service;
	mObservers.append(new DBusServiceObserver(service));
}

void DBusObserver::onServiceUnregistered(const QString &service)
{
	DBusServiceObserver *observer = findObserver(service);
	if (observer == 0)
		return;
	mObservers.removeOne(observer);
	delete observer;
}

void DBusObserver::onServiceOwnerChanged(const QString &name,
									   const QString &oldOwner,
									   const QString &newOwner)
{
	if (oldOwner != newOwner) {
		if (!oldOwner.isEmpty())
			onServiceUnregistered(name);
		if (!newOwner.isEmpty())
			onServiceRegistered(name);
	}
}

DBusServiceObserver *DBusObserver::findObserver(const QString &service) const
{
	foreach (DBusServiceObserver *observer, mObservers) {
		if (observer->service() == service)
			return observer;
	}
	return 0;
}

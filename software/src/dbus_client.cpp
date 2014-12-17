#include <QDBusConnectionInterface>
#include <QDBusConnection>
#include <QDomDocument>
#include <velib/qt/v_busitem.h>
#include "dbus_client.h"

DBusClient::DBusClient(const QString &servicePrefix, QObject *parent):
	QObject(parent),
	mServicePrefix(servicePrefix)
{
	QDBusConnectionInterface *ci = QDBusConnection::sessionBus().interface();
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

void DBusClient::onServiceRegistered(const QString &service)
{
	if (service.startsWith(mServicePrefix) && !mItems.contains(service)) {
		qDebug() << "New service:" << service;
		scanObjects(service, "/");
	}
}

void DBusClient::onServiceUnregistered(const QString &service)
{
	if (service.startsWith(mServicePrefix) && mItems.contains(service)) {
		qDebug() << "Service removed:" << service;
		for (QMultiMap<QString, VBusItem *>::iterator it = mItems.find(service);
			 it != mItems.end() && it.key() == service;
			 ++it)
		{
			delete it.value();
		}
		mItems.remove(service);
	}
}

void DBusClient::onServiceOwnerChanged(const QString &name,
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

void DBusClient::onValueChanged()
{
	VBusItem *item = static_cast<VBusItem *>(sender());
	qDebug() << item->getBind() << item->getValue();
}

void DBusClient::scanObjects(const QString &service, const QString &path)
{
	QDBusMessage reply = QDBusConnection::sessionBus().call(
			QDBusMessage::createMethodCall(
					service,
					path,
					"org.freedesktop.DBus.Introspectable",
					"Introspect"));
	QVariant v = reply.arguments().first();
	QDomDocument doc;
	doc.setContent(v.toString());
	QDomElement root = doc.documentElement();
	bool isLeaf = true;
	for (QDomElement node = root.firstChildElement("node");
		 !node.isNull();
		 node = node.nextSiblingElement("node")) {
		QString subPath = path;
		if (!subPath.endsWith(('/'))) {
			subPath += '/';
		}
		subPath += node.attribute("name");
		isLeaf = false;
		scanObjects(service, subPath);
	}
	if (isLeaf) {
		VBusItem *item = new VBusItem(this);
		item->consume(service, path);
		mItems.insert(service, item);
		connect(item, SIGNAL(valueChanged()), this, SLOT(onValueChanged()));
		qDebug() << path << item->getValue();
	}
}

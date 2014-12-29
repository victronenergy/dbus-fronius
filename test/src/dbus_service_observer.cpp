#include <QDBusConnection>
#include <QDomDocument>
#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>
#include "dbus_service_observer.h"

DBusServiceObserver::DBusServiceObserver(const QString &service,
										 QObject *parent) :
	QObject(parent),
	mService(service)
{
	scanObjects("/");
}

const QString &DBusServiceObserver::service() const
{
	return mService;
}

QVariant DBusServiceObserver::getValue(const QString &path) const
{
	VBusItem *item = findItem(path);
	return item == 0 ? QVariant() : item->getValue();
}

QString DBusServiceObserver::getText(const QString &path) const
{
	VBusItem *item = findItem(path);
	return item == 0 ? QString() : item->getText();
}

bool DBusServiceObserver::setValue(const QString &path, const QVariant &value)
{
	VBusItem *item = findItem(path);
	return item != 0 && item->setValue(value) == 0;
}

void DBusServiceObserver::onIntrospectSuccess(const QDBusMessage &reply)
{
	QVariant v = reply.arguments().first();
	QDomDocument doc;
	doc.setContent(v.toString());
	QDomElement root = doc.documentElement();
	bool isLeaf = true;
	QString path = mIntrospectPath;
	for (QDomElement node = root.firstChildElement("node");
		 !node.isNull();
		 node = node.nextSiblingElement("node")) {
		QString subPath = path;
		if (!subPath.endsWith(('/'))) {
			subPath += '/';
		}
		subPath += node.attribute("name");
		isLeaf = false;
		mPendingPaths.append(subPath);
	}
	if (isLeaf) {
		QLOG_TRACE() << "New item: " << path;
		VBusItem *item = new VBusItem(this);
		mItems.append(item);
		connect(item, SIGNAL(valueChanged()), this, SLOT(onValueChanged()));
		item->consume(mService, path);
		// Force value retrieval
		item->getValue();
	}
	if (!mPendingPaths.isEmpty()) {
		scanObjects(mPendingPaths.first());
		mPendingPaths.removeFirst();
	}
}

void DBusServiceObserver::onIntrospectFailure(const QDBusError &error,
											  const QDBusMessage &reply)
{
	QLOG_ERROR() << "Error while scanning DBus"
				 << error.message()
				 << reply.errorMessage();
}

void DBusServiceObserver::onValueChanged()
{
	VBusItem *item = static_cast<VBusItem *>(sender());
	QLOG_INFO() << item->getBind() << item->getValue();
}

void DBusServiceObserver::scanObjects(const QString &path)
{
	mIntrospectPath = path;
	QDBusMessage introspect	= QDBusMessage::createMethodCall(
		mService,
		path,
		"org.freedesktop.DBus.Introspectable",
		"Introspect");
	QDBusConnection &cnx = VBusItems::getConnection();
	cnx.callWithCallback(introspect, this,
						 SLOT(onIntrospectSuccess(QDBusMessage)),
						 SLOT(onIntrospectFailure(QDBusError, QDBusMessage)));
}

VBusItem *DBusServiceObserver::findItem(const QString &path) const
{
	QString url = mService + path;
	foreach (VBusItem *item, mItems) {
		if (item->getBind() == url)
			return item;
	}
	return 0;
}
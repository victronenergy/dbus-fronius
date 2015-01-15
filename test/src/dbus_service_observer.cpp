#include <QDBusConnection>
#include <QDomDocument>
#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>
#include "dbus_service_observer.h"

DBusServiceObserver::DBusServiceObserver(const QString &service,
										 QObject *parent) :
	QObject(parent),
	mService(service),
	mInitialized(false)
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

bool DBusServiceObserver::initialized() const
{
	return mInitialized;
}

bool DBusServiceObserver::logChangedPaths() const
{
	return mLogChangedPaths;
}

void DBusServiceObserver::setLogChangedPaths(bool l)
{
	mLogChangedPaths = l;
}

const QStringList &DBusServiceObserver::changedPaths() const
{
	return mChangedPaths;
}

void DBusServiceObserver::resetChangedPaths()
{
	mChangedPaths.clear();
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
		ItemWrapper w;
		w.item = new VBusItem(this);
		w.initialized = false;
		mItems.append(w);
		connect(w.item, SIGNAL(valueChanged()), this, SLOT(onValueChanged()));
		w.item->consume(mService, path);
		// Force value retrieval
		w.item->getValue();
	}
	scanNext();
}

void DBusServiceObserver::onIntrospectFailure(const QDBusError &error,
											  const QDBusMessage &reply)
{
	QLOG_ERROR() << "Error while scanning DBus"
				 << error.message()
				 << reply.errorMessage();
	scanNext();
}

void DBusServiceObserver::onValueChanged()
{
	VBusItem *item = static_cast<VBusItem *>(sender());
	QLOG_INFO() << item->getBind() << item->getValue();
	for (QList<ItemWrapper>::iterator it = mItems.begin(); it != mItems.end(); ++it) {
		if (it->item == item) {
			if (!it->initialized)
			{
				it->initialized = true;
				checkInitialized();
			}
			break;
		}
	}
	if (mLogChangedPaths) {
		const QString &bind = item->getBind();
		int i = bind.indexOf('/');
		mChangedPaths.append(bind.mid(i));
	}
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

void DBusServiceObserver::checkInitialized()
{
	if (mInitialized || !mIntrospectPath.isEmpty())
		return;
	foreach (ItemWrapper w, mItems) {
		if (!w.initialized)
			return;
	}
	mInitialized = true;
	emit initializedChanged();
}

void DBusServiceObserver::scanNext()
{
	if (mPendingPaths.isEmpty()) {
		mIntrospectPath.clear();
		checkInitialized();
	} else {
		scanObjects(mPendingPaths.first());
		mPendingPaths.removeFirst();
	}
}

VBusItem *DBusServiceObserver::findItem(const QString &path) const
{
	QString url = mService + path;
	foreach (ItemWrapper w, mItems) {
		if (w.item->getBind() == url)
			return w.item;
	}
	return 0;
}

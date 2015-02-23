#include <QDBusVariant>
#include <QDir>
#include <QDomDocument>
#include <QsLog.h>
#include <QTimer>
#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>
#include "v_bus_node.h"
#include "dbus_settings.h"
#include "dbus_settings_adaptor.h"

DBusSettings::DBusSettings(QObject *parent):
	QObject(parent),
	mCnx(VBusItems::getConnection()),
	mTrackChanges(true),
	mTimer(0)
{
	mSettingsPath = QDir::home().path() + "/.dbus_settings.xml";
	new DBusSettingsAdaptor(this);
	mCnx.registerObject("/Settings", this);
	mCnx.registerService("com.victronenergy.settings");
}

DBusSettings::~DBusSettings()
{
	mCnx.unregisterService("com.victronenergy.settings");
}

void DBusSettings::loadSettings()
{
	QFile file(mSettingsPath);
	file.open(QFile::ReadOnly | QFile::Text);
	QDomDocument doc;
	doc.setContent(&file);
	QDomElement root = doc.documentElement();
	for (QDomElement node = root.firstChildElement("object");
		 !node.isNull();
		 node = node.nextSiblingElement("object")) {
		QString path = node.attribute("path");
		QString value = node.attribute("value");
		addSetting(path, QDBusVariant(value));
	}
}

void DBusSettings::storeSettings()
{
	QLOG_TRACE() << "Save settings";
	QDomDocument doc;
	QDomElement root = doc.createElement("objects");
	doc.appendChild(root);
	if (!mRoot.isNull()) {
		QStringList paths = mRoot->enumeratePaths();
		foreach (QString path, paths) {
			QVariant v = mRoot->findItem(path)->getValue();
			QDomElement obj	= doc.createElement("object");
			obj.setAttribute("path", path);
			obj.setAttribute("value", v.toString());
			root.appendChild(obj);
		}
	}
	QFile file(mSettingsPath);
	file.open(QFile::WriteOnly | QFile::Text);
	QTextStream str(&file);
	doc.save(str, 4);
}

bool DBusSettings::trackChanges() const
{
	return mTrackChanges;
}

void DBusSettings::setTrackChanges(bool b)
{
	mTrackChanges = b;
	if (!mTrackChanges)
		mChangedPaths.clear();
}

bool DBusSettings::autoSave() const
{
	return mTimer != 0;
}

void DBusSettings::setAutoSave(bool s)
{
	if (s && mTimer == 0) {
		mTimer = new QTimer(this);
		mTimer->setInterval(5000);
		connect(mTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
	} else if (!s && mTimer != 0) {
		delete mTimer;
		mTimer = 0;
	}
}

QVariant DBusSettings::getValue(const QString &path) const
{
	if (!mRoot.isNull()) {
		VBusItem *item = mRoot->findItem(path);
		if (item != 0)
			return item->getValue();
	}
	return QVariant();
}

bool DBusSettings::setValue(const QString &path, const QVariant &value)
{
	if (!mRoot.isNull()) {
		VBusItem *item = mRoot->findItem(path);
		if (item != 0) {
			return item->setValue(value) == 0;
		}
	}
	return false;
}

void DBusSettings::resetChangedPaths()
{
	mChangedPaths.clear();
}

const QStringList &DBusSettings::changedPaths() const
{
	return mChangedPaths;
}

void DBusSettings::addSetting(const QString &path,
							  const QDBusVariant &defaultValue)
{
	if (mRoot.isNull())
		mRoot = new VBusNode(mCnx, "/", this);
	VBusItem *vbi = new VBusItem(this);
	if (mRoot->findItem(path) == 0) {
		QLOG_INFO() << "New Path:" << path;
		mRoot->addChild(path, vbi);
		scheduleSave();
		emit ObjectPathsChanged();
	}
	connect(vbi, SIGNAL(valueChanged()), this, SLOT(onVBusItemChanged()));
	vbi->produce(mCnx, path, "", defaultValue.variant());
}

int DBusSettings::AddSetting(const QString &group, const QString &name,
							 const QDBusVariant &defaultValue,
							 const QString &itemType,
							 const QDBusVariant &minimum,
							 const QDBusVariant &maximum)
{
	Q_UNUSED(itemType);
	Q_UNUSED(minimum);
	Q_UNUSED(maximum);

	QString path = QString("/Settings/%1/%2").arg(group, name);
	addSetting(path, defaultValue);

	return 0;
}

void DBusSettings::onVBusItemChanged()
{
	if (mTrackChanges)
		mChangedPaths.append(mRoot->findPath(static_cast<VBusItem *>(sender())));
	scheduleSave();
}

void DBusSettings::onTimer()
{
	storeSettings();
	mTimer->stop();
}

void DBusSettings::scheduleSave()
{
	if (mTimer == 0 || mTimer->isActive())
		return;
	mTimer->start();
}

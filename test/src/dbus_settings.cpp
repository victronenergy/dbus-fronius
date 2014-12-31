#include <QDBusVariant>
#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include <velib/qt/v_busitems.h>
#include "dbus_settings.h"
#include "dbus_settings_adaptor.h"
#include "v_bus_node.h"

DBusSettings::DBusSettings(QObject *parent):
	QObject(parent),
	mCnx(VBusItems::getConnection())
{
	new DBusSettingsAdaptor(this);
	mCnx.registerObject("/Settings", this);
	mCnx.registerService("com.victronenergy.settings");
}

DBusSettings::~DBusSettings()
{
	mCnx.unregisterService("com.victronenergy.settings");
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
		if (item != 0)
			return item->setValue(value) == 0;
	}
	return false;
}

void DBusSettings::resetChangedPaths()
{
	mChangedPaths.clear();
}

const QList<QString> &DBusSettings::changedPaths() const
{
	return mChangedPaths;
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

	VBusItem *vbi = new VBusItem(this);
	QString path = QString("/Settings/%1/%2").arg(group, name);
	if (mRoot.isNull())
		mRoot = new VBusNode(mCnx, "/", this);
	if (mRoot->findItem(path) == 0) {
		QLOG_INFO() << "New Path:" << path;
		mRoot->addChild(path, vbi);
		emit ObjectPathsChanged();
	}
	connect(vbi, SIGNAL(valueChanged()), this, SLOT(onVBusItemChanged()));
	vbi->produce(mCnx, path, "", defaultValue.variant());

	return 0;
}

void DBusSettings::onVBusItemChanged()
{
	mChangedPaths.append(mRoot->findPath(static_cast<VBusItem *>(sender())));
}

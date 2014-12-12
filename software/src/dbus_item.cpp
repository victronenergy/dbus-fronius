#include "dbus_item.h"
#include <cassert>
#include <QDebug>

DBusItem::DBusItem(QString slotName, QString unit, const char *updateSignal, QObject *parent):
	QDBusAbstractAdaptor(parent),
	mPropertyName(slotName),
	mUnit(unit)
{
	assert(parent != 0);
	connect(parent, updateSignal, this, SIGNAL(PropertiesChanged(QVariantMap)));
}

QDBusVariant DBusItem::GetValue() const
{
	qDebug() << "DBusItem::GetValue" << mPropertyName.toAscii().data();
	QVariant v = parent()->property(mPropertyName.toAscii().data());
	qDebug() << "DBusItem::GetValue" << v;
	return QDBusVariant(v);
}

QString DBusItem::GetText() const
{
	return mUnit;
}

QString DBusItem::GetDescription() const
{
	return "n/a"; /// @todo EV ???
}

//void DBusItem::onPropertyChanged()
//{
//	qDebug() << "DBusItem::onPropertyChanged";
//	QVariantMap m;
//	m[mPropertyName] = parent()->property(mPropertyName.toAscii().data());
//	emit PropertiesChanged(m);
//}

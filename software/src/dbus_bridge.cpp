#include <QsLog.h>
#include <velib/qt/v_busitem.h>
#include "dbus_bridge.h"
#include "v_bus_node.h"

DBusBridge::DBusBridge(QObject *parent) :
	QObject(parent)
{
}

void DBusBridge::produce(QDBusConnection &connection, QObject *src,
						const char *property, const QString &path,
						const QString &unit, int precision)
{
	VBusItem *vbi = new VBusItem(this);
	QVariant value = src->property(property);
	connectItem(vbi, src, property, path);
	vbi->produce(connection, path, "?", value, unit, precision);
	addVBusNodes(connection, path, vbi);
}

void DBusBridge::produce(QDBusConnection &connection,
						 const QString &path,
						 const QVariant &value,
						 const QString &unit,
						 int precision)
{
	VBusItem *vbi = new VBusItem(this);
	connectItem(vbi, 0, 0, path);
	vbi->produce(connection, path, "", value, unit, precision);
	addVBusNodes(connection, path, vbi);
}

void DBusBridge::consume(QDBusConnection &connection, const QString &service,
						QObject *src, const char *property, const QString &path)
{
	VBusItem *vbi = new VBusItem(this);
	connectItem(vbi, src, property, path);
	vbi->consume(connection, service, path);
	vbi->getValue(); // force value retrieval
}

void DBusBridge::toDBus(const QString &, QVariant &)
{
}

void DBusBridge::fromDBus(const QString &, QVariant &)
{
}

void DBusBridge::onPropertyChanged()
{
	QObject *src = sender();
	int signalIndex = senderSignalIndex();
	const QMetaObject *mo = src->metaObject();
	for (int i=0; i<mo->propertyCount(); ++i) {
		QMetaProperty mp = mo->property(i);
		if (mp.hasNotifySignal() && mp.notifySignalIndex() == signalIndex) {
			foreach (BusItemBridge bib, mBusItems) {
				if (bib.src == src &&
						strcmp(bib.property.name(), mp.name()) == 0) {
					QVariant value = src->property(mp.name());
					toDBus(bib.path, value);
					bib.item->setValue(value);
					break;
				}
			}
		}
	}
}

void DBusBridge::onVBusItemChanged()
{
	foreach (BusItemBridge bib, mBusItems) {
		if (bib.item == sender()) {
			QVariant value = bib.item->getValue();
			fromDBus(bib.path, value);
			bib.src->setProperty(bib.property.name(), value);
			break;
		}
	}
}

void DBusBridge::connectItem(VBusItem *busItem, QObject *src,
							const char *property, const QString &path)
{
	BusItemBridge bib;
	bib.item = busItem;
	bib.src = src;
	bib.path = path;
	if (src == 0) {
		if (property != 0) {
			QLOG_ERROR() << "Property specified (" << property
						 << "), but source omitted in DBusBridge"
						 << ". Path was" << path;
		}
	} else {
		if (property == 0) {
			QLOG_ERROR() << "Source specified, but property omitted in DBusBridge"
						 << ". Path was" << path;
		} else {
			const QMetaObject *mo = src->metaObject();
			int i = mo->indexOfProperty(property);
			if (i == -1) {
				QLOG_ERROR() << "DBusBridge could not find property" << property
							 << "Path was" << path;
			} else {
				QMetaProperty mp = mo->property(i);
				if (mp.hasNotifySignal()) {
					QMetaMethod signal = mp.notifySignal();
					QMetaMethod slot = metaObject()->method(
								metaObject()->indexOfSlot("onPropertyChanged()"));
					connect(src, signal, this, slot);
				}
				bib.property = mp;
			}
		}
	}
	mBusItems.push_back(bib);
	connect(busItem, SIGNAL(valueChanged()), this, SLOT(onVBusItemChanged()));
}

void DBusBridge::addVBusNodes(QDBusConnection &connection, const QString &path,
							 VBusItem *vbi)
{
	if (mServiceRoot.isNull())
		mServiceRoot = new VBusNode(connection, "/", this);
	mServiceRoot->addChild(path, vbi);
}

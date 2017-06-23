#include <qnumeric.h>
#include <QsLog.h>
#include "ve_service.h"

VeService::VeService(VeQItem *root, QObject *parent):
	QObject(parent),
	mRoot(root)
{
	Q_ASSERT(mRoot != 0);
	connect(mRoot, SIGNAL(destroyed()), this, SLOT(onRootDestroyed()));
}

VeService::~VeService()
{
	if (mRoot != 0)
		removeFromItems(mRoot);
	mRoot->itemDelete();
	mRoot = 0;
}

VeQItem *VeService::createItem(const QString &path)
{
	VeQItem *item = mRoot->itemGetOrCreate(path);
	static_cast<VeProducerItem *>(item)->setService(this);
	return item;
}

void VeService::registerService()
{
	QLOG_INFO() << "Registering service" << mRoot->id();
	mRoot->produceValue(QVariant(), VeQItem::Synchronized);
}

void VeService::produceDouble(VeQItem *item, double value, int precision, const QString &unit)
{
	QVariant v = qIsFinite(value) ? QVariant(value) : QVariant();
	QString text;
	if (v.isValid()) {
		if (precision >= 0) {
			text.setNum(value, 'f', precision);
		} else {
			text.setNum(value);
		}
		if (!text.isEmpty())
			text += unit;
	}
	produceValue(item, v, text);
}

void VeService::produceValue(VeQItem *item, const QVariant &value)
{
	produceValue(item, value, value.toString());
}

void VeService::produceValue(VeQItem *item, const QVariant &value, const QString &text)
{
	item->produceValue(value);
	item->produceText(text);
}

double VeService::getDouble(VeQItem *item) const
{
	return getDouble(item, qQNaN());
}

double VeService::getDouble(VeQItem *item, double defaultValue) const
{
	QVariant v = item->getValue();
	return v.isValid() ? v.toDouble() : defaultValue;
}

void VeService::onRootDestroyed()
{
	mRoot = 0;
}

void VeService::removeFromItems(VeQItem *item)
{
	static_cast<VeProducerItem *>(item)->removeService(this);
	for (int i=0;;++i) {
		VeQItem *child = item->itemChild(i);
		if (child == 0)
			return;
		removeFromItems(child);
	}
}

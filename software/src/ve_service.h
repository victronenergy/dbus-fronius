#ifndef VESERVICE_H
#define VESERVICE_H

#include <QObject>
#include <veutil/qt/ve_qitem.hpp>

class VeService: public QObject
{
	Q_OBJECT
public:
	virtual ~VeService();

	virtual int handleSetValue(VeQItem *item, const QVariant &variant)
	{
		Q_UNUSED(item)
		Q_UNUSED(variant)
		return 1;
	}

	VeQItem *root() const
	{
		return mRoot;
	}

protected:
	VeService(VeQItem *root, QObject *parent = 0);

	VeQItem *createItem(const QString &path);

	void registerService();

	void produceDouble(VeQItem *item, double value, int precision, const QString &unit);

	void produceValue(VeQItem *item, const QVariant &value);

	void produceValue(VeQItem *item, const QVariant &value, const QString &text);

	double getDouble(VeQItem *item) const;

	double getDouble(VeQItem *item, double defaultValue) const;

private slots:
	void onRootDestroyed();

private:
	void removeFromItems(VeQItem *parent);

	VeQItem *mRoot;
};

class VeProducerItem : public VeQItem
{
	Q_OBJECT
public:
	explicit VeProducerItem(VeQItemProducer *producer, QObject *parent = 0):
		VeQItem(producer, parent),
		mService(0)
	{
	}

	void setService(VeService *r)
	{
		Q_ASSERT(mService == 0 || mService == r);
		mService = r;
	}

	void removeService(VeService *r)
	{
		if (mService == r)
			mService = 0;
	}

	int setValue(QVariant const &value) override
	{
		if (mService == 0)
			return VeQItem::setValue(value);
		return mService->handleSetValue(this, value);
	}

private:
	VeService *mService;
};

class VeProducer: public VeQItemProducer
{
	Q_OBJECT
public:
	VeProducer(VeQItem *root, QString id, QObject *parent = 0):
		VeQItemProducer(root, id, parent)
	{
	}

	VeQItem *createItem() override
	{
		return new VeProducerItem(this);
	}
};

#endif // VESERVICE_H

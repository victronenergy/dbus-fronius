#ifndef VEQITEMCONSUMER_H
#define VEQITEMCONSUMER_H

#include <QObject>
#include <QVariant>
#include <QList>
#include <QVariantMap>
#ifdef QT_DBUS_LIB
#include <velib/qt/ve_qitems_dbus.hpp>
#endif // QT_DBUS_LIB

Q_DECLARE_METATYPE (QList<QVariantMap>);

class VeQItem;

class VeQItemConsumer : public QObject
{
	Q_OBJECT

public:
	VeQItem *root() const
	{
		return mRoot;
	}

	bool addSetting(const QString &path, const QVariant &defaultValue, const QVariant &minValue,
					const QVariant &maxValue, bool silent);

	static bool addSetting(VeQItem *root, const QString &path, const QVariant &defaultValue,
						   const QVariant &minValue, const QVariant &maxValue, bool silent);

	int getDeviceInstance(const QString &uniqueId, const QString &deviceClass, int defaultValue);

protected:
	VeQItemConsumer(VeQItem *root, QObject *parent = 0);

	void setRoot(VeQItem *root);

	VeQItem *connectItem(const QString &path, const char *signal, bool forceSync = false);

	VeQItem *connectItem(const QString &path, double defaultValue, double minValue,
						 double maxValue, const char *signal, bool silent = false);

	VeQItem *connectItem(const QString &path, int defaultValue, const char *signal,
						 bool silent = false);

	VeQItem *connectItem(const QString &path, const QString &defaultValue, const char *signal,
						 bool silent = false);

	void connectItem(VeQItem *&item, const QString &path, const char *signal, bool forceSync = false);

	static double getDouble(VeQItem *item);

private slots:
	void onValueChanged(VeQItem *item, QVariant value);

private:
	VeQItem *mRoot;
};


#ifdef QT_DBUS_LIB
/**
 * A VeQItemDbus that forces the item's state back to Synchronized after a call to setValue.
 * This is a workaround for the problem that a call to setValue will not trigger a SetValue on the
 * D-Bus if the previous setValue used the same value and there was no PropertiesChange from
 * the D-Bus object after the first setValue.
 */
class NoStorageQItem: public VeQItemDbus
{
	Q_OBJECT
public:
	NoStorageQItem(VeQItemDbusProducer *producer):
		VeQItemDbus(producer),
		mForceSync(false)
	{
	}

	virtual int setValue(QVariant const &value)
	{
		int i = VeQItemDbus::setValue(value);
		if (!mForceSync || i != 0)
			return i;
		VeQItemDbus::setState(VeQItem::Synchronized);
		return 0;
	}

	void setForceSync(bool f)
	{
		mForceSync = f;
	}

private:
	bool mForceSync;
};

/**
 * A VeQItemDbusProducer that creates NoStorageQItem instead of VeQItemDbus
 */
class NostorageQItemProducer: public VeQItemDbusProducer
{
	Q_OBJECT
public:
	NostorageQItemProducer(VeQItem *root, QString id, bool findVictronServices = true,
						   bool bulkInitOfNewService = true, QObject *parent = 0):
		VeQItemDbusProducer(root, id, findVictronServices, bulkInitOfNewService, parent)
	{
	}

	virtual VeQItem *createItem()
	{
		return new NoStorageQItem(this);
	}
};
#endif // QT_DBUS_LIB

#endif // VEQITEMCONSUMER_H

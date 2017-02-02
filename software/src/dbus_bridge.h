#ifndef DBUS_BRIDGE_H
#define DBUS_BRIDGE_H

#include <QList>
#include <QMetaProperty>
#include <QObject>
#include <QPointer>
#include <QString>
#include <velib/qt/ve_qitem.hpp>

class BridgeItem;
class QDBusConnection;
class QTimer;
class VeQItem;

/*!
 * \brief Synchronizes QT properties with DBus objects.
 * This class synchronizes properties defined by Q_PROPERTY with objects on the
 * DBus. Whenever the value of a QT property changes, the associated SetItem
 * function will be called on the DBus and the PropertiesChanged (DBus) signal
 * will be fired. DBusBridge will also update the QT property if the DBus object
 * changes.
 * This class assumes that the DBus object has the usual victron layout. So
 * each object should have the methods GetValue, SetValue, and GetText as well
 * as the PropertiesChanged signal.
 */
class DBusBridge : public QObject
{
	Q_OBJECT
public:
	DBusBridge(const QString &serviceName, bool isProducer, QObject *parent = 0);

	DBusBridge(VeQItem *serviceRoot, bool isProducer, QObject *parent = 0);

	~DBusBridge();

	void setUpdateInterval(int interval);

	/*!
	 * \brief Connects a QT property to a DBus object, and registers the object.
	 * Connects the QT property specified by `src` and `property` to the
	 * DBus object specified by `connection` and `path`.
	 * The DBus object will also be registered by this function.
	 * \param src The owner of the property
	 * \param property The name of the property.
	 * \param path The path to the DBus object
	 * \param unit The unit of the DBus value (will be used to construct the
	 * output of the GetText method together with the value of the property).
	 * \param precision The precision of the GetText result. Will only be used
	 * if the property has a floating point value.
	 */
	void produce(QObject *src, const char *property, const QString &path,
				 const QString &unit = QString(), int precision = -1, bool alwaysNotify = false);

	/*!
	 * \brief Pushes a constant value to the DBus, and registers the object.
	 * `value` will be pushed (SetValue) to the DBus object specified by
	 * `connection` and `path`. The value will not be adjusted later on, even
	 * if it is changed on the DBus. Since the caller of this function has
	 * full control of the variant passed to this function, it will not be
	 * passed through the toDBus function.
	 * The DBus object will also be registered by this function.
	 * \param path
	 * \param value
	 * \param unit
	 */
	void produce(const QString &path, const QVariant &value,
				 const QString &unit = QString(), int precision = -1);

	/*!
	 * \brief Connects a QT property to an existing DBus item.
	 * Connects the QT property specified by `src` and `property` to the
	 * DBus object specified by `connection`, `service`, and `path`.
	 * \param service
	 * \param src
	 * \param property
	 * \param path
	 */
	void consume(QObject *src, const char *property, const QString &path);

	void consume(QObject *src, const char *property,
				 const QVariant &defaultValue, const QString &path, bool silentSetting);

	void consume(QObject *src, const char *property, double defaultValue,
				 double minValue, double maxValue, const QString &path, bool silentSetting);

	VeQItem *service() const
	{
		return mServiceRoot.data();
	}

	QString serviceName() const
	{
		return mServiceRoot ? mServiceRoot->id() : QString();
	}

	void registerService();

	void updateIsInitialized();

	int updateValue(BridgeItem *prodItem, QVariant &value);

	bool alwaysNotify(BridgeItem *item);

	bool addSetting(const QString &path, const QVariant &defaultValue,
					const QVariant &minValue, const QVariant &maxValue, bool silent);

signals:
	void initialized();

protected:
	/*!
	 * \brief Allows conversion of values sent to DBus.
	 * This function will be called when a value is prepared to be sent
	 * over the DBus (`SetValue`), and allows you to change (convert) to value.
	 * The default implementation of this function is empty.
	 * \param path The path to the DBus object.
	 * \param v The value to be sent. On input this will be the value taken
	 * from the QT property.
	 * \retval If the implementation returns false, the value will not be sent
	 * to the DBus.
	 */
	virtual bool toDBus(const QString &path, QVariant &v);

	/*!
	 * \brief Allows conversion of values received from DBus.
	 * This function will be called before a value received from the
	 * DBus (`GetValue` of `PropertiesChanged`) in assigned to the associated
	 * property.
	 * The default implementation of this function is empty.
	 * \param path The path to the DBus object.
	 * \param v The value received from the DBus.
	 * \retval If the implementation returns false, the value from the DBus will
	 * not be written to the associated property.
	 */
	virtual bool fromDBus(const QString &path, QVariant &v);

	virtual QString toText(const QString &path, const QVariant &value, const QString &unit,
						   int precision);

private slots:
	void onPropertyChanged();

	void onVBusItemChanged(VeQItem *item);

	void onUpdateTimer();

private:
	struct BusItemBridge
	{
		VeQItem *item;
		QObject *src;
		QMetaProperty property;
		QString path;
		QString unit;
		int precision;
		bool busy;
		bool changed;
		bool alwaysNotify;
	};

	BusItemBridge &connectItem(VeQItem *item, QObject *src, const char *property,
							   const QString &path, const QString &unit, int precision,
							   bool publish, bool alwaysNotify);

	void publishValue(BusItemBridge &item);

	void publishValue(BusItemBridge &item, QVariant value);

	void setValue(BusItemBridge &bridge, QVariant &value);

	BusItemBridge *findBridge(VeQItem *item);

	QList<BusItemBridge> mBusItems;
	QPointer<VeQItem> mServiceRoot;
	QTimer *mUpdateTimer;
	bool mIsProducer;
	bool mIsInitialized;
};

class BridgeItem : public VeQItem
{
	Q_OBJECT
public:
	explicit BridgeItem(VeQItemProducer *producer, QObject *parent = 0):
		VeQItem(producer, parent),
		mBridge(0)
	{
	}

	void setBridge(DBusBridge *bridge)
	{
		mBridge = bridge;
	}

	virtual int setValue(QVariant const &value)
	{
		if (mBridge != 0) {
			QVariant v = value;
			return mBridge->updateValue(this, v);
		}
		return VeQItem::setValue(value);
	}

	virtual void produceValue(QVariant value, State state = Synchronized);

private:
	DBusBridge *mBridge;
};

class BridgeItemProducer: public VeQItemProducer
{
	Q_OBJECT
public:
	BridgeItemProducer(VeQItem *root, QString id, QObject *parent = 0):
		VeQItemProducer(root, id, parent)
	{
	}

	virtual VeQItem *createItem()
	{
		return new BridgeItem(this);
	}
};

#endif // DBUS_BRIDGE_H

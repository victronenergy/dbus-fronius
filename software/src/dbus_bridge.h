#ifndef DBUS_BRIDGE_H
#define DBUS_BRIDGE_H

#include <QList>
#include <QMetaProperty>
#include <QObject>
#include <QPointer>
#include <QString>

class QDBusConnection;
class QDBusVariant;
class VBusItem;
class VBusNode;

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
	explicit DBusBridge(QObject *parent);

	/*!
	 * \brief Connects a QT property to a DBus object, and registers the object.
	 * Connects the QT property specified by `src` and `property` to the
	 * DBus object specified by `connection` and `path`.
	 * The DBus object will also be registered by this function.
	 * \param connection
	 * \param src The owner of the property
	 * \param property The name of the property.
	 * \param path The path to the DBus object
	 * \param unit The unit of the DBus value (will be used to construct the
	 * output of the GetText method together with the value of the property).
	 * \param precision The precision of the GetText result. Will only be used
	 * if the property has a floating point value.
	 */
	void produce(QDBusConnection &connection, QObject *src,
				 const char *property, const QString &path,
				 const QString &unit = QString(), int precision = -1);

	/*!
	 * \brief Pushes a constant value to the DBus, and registers the object.
	 * `value` will be pushed (SetValue) to the DBus object specified by
	 * `connection` and `path`. The value will not be adjusted later on, even
	 * if it is changed on the DBus. Since the caller of this function has
	 * full control of the variant passed to this function, it will not be
	 * passed through the toDBus function.
	 * The DBus object will also be registered by this function.
	 * \param connection
	 * \param path
	 * \param value
	 * \param unit
	 */
	void produce(QDBusConnection &connection, const QString &path,
				 const QVariant &value, const QString &unit = QString(),
				 int precision = -1);

	/*!
	 * \brief Connects a QT property to an existing DBus item.
	 * Connects the QT property specified by `src` and `property` to the
	 * DBus object specified by `connection`, `service`, and `path`.
	 * \param connection
	 * \param service
	 * \param src
	 * \param property
	 * \param path
	 */
	void consume(QDBusConnection &connection, const QString &service,
				 QObject *src, const char *property, const QString &path);

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

	static bool addDBusObject(const QString &group, const QString &name,
							  QChar type, const QDBusVariant &defaultValue,
							  const QDBusVariant &minValue,
							  const QDBusVariant &maxValue);

	static bool addDBusObject(const QString &group, const QString &name,
							  QChar type, const QDBusVariant &defaultValue);

	static bool addDBusDouble(const QString &group, const QString &name,
							  double defaultValue, double minValue,
							  double maxValue);

private slots:
	void onPropertyChanged();

	void onVBusItemChanged();

private:
	void connectItem(VBusItem *item, QObject *src, const char *property,
					 const QString &path);

	void addVBusNodes(QDBusConnection &connection, const QString &path,
					  VBusItem *vbi);

	struct BusItemBridge
	{
		VBusItem *item;
		QObject *src;
		QMetaProperty property;
		QString path;
		bool initialized;
	};
	QList<BusItemBridge> mBusItems;
	QPointer<VBusNode> mServiceRoot;
};

#endif // DBUS_BRIDGE_H

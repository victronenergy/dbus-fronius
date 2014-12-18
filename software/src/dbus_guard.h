#ifndef DBUS_GUARD_H
#define DBUS_GUARD_H

#include <QList>
#include <QObject>
#include <QString>
#include <QMetaProperty>

class QDBusConnection;
class VBusItem;
class VBusNode;

class DBusGuard : public QObject
{
	Q_OBJECT
public:
	explicit DBusGuard(QObject *parent);

protected:
	virtual void toDBus(const QString &path, QVariant &v);

	virtual void fromDBus(const QString &path, QVariant &v);

protected:
	void produce(QDBusConnection &connection, QObject *src,
				 const char *property, const QString &path,
				 const QString &unit = QString(), int precision = -1);

	void produce(QDBusConnection &connection, const QString &path,
				 const QVariant &value, const QString &unit = QString());

	void consume(QDBusConnection &connection, const QString &service,
				 QObject *src, const char *property, const QString &path);

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
	};
	QList<BusItemBridge> mBusItems;
	VBusNode *mServiceRoot;
};

#endif // DBUS_GUARD_H

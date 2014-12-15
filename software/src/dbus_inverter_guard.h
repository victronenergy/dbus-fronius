#ifndef DBUS_INVERTER_GUARD_H
#define DBUS_INVERTER_GUARD_H

#include <QList>
#include <QObject>
#include <QMap>
#include <QString>
#include <QVariantMap>

class Inverter;
class PowerInfo;
class QDBusConnection;
class VBusItem;

class DBusInverterGuard : public QObject
{
	Q_OBJECT
public:
	explicit DBusInverterGuard(Inverter *inverter);

private slots:
	void onPropertyChanged(const QString &property);

	void onVBusItemChanged();

private:
	void addBusItems(QDBusConnection &connection, PowerInfo *pi,
					 const QString &path);

	void addBusItem(QDBusConnection &connection, QObject *src,
					const QString &path, const QString &property,
					const QString &unit);

	void addConstBusItem(QDBusConnection &connection, const QString &path,
					const QVariant &value);

	void addNodes(QDBusConnection &connection, const QString &path);

	void updateNodes(const QString &path);

	QVariantMap createNodeMap(const QString &path) const;

	static QString fixServiceNameFragment(const QString &s);

	static QString getParentPath(const QString &s);

	static bool isChildPath(const QString &p0, const QString &p1);

	const Inverter *mInverter;
	struct BusItemBridge
	{
		VBusItem *item;
		QObject *src;
		QString property;
		QString path;
	};
	QList<BusItemBridge> mBusItems;
	QMap<QString, VBusItem *> mNodes;
};

#endif // DBUS_INVERTER_GUARD_H

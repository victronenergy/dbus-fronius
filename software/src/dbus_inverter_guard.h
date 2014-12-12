#ifndef DBUS_INVERTER_GUARD_H
#define DBUS_INVERTER_GUARD_H

#include <QList>
#include <QObject>
#include <QString>

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
	void onPropertyChanged(QString property);

private:
	void addBusItems(QDBusConnection &connection, PowerInfo *pi, QString path);

	void addBusItem(QDBusConnection &connection, QObject *src, QString path,
					QString property, QString unit);

	static QString fixServiceNameFragment(const QString &s);

	const Inverter *mInverter;
	struct BusItemBridge
	{
		VBusItem *item;
		QObject *src;
		QString property;
	};
	QList<BusItemBridge> mBusItems;
};

#endif // DBUS_INVERTER_GUARD_H

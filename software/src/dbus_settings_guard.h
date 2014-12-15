#ifndef DBUS_SETTINGS_GUARD_H
#define DBUS_SETTINGS_GUARD_H

#include <QObject>
#include <QMap>

class Settings;
class VBusItem;

class DBusSettingsGuard : public QObject
{
	Q_OBJECT
public:
	DBusSettingsGuard(Settings *settings, QObject *parent = 0);

	~DBusSettingsGuard();

private slots:
	void onPropertyChanged(const QString &property);

	void onVBusItemChanged();

private:
	void addBusItem(const QString &path, const QString &property);

	Settings *mSettings;
	struct ItemPropertyBridge
	{
		QString property;
		VBusItem *item;
	};
	QList<ItemPropertyBridge> mVBusItems;
};

#endif // DBUS_SETTINGS_GUARD_H

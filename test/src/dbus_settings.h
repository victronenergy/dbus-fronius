#ifndef DBUS_SETTINGS_H
#define DBUS_SETTINGS_H

#include <QDBusConnection>
#include <QList>
#include <QObject>
#include <QPointer>

class QString;
class QDBusVariant;
class VBusNode;
class VBusItem;

class DBusSettings : public QObject
{
	Q_OBJECT
public:
	DBusSettings(QObject *parent = 0);

	QVariant getValue(const QString &path) const;

	bool setValue(const QString &path, const QVariant &value);

	void resetChangedPaths();

	const QList<QString> &changedPaths() const;

public slots:
	int AddSetting(const QString &group, const QString &name,
				   const QDBusVariant &defaultValue, const QString &itemType,
				   const QDBusVariant &minimum, const QDBusVariant &maximum);

signals:
	void ObjectPathsChanged();

private slots:
	void onVBusItemChanged();

private:
	QDBusConnection mCnx;
	QPointer<VBusNode> mRoot;
	QList<QString> mChangedPaths;
};

#endif // DBUS_SETTINGS_H

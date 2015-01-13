#ifndef DBUS_SETTINGS_H
#define DBUS_SETTINGS_H

#include <QDBusConnection>
#include <QList>
#include <QObject>
#include <QPointer>

class QDBusVariant;
class QString;
class QTimer;
class VBusNode;
class VBusItem;

class DBusSettings : public QObject
{
	Q_OBJECT
public:
	DBusSettings(QObject *parent = 0);

	~DBusSettings();

	void loadSettings();

	void storeSettings();

	bool trackChanges() const;

	void setTrackChanges(bool b);

	bool autoSave() const;

	void setAutoSave(bool s);

	QVariant getValue(const QString &path) const;

	bool setValue(const QString &path, const QVariant &value);

	void resetChangedPaths();

	const QList<QString> &changedPaths() const;

	void addSetting(const QString &path, const QDBusVariant &defaultValue);

public slots:
	int AddSetting(const QString &group, const QString &name,
				   const QDBusVariant &defaultValue, const QString &itemType,
				   const QDBusVariant &minimum, const QDBusVariant &maximum);

signals:
	void ObjectPathsChanged();

private slots:
	void onVBusItemChanged();

	void onTimer();

private:
	void scheduleSave();

	QDBusConnection mCnx;
	QPointer<VBusNode> mRoot;
	QList<QString> mChangedPaths;
	QString mSettingsPath;
	bool mTrackChanges;
	QTimer *mTimer;
};

#endif // DBUS_SETTINGS_H

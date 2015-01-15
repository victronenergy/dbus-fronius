#ifndef DBUSSERVICEOBSERVER_H
#define DBUSSERVICEOBSERVER_H

#include <QObject>
#include <QString>
#include <QStringList>

class QDBusError;
class QDBusMessage;
class VBusItem;

class DBusServiceObserver : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool initialized READ initialized NOTIFY initializedChanged)
public:
	explicit DBusServiceObserver(const QString &service, QObject *parent = 0);

	const QString &service() const;

	QVariant getValue(const QString &path) const;

	QString getText(const QString &path) const;

	bool setValue(const QString &path, const QVariant &value);

	bool initialized() const;

	bool logChangedPaths() const;

	void setLogChangedPaths(bool l);

	const QStringList &changedPaths() const;

	void resetChangedPaths();

signals:
	void initializedChanged();

private slots:
	void onIntrospectSuccess(const QDBusMessage &reply);

	void onIntrospectFailure(const QDBusError &error, const QDBusMessage &reply);

	void onValueChanged();

private:
	void scanObjects(const QString &path);

	void checkInitialized();

	void scanNext();

	VBusItem *findItem(const QString &path) const;

	QString mService;
	struct ItemWrapper {
		VBusItem *item;
		bool initialized;
	};
	QList<ItemWrapper> mItems;
	QStringList mPendingPaths;
	QString mIntrospectPath;
	bool mInitialized;
	QStringList mChangedPaths;
	bool mLogChangedPaths;
};

#endif // DBUSSERVICEOBSERVER_H

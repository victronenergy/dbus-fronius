#ifndef DBUSSERVICEOBSERVER_H
#define DBUSSERVICEOBSERVER_H

#include <QObject>

class QDBusError;
class QDBusMessage;
class VBusItem;

class DBusServiceObserver : public QObject
{
	Q_OBJECT
public:
	explicit DBusServiceObserver(const QString &service, QObject *parent = 0);

	const QString &service() const;

	QVariant getValue(const QString &path) const;

	QString getText(const QString &path) const;

	bool setValue(const QString &path, const QVariant &value);

private slots:
	void onIntrospectSuccess(const QDBusMessage &reply);

	void onIntrospectFailure(const QDBusError &error, const QDBusMessage &reply);

	void onValueChanged();

private:
	void scanObjects(const QString &path);

	VBusItem *findItem(const QString &path) const;

	QString mService;
	QList<VBusItem *> mItems;
	QList<QString> mPendingPaths;
	QString mIntrospectPath;
};

#endif // DBUSSERVICEOBSERVER_H

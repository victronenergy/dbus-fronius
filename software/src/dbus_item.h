#ifndef DBUSITEM_H
#define DBUSITEM_H

#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QVariant>
#include <QString>

class DBusItem : public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "com.victronenergy.DBusItem")
public:
	DBusItem(QString slotName, QString unit, const char *updateSignal, QObject *parent);

signals:
	void PropertiesChanged(const QVariantMap &changes);

public slots:
	QDBusVariant GetValue() const;

	QString GetText() const;

	QString GetDescription() const;

//private slots:
//	void onPropertyChanged();

private:
	QString mPropertyName;
	QString mUnit;
};

#endif // DBUSITEM_H

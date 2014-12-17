#ifndef V_BUS_NODE_H
#define V_BUS_NODE_H

#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QDBusConnection>
#include <QMap>

class VBusItem;

class VBusNode : public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "com.victronenergy.BusNode")
public:
	VBusNode(QDBusConnection &connection, const QString &path,
			 QObject *parent);

	void addChild(const QString &path, VBusItem *item);

	VBusItem *findItem(const QString &path) const;

	VBusNode *findNode(const QString &path) const;

public slots:
	QDBusVariant GetValue();

signals:
	void PropertiesChanged(const QVariantMap &changes);

private slots:
	void onItemDeleted();

	void onNodeDeleted();

private:
	void addToMap(const QString &prefix, QVariantMap &map);

	void addChild(const QString &nodePath, const QString &subPath,
				  VBusItem *item);

	QMap<QString, VBusItem *> mLeafs;
	QMap<QString, VBusNode *> mNodes;
	QDBusConnection mConnection;
};

#endif // V_BUS_NODE_H

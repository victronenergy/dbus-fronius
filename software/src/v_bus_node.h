#ifndef V_BUS_NODE_H
#define V_BUS_NODE_H

#include <QDBusAbstractAdaptor>
#include <QDBusVariant>
#include <QDBusConnection>
#include <QMap>

class VBusItem;

/*!
 * @brief A D-Bus item that creates a map of its substructure when its
 * GetValue function is called.
 * Usage: create a `VBusNode` as root object for a D-Bus service. Register all
 * `VBusItem`s, which are part of the service, by calling the `addChild`
 * function. The root object will create additional `VbusNode` objects for all
 * nodes it the D-Bus structure. The GetValue function of each node will return
 * a map with all paths and value of its substructure.
 * @note A `VBusNode` will delete itself (by calling `deleteLater` when all its
 * children (`VBusNode`s and `VBusItem`s are deleted). If you delete `VBusItem`s
 * dynamically, use a `QPointer` to store the pointer to the root node and check
 * whether it is null before use.
 */
class VBusNode : public QDBusAbstractAdaptor
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "com.victronenergy.BusNode")
public:
	VBusNode(QDBusConnection &connection, const QString &path, QObject *parent);

	/*!
	 * @brief A a `VBusItem` to the node structure. All nodes between this
	 * node and the item will be automatically created whenever necessary.
	 * @param path The relative path from this node to the item. If the node is
	 * the root node, the path is absolute.
	 * @param item The `VBusItem` to be added to the node structure.
	 */
	void addChild(const QString &path, VBusItem *item);

	/*!
	 * @brief Retrieves the `VBusItem` located at the specified path.
	 * Returns 0 if there's no item at the given location.
	 */
	VBusItem *findItem(const QString &path) const;

	/*!
	 * @brief Retrieves the `VBusNode` located at the specified path.
	 * Returns 0 if there's no item at the given location.
	 */
	VBusNode *findNode(const QString &path) const;

	/*!
	 * @brief Returns thet path of the given `VBusItem`.
	 * Returns an empty string if the item could not be found.
	 */
	QString findPath(const VBusItem *item) const;

	/*!
	 * @brief Returns thet path of the given `VBusNode`.
	 * Returns an empty string if the item could not be found.
	 */
	QString findPath(const VBusNode *item) const;

	/*!
	 * @brief lists the paths of all `VBusItem`s in the substructure. Node that
	 * those paths are relative to this node, or absolute if this node is the
	 * root.
	 */
	QStringList enumeratePaths() const;

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

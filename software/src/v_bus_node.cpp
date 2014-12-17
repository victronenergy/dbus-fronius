#include <velib/qt/v_busitem.h>
#include "v_bus_node.h"

static QString combine(QString lhs, QString rhs) {
	if (!lhs.isEmpty() && !lhs.endsWith('/'))
		lhs.append('/');
	lhs.append(rhs);
	return lhs;
}

VBusNode::VBusNode(QDBusConnection &connection, const QString &path,
				   QObject *parent) :
	QDBusAbstractAdaptor(new QObject(parent)),
	mConnection(connection)
{
	// We need some trickery to get the adaptor running. A QDBusAbstractAdaptor
	// consideres its parent to be the data source. It is the responsibility
	// of the subclass (this class) to retrieve the data from that class.
	// This class does not do that, but the QT DBus framework still expects a
	// unique parent for each adapter.
	connection.registerObject(path, this->parent());
}

void VBusNode::addChild(const QString &path, VBusItem *item)
{
	addChild("/", path, item);
}

VBusItem *VBusNode::findItem(const QString &path) const
{
	int i = path.indexOf('/', 1);
	QString id = path.mid(1, i);
	if (i == -1) {
		QMap<QString, VBusItem *>::const_iterator it = mLeafs.find(id);
		return it == mLeafs.end() ? 0 : it.value();
	} else {
		QMap<QString, VBusNode *>::const_iterator it = mNodes.find(id);
		return it == mNodes.end() ? 0 : it.value()->findItem(path.mid(i));
	}
}

VBusNode *VBusNode::findNode(const QString &path) const
{
	int i = path.indexOf('/', 1);
	QString id = path.mid(1, i);
	if (i == -1) {
		QMap<QString, VBusNode *>::const_iterator it = mNodes.find(id);
		return it == mNodes.end() ? 0 : it.value();
	} else {
		QMap<QString, VBusNode *>::const_iterator it = mNodes.find(id);
		return it == mNodes.end() ? 0 : it.value()->findNode(path.mid(i));
	}
}

QDBusVariant VBusNode::GetValue()
{
	QVariantMap result;
	addToMap(QString(), result);
	return QDBusVariant(result);
}

void VBusNode::onItemDeleted() {
	for (QMap<QString, VBusItem *>::iterator it = mLeafs.begin();
		 it != mLeafs.end();
		 ++it) {
		if (it.value() == sender())
		{
			mLeafs.remove(it.key());
			if (mLeafs.empty() && mNodes.empty()) {
				deleteLater();
			}
		}
	}
}

void VBusNode::onNodeDeleted()
{
	for (QMap<QString, VBusNode *>::iterator it = mNodes.begin();
		 it != mNodes.end();
		 ++it) {
		if (it.value() == sender())
		{
			mNodes.remove(it.key());
			if (mLeafs.empty() && mNodes.empty()) {
				deleteLater();
			}
		}
	}
}

void VBusNode::addToMap(const QString &prefix, QVariantMap &map)
{
	for (QMap<QString, VBusItem *>::iterator it = mLeafs.begin();
		 it != mLeafs.end();
		 ++it) {
		map[combine(prefix, it.key())] = it.value()->getValue();
	}
	for (QMap<QString, VBusNode *>::iterator it = mNodes.begin();
		 it != mNodes.end();
		 ++it) {
		it.value()->addToMap(combine(prefix, it.key()), map);
	}
}

void VBusNode::addChild(const QString &nodePath, const QString &subPath,
						VBusItem *item)
{
	Q_ASSERT(nodePath.startsWith('/'));
	Q_ASSERT(subPath.startsWith('/'));
	int i = subPath.indexOf('/', 1);
	if (i == -1) {
		connect(item, SIGNAL(destroyed()), this, SLOT(onItemDeleted()));
		mLeafs.insert(subPath.mid(1), item);
	} else {
		QString id = subPath.mid(1, i - 1);
		QString newNodePath = combine(nodePath, id);
		QMap<QString, VBusNode *>::iterator it = mNodes.find(id);
		if (it == mNodes.end()) {
			VBusNode *node = new VBusNode(mConnection, newNodePath, parent());
			it = mNodes.insert(id, node);
		}
		it.value()->addChild(newNodePath, subPath.mid(i), item);
	}
	// emit PropertiesChanged(createMap());
}

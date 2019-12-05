#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QMetaType>
#include <QStringList>
#include <ve_qitem_consumer.h>

class VeQItem;

class Settings : public VeQItemConsumer
{
	Q_OBJECT
public:
	explicit Settings(VeQItem *root, QObject *parent = 0);

	int portNumber() const;

	QList<QHostAddress> ipAddresses() const;

	void setIpAddresses(const QList<QHostAddress> &addresses);

	QList<QHostAddress> knownIpAddresses() const;

	void setKnownIpAddresses(const QList<QHostAddress> &addresses);

	bool autoScan() const;

	/*!
	 * Returns the list with D-Bus object names for each registered inverter.
	 * The names in the list are based on the device type and the serial
	 * (unique ID).
	 */
	QStringList inverterIds() const;

	/*!
	 * Registers an inverter.
	 * If the inverter is unknown, it will be added to `inverterIds`.
	 * @param deviceType The device type as specified by Fronius.
	 * @param unique The inverter serial (called unique ID by Fronius).
	 */
	int registerInverter(const QString &uniqueId);

	/*!
	 * Creates the D-Bus settings object name for the specified inverter.
	 * @param deviceType The device type as specified by Fronius.
	 * @param unique The inverter serial (called unique ID by Fronius).
	 */
	static QString createInverterId(const QString &deviceSerial);

signals:
	void portNumberChanged();

	void ipAddressesChanged();

private slots:
	void onInverterdIdsChanged();

private:
	QList<QHostAddress> toAdressList(const QString &s) const;

	QString fromAddressList(const QList<QHostAddress> &a);

	VeQItem *mPortNumber;
	VeQItem *mIpAddresses;
	VeQItem *mKnownIpAddresses;
	VeQItem *mInverterIds;
	VeQItem *mAutoScan;
	QStringList mInverterIdCache;
};

#endif // SETTINGS_H

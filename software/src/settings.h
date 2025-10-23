#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QPair>
#include <QMetaType>
#include <ve_qitem_consumer.h>

class VeQItem;

class Settings : public VeQItemConsumer
{
	Q_OBJECT
public:
	explicit Settings(VeQItem *root, QObject *parent = 0);

	int portNumber() const;

	QList<QHostAddress> ipAddresses() const;

	QList<QHostAddress> knownIpAddresses() const;

	void setKnownIpAddresses(const QList<QHostAddress> &addresses);

	QList<QPair<int,quint8>> modbusAlternates() const;

	bool autoScan() const;

	bool idBySerial() const;

	/*!
	 * Registers an inverter.
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

private:
	QList<QHostAddress> toAdressList(const QString &s) const;

	QString fromAddressList(const QList<QHostAddress> &a);

	VeQItem *mPortNumber;
	VeQItem *mModbusAlternates;
	VeQItem *mIpAddresses;
	VeQItem *mKnownIpAddresses;
	VeQItem *mAutoScan;
	VeQItem *mIdBySerial;
};

#endif // SETTINGS_H

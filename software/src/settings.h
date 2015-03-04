#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QMetaType>
#include <QStringList>

class Settings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(int portNumber READ portNumber WRITE setPortNumber NOTIFY portNumberChanged)
	Q_PROPERTY(QList<QHostAddress> ipAddresses READ ipAddresses WRITE setIpAddresses NOTIFY ipAddressesChanged)
	Q_PROPERTY(QList<QHostAddress> knownIpAddresses READ knownIpAddresses WRITE setKnownIpAddresses NOTIFY knownIpAddressesChanged)
	Q_PROPERTY(QStringList inverterIds READ inverterIds WRITE setInverterIds NOTIFY inverterIdsChanged)
public:
	explicit Settings(QObject *parent = 0);

	int portNumber() const;

	void setPortNumber(int p);

	const QList<QHostAddress> &ipAddresses() const;

	void setIpAddresses(const QList<QHostAddress> &addresses);

	const QList<QHostAddress> &knownIpAddresses() const;

	void setKnownIpAddresses(const QList<QHostAddress> &addresses);

	/*!
	 * Returns the list with D-Bus object names for each registered inverter.
	 * The names in the list are based on the device type and the serial
	 * (unique ID).
	 */
	const QStringList &inverterIds() const;

	void setInverterIds(const QStringList &serial);

	/*!
	 * Registers an inverter.
	 * If the inverter is unknown, it will be added to `inverterIds`.
	 * @param deviceType The device type as specified by Fronius.
	 * @param unique The inverter serial (called unique ID by Fronius).
	 */
	void registerInverter(int deviceType, const QString &uniqueId);

	int getDeviceInstance(int deviceType, const QString &uniqueId) const;

	/*!
	 * Creates the D-Bus settings object name for the specified inverter.
	 * @param deviceType The device type as specified by Fronius.
	 * @param unique The inverter serial (called unique ID by Fronius).
	 */
	static QString createInverterId(int deviceType, const QString &deviceSerial);

signals:
	void portNumberChanged();

	void ipAddressesChanged();

	void knownIpAddressesChanged();

	void inverterIdsChanged();

private:
	int mPortNumber;
	QList<QHostAddress> mIpAddresses;
	QList<QHostAddress> mKnownIpAddresses;
	QStringList mInverterIds;
};

#endif // SETTINGS_H

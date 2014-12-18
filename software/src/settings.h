#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QMetaType>

Q_DECLARE_METATYPE(QList<QHostAddress>)
Q_DECLARE_METATYPE(QHostAddress)

class Settings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool autoDetect READ autoDetect WRITE setAutoDetect NOTIFY autoDetectChanged)
	Q_PROPERTY(QList<QHostAddress> ipAddresses READ ipAddresses WRITE setIpAddresses NOTIFY ipAddressesChanged)
	Q_PROPERTY(QList<QHostAddress> knownIpAddresses READ knownIpAddresses WRITE setKnownIpAddresses NOTIFY knownIpAddressesChanged)
public:
	explicit Settings(QObject *parent = 0);

	bool autoDetect() const;

	void setAutoDetect(bool a);

	const QList<QHostAddress> &ipAddresses() const;

	void setIpAddresses(const QList<QHostAddress> &addresses);

	const QList<QHostAddress> &knownIpAddresses() const;

	void setKnownIpAddresses(const QList<QHostAddress> &addresses);

signals:
	void autoDetectChanged();

	void ipAddressesChanged();

	void knownIpAddressesChanged();

private:
	bool mAutoDetect;
	QList<QHostAddress> mIpAddresses;
	QList<QHostAddress> mKnownIpAddresses;
};

#endif // SETTINGS_H

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QMetaType>
#include "inverter_settings.h"

class Settings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool autoDetect READ autoDetect WRITE setAutoDetect NOTIFY autoDetectChanged)
	Q_PROPERTY(int portNumber READ portNumber WRITE setPortNumber NOTIFY portNumberChanged)
	Q_PROPERTY(QList<QHostAddress> ipAddresses READ ipAddresses WRITE setIpAddresses NOTIFY ipAddressesChanged)
	Q_PROPERTY(QList<QHostAddress> knownIpAddresses READ knownIpAddresses WRITE setKnownIpAddresses NOTIFY knownIpAddressesChanged)
	Q_PROPERTY(QList<InverterSettings *> inverterSettings READ inverterSettings WRITE setInverterSettings NOTIFY inverterSettingsChanged)
public:
	explicit Settings(QObject *parent = 0);

	bool autoDetect() const;

	void setAutoDetect(bool a);

	int portNumber() const;

	void setPortNumber(int p);

	const QList<QHostAddress> &ipAddresses() const;

	void setIpAddresses(const QList<QHostAddress> &addresses);

	const QList<QHostAddress> &knownIpAddresses() const;

	void setKnownIpAddresses(const QList<QHostAddress> &addresses);

	const QList<InverterSettings *> &inverterSettings() const;

	void setInverterSettings(const QList<InverterSettings *> &settings);

	InverterSettings *findInverterSettings(const QString &uniqueId);

signals:
	void autoDetectChanged();

	void portNumberChanged();

	void ipAddressesChanged();

	void knownIpAddressesChanged();

	void inverterSettingsChanged();

private:
	bool mAutoDetect;
	int mPortNumber;
	QList<QHostAddress> mIpAddresses;
	QList<QHostAddress> mKnownIpAddresses;
	QList<InverterSettings *> mInverterSettings;
};

#endif // SETTINGS_H

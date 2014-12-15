#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QHostAddress>
#include <QList>

class Settings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool autoDetect READ autoDetect WRITE setAutoDetect)
	Q_PROPERTY(QList<QHostAddress> ipAddresses READ ipAddresses WRITE setIpAddresses)
public:
	explicit Settings(QObject *parent = 0);

	bool autoDetect() const;

	void setAutoDetect(bool a);

	const QList<QHostAddress> &ipAddresses() const;

	void setIpAddresses(const QList<QHostAddress> &addresses);

signals:
	void propertyChanged(const QString &property);

private:
	bool mAutoDetect;
	QList<QHostAddress> mIpAddresses;
};

#endif // SETTINGS_H

#ifndef FRONIUS_UDP_DETECTOR_H
#define FRONIUS_UDP_DETECTOR_H

#include <QObject>
#include <QHostAddress>
#include <QSet>
#include <QList>

class QTimer;
class QUdpSocket;

class FroniusUdpDetector: public QObject
{
	Q_OBJECT
public:
	FroniusUdpDetector(QObject *parent = 0);
	void reset();
	void start();
	QList<QHostAddress> devicesFound() { return mDevicesFound.toList(); }

signals:
	void finished();

private slots:
	void responseReceived();

private:
	QTimer *mTimeout;
	QUdpSocket *mUdpSocket;
	QSet<QHostAddress> mDevicesFound;
};

#endif

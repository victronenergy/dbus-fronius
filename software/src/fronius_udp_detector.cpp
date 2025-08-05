#include <QNetworkInterface>
#include <QHostAddress>
#include <QTimer>
#include <QByteArray>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include "fronius_udp_detector.h"

FroniusUdpDetector::FroniusUdpDetector(QObject *parent) :
	QObject(parent),
	mTimeout(new QTimer(this)),
	mUdpSocket(new QUdpSocket(this))
{
	mTimeout->setSingleShot(true);
	connect(mTimeout, SIGNAL(timeout()), this, SIGNAL(finished()));

	// Fronius inverters respond on this port
	mUdpSocket->bind(QHostAddress::AnyIPv4, 50050, QUdpSocket::ReuseAddressHint);
	connect(mUdpSocket, SIGNAL(readyRead()), this, SLOT(responseReceived()));
}

void FroniusUdpDetector::reset()
{
	mDevicesFound.clear();
}

void FroniusUdpDetector::start()
{
	// Probe for inverters by broadcasting on 50049
	QByteArray dgram = "{\"GetFroniusLoggerInfo\":\"all\"}";
	mUdpSocket->writeDatagram(dgram, QHostAddress::Broadcast, 50049);

	// Give the inverters 5 seconds to respond before blindly moving on
	mTimeout->start(5000);
}

void FroniusUdpDetector::responseReceived()
{
	while (mUdpSocket->hasPendingDatagrams()) {
		QNetworkDatagram datagram = mUdpSocket->receiveDatagram();
		mDevicesFound.insert(datagram.senderAddress());
	}
}

QList<QHostAddress> FroniusUdpDetector::devicesFound()
{
	return QList<QHostAddress>(mDevicesFound.begin(), mDevicesFound.end());
}

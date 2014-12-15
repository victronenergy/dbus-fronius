#include <velib/qt/v_busitem.h>
#include "settings.h"

Settings::Settings(QObject *parent) :
	QObject(parent),
	mAutoDetect(true)
{
}

bool Settings::autoDetect() const
{
	return mAutoDetect;
}

void Settings::setAutoDetect(bool a)
{
	if (mAutoDetect == a)
		return;
	qDebug() << __FUNCTION__ << a;
	mAutoDetect = a;
	emit propertyChanged("autoDetect");
}

const QList<QHostAddress> &Settings::ipAddresses() const
{
	return mIpAddresses;
}

void Settings::setIpAddresses(const QList<QHostAddress> &addresses)
{
	qDebug() << __FUNCTION__ << addresses;
	mIpAddresses = addresses;
	emit propertyChanged("ipAddresses");
}

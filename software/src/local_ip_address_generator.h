#ifndef LOCAL_IP_ADDRESS_GENERATOR_H
#define LOCAL_IP_ADDRESS_GENERATOR_H

#include <QHostAddress>

class LocalIpAddressGenerator
{
public:
	LocalIpAddressGenerator();

	QHostAddress next();

	bool hasNext() const;

private:
	quint32 mCurrent;
	quint32 mLast;
	quint32 mLocalHost;
};

#endif // LOCAL_IP_ADDRESS_GENERATOR_H

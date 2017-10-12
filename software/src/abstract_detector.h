#ifndef ABSTRACT_DETECTOR_H
#define ABSTRACT_DETECTOR_H

#include <QObject>
#include "defines.h"

class DetectorReply : public QObject
{
	Q_OBJECT
public:
	virtual QString hostName() const = 0;

signals:
	void deviceFound(const DeviceInfo &info);

	void finished();

protected:
	explicit DetectorReply(QObject *parent = 0);
};


class AbstractDetector : public QObject
{
	Q_OBJECT
public:
	virtual DetectorReply *start(const QString &hostName) = 0;

protected:
	explicit AbstractDetector(QObject *parent = 0);
};

#endif // ABSTRACT_DETECTOR_H

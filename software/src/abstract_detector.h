#ifndef ABSTRACT_DETECTOR_H
#define ABSTRACT_DETECTOR_H

#include <QObject>
#include "defines.h"

/*!
 * Tracks a device detection process started by a `AbstractDetector` class.
 */
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


/*!
 * Base class for classes that can detect the presence of a PV inverter on a specific host.
 */
class AbstractDetector : public QObject
{
	Q_OBJECT
public:
	/*!
	 * Start detection of a PV inverter on the specified host.
	 * This function may be called more than once with different hostNames, there is no need to
	 * wait for result from earlier calls first.
	 * @param hostName The hostname
	 * @return An reply object which should be used to observe progress and obtain the results of
	 * the scan. For each inverter found, the `deviceFound` signal will be emitted. On completion
	 * the `finished` signal will be emitted. Be aware that the `deviceFound` signal may be emitted
	 * more than once (but always before the `finished` signal is emitted). The user of the
	 * `AbstractDetector` class is responsible for deleting the reply. Usually this is done by
	 * called `deleteLater` in the slot handling the `finished` signal (do not delete the reply
	 * there).
	 */
	virtual DetectorReply *start(const QString &hostName) = 0;

protected:
	explicit AbstractDetector(QObject *parent = 0);
};

#endif // ABSTRACT_DETECTOR_H

#ifndef INVERTERSETTINGS_H
#define INVERTERSETTINGS_H

#include <QMetaType>
#include <QObject>
#include "defines.h"

class InverterSettings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString uniqueId READ uniqueId)
	Q_PROPERTY(int deviceType READ deviceType)
	Q_PROPERTY(QString customName READ customName WRITE setCustomName NOTIFY customNameChanged)
	Q_PROPERTY(InverterPhase phase READ phase WRITE setPhase NOTIFY phaseChanged)
	Q_PROPERTY(InverterPosition position READ position WRITE setPosition NOTIFY positionChanged)
	Q_PROPERTY(int deviceInstance READ deviceInstance WRITE setDeviceInstance NOTIFY deviceInstanceChanged)
	Q_PROPERTY(double l1Energy READ l1Energy WRITE setL1Energy NOTIFY l1EnergyChanged)
	Q_PROPERTY(double l2Energy READ l2Energy WRITE setL2Energy NOTIFY l2EnergyChanged)
	Q_PROPERTY(double l3Energy READ l3Energy WRITE setL3Energy NOTIFY l3EnergyChanged)
public:
	InverterSettings(int deviceType, const QString &uniqueId,
					 QObject *parent = 0);

	QString uniqueId() const;

	int deviceType() const;

	InverterPhase phase() const;

	void setPhase(InverterPhase phase);

	InverterPosition position() const;

	void setPosition(InverterPosition position);

	int deviceInstance() const;

	void setDeviceInstance(int instance);

	QString customName() const;

	void setCustomName(const QString &n);

	double l1Energy() const;

	void setL1Energy(double e);

	double l2Energy() const;

	void setL2Energy(double e);

	double l3Energy() const;

	void setL3Energy(double e);

	double getEnergy(InverterPhase phase) const;

	void setEnergy(InverterPhase phase, double value);

signals:
	void phaseChanged();

	void positionChanged();

	void deviceInstanceChanged();

	void customNameChanged();

	void l1EnergyChanged();

	void l2EnergyChanged();

	void l3EnergyChanged();

private:
	QString mUniqueId;
	int mDeviceType;
	int mDeviceInstance;
	QString mCustomName;
	InverterPhase mPhase;
	InverterPosition mPosition;
	double mL1Energy;
	double mL2Energy;
	double mL3Energy;
};

Q_DECLARE_METATYPE(InverterPhase)
Q_DECLARE_METATYPE(InverterPosition)

#endif // INVERTERSETTINGS_H

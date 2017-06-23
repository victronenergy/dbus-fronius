#ifndef INVERTERSETTINGS_H
#define INVERTERSETTINGS_H

#include "defines.h"
#include "ve_qitem_consumer.h"

class InverterSettings : public VeQItemConsumer
{
	Q_OBJECT
public:
	InverterSettings(VeQItem *root, QObject *parent = 0);

	InverterPhase phase() const;

	void setPhase(InverterPhase phase);

	InverterPosition position() const;

	QString customName() const;

	void setCustomName(const QString &n);

	bool isActive() const;

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

	void customNameChanged();

	void isActiveChanged();

	void l1EnergyChanged();

	void l2EnergyChanged();

	void l3EnergyChanged();

private:
	static VeQItem *getSettingsRoot(VeQItem *root, int deviceType, const QString &uniqueId);

	VeQItem *mPhase;
	VeQItem *mPosition;
	VeQItem *mCustomName;
	VeQItem *mIsActive;
	VeQItem *mL1Energy;
	VeQItem *mL2Energy;
	VeQItem *mL3Energy;
};

#endif // INVERTERSETTINGS_H

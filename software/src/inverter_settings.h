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

	void setPhaseCount(int phaseCount);

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

	void setSerialNumber(const QString &s);

	bool enableLimiter() const;

signals:
	void phaseChanged();

	void positionChanged();

	void customNameChanged();

	void isActiveChanged();

	void l1EnergyChanged();

	void l2EnergyChanged();

	void l3EnergyChanged();

private:
	VeQItem *mPhase;
	VeQItem *mPhaseCount;
	VeQItem *mPosition;
	VeQItem *mCustomName;
	VeQItem *mIsActive;
	VeQItem *mL1Energy;
	VeQItem *mL2Energy;
	VeQItem *mL3Energy;
	VeQItem *mSerialNumber;
	VeQItem *mEnableLimiter;
};

#endif // INVERTERSETTINGS_H

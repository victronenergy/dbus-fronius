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

	void setPosition(InverterPosition p);

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

	void setLimiterSupported(LimiterSupport v);

	bool enableLimiter() const;

	double lastTotalEnergy() const;

	void setLastTotalEnergy(double e);

signals:
	void phaseChanged();

	void positionChanged();

	void customNameChanged();

	void isActiveChanged();

	void l1EnergyChanged();

	void l2EnergyChanged();

	void l3EnergyChanged();

	void enableLimiterChanged();

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
	VeQItem *mLimiterSupported;
	VeQItem *mEnableLimiter;
	// Not stored in localsettings. It lives here, rather than with the
	// inverter, so that it survives losing the connection overnight.
	double mLastTotalEnergy;
};

#endif // INVERTERSETTINGS_H

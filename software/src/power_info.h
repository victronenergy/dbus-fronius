#ifndef POWER_INFO_H
#define POWER_INFO_H

#include "ve_service.h"

class BasicPowerInfo : public VeService
{
	Q_OBJECT
public:
	explicit BasicPowerInfo(VeQItem *root, QObject *parent = 0);

	double power() const;

	void setPower(double p);

	double totalEnergy() const;

	void setTotalEnergy(double e);

	void resetValues();

private:
	VeQItem *mPower;
	VeQItem *mTotalEnergy;
};

class PowerInfo : public BasicPowerInfo
{
	Q_OBJECT
public:
	explicit PowerInfo(VeQItem *root, QObject *parent = 0);

	double current() const;

	void setCurrent(double c);

	double voltage() const;

	void setVoltage(double v);

	/*!
	 * @brief Reset all measured values to NaN
	 */
	void resetValues();

private:
	VeQItem *mCurrent;
	VeQItem *mVoltage;
};

#endif // POWER_INFO_H

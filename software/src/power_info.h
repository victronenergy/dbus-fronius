#ifndef POWER_INFO_H
#define POWER_INFO_H

#include <QObject>

class PowerInfo : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double current READ current WRITE setCurrent NOTIFY currentChanged)
	Q_PROPERTY(double voltage READ voltage WRITE setVoltage NOTIFY voltageChanged)
	Q_PROPERTY(double power READ power WRITE setPower NOTIFY powerChanged)
	Q_PROPERTY(double totalEnergy READ totalEnergy WRITE setTotalEnergy	NOTIFY totalEnergyChanged)
public:
	explicit PowerInfo(QObject *parent = 0);

	double current() const;

	void setCurrent(double c);

	double voltage() const;

	void setVoltage(double v);

	double power() const;

	void setPower(double p);

	double totalEnergy() const;

	void setTotalEnergy(double e);

	/*!
	 * @brief Reset all measured values to NaN
	 */
	void resetValues();

signals:
	void currentChanged();

	void voltageChanged();

	void powerChanged();

	void totalEnergyChanged();

private:
	double mCurrent;
	double mVoltage;
	double mPower;
	double mTotalEnergy;
};

#endif // POWER_INFO_H

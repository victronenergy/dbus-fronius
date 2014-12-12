#ifndef POWER_INFO_H
#define POWER_INFO_H

#include <QObject>

class PowerInfo : public QObject
{
	Q_OBJECT
	Q_PROPERTY(double current READ current WRITE setCurrent)
	Q_PROPERTY(double voltage READ voltage WRITE setVoltage)
	Q_PROPERTY(double power READ power WRITE setPower)
public:
	explicit PowerInfo(QObject *parent = 0);

	double current() const;

	void setCurrent(double c);

	double voltage() const;

	void setVoltage(double v);

	double power() const;

	void setPower(double p);

signals:
	void propertyChanged(const QString &property);

private:
	double mCurrent;
	double mVoltage;
	double mPower;
};

#endif // POWER_INFO_H

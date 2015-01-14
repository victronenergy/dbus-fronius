#ifndef INVERTERSETTINGS_H
#define INVERTERSETTINGS_H

#include <QMetaType>
#include <QObject>

class InverterSettings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString uniqueId READ uniqueId)
	Q_PROPERTY(QString customName READ customName WRITE setCustomName NOTIFY customNameChanged)
	// Without the namespace prefixes below, we cannot use the setProperty
	// function to set change the properties.
	Q_PROPERTY(InverterSettings::Phase phase READ phase WRITE setPhase NOTIFY phaseChanged)
	Q_PROPERTY(InverterSettings::Position position READ position WRITE setPosition NOTIFY positionChanged)
	Q_PROPERTY(int deviceInstance READ deviceInstance NOTIFY deviceInstanceChanged)
public:
	enum Phase {
		/*!
		 * Inverter produces 3 phased power
		 */
		AllPhases,
		L1,
		L2,
		L3
	};

	enum Position {
		Input1 = 0,
		Output = 1,
		Input2 = 2
	};

	explicit InverterSettings(const QString &uniqueId, QObject *parent = 0);

	QString uniqueId() const;

	Phase phase() const;

	void setPhase(Phase phase);

	Position position() const;

	void setPosition(Position position);

	int deviceInstance() const;

	QString customName() const;

	void setCustomName(const QString &n);

signals:
	void phaseChanged();

	void positionChanged();

	void deviceInstanceChanged();

	void customNameChanged();

private:
	QString mUniqueId;
	QString mCustomName;
	Phase mPhase;
	Position mPosition;
};

Q_DECLARE_METATYPE(InverterSettings::Phase)
Q_DECLARE_METATYPE(InverterSettings::Position)

#endif // INVERTERSETTINGS_H

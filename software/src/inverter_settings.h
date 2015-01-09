#ifndef INVERTERSETTINGS_H
#define INVERTERSETTINGS_H

#include <QMetaType>
#include <QObject>

class InverterSettings : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString uniqueId READ uniqueId)
	Q_PROPERTY(Phase phase READ phase WRITE setPhase NOTIFY phaseChanged)
	Q_PROPERTY(Position position READ position WRITE setPosition NOTIFY positionChanged)
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

	~InverterSettings();

	QString uniqueId() const;

	Phase phase() const;

	void setPhase(Phase phase);

	Position position() const;

	void setPosition(Position position);

signals:
	void phaseChanged();

	void positionChanged();

private:
	QString mUniqueId;
	Phase mPhase;
	Position mPosition;
};

Q_DECLARE_METATYPE(InverterSettings::Phase)
Q_DECLARE_METATYPE(InverterSettings::Position)

#endif // INVERTERSETTINGS_H

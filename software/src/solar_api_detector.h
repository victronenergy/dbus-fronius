#ifndef SOLAR_API_DETECTOR_H
#define SOLAR_API_DETECTOR_H

#include <QList>
#include <QHash>
#include "abstract_detector.h"
#include "froniussolar_api.h"

class Settings;
class SunspecDetector;

class SolarApiDetector: public AbstractDetector
{
	Q_OBJECT
public:
	SolarApiDetector(const Settings *settings, QObject *parent = 0);

	virtual DetectorReply *start(const QString &hostName);

private slots:
	void onDeviceInfoFound(const DeviceInfoData &data);

	void onConverterInfoFound(const InverterListData &data);

	void onSunspecDeviceFound(const DeviceInfo &info);

	void onSunspecDone();

private:
	class Reply: public DetectorReply
	{
	public:
		Reply(QObject *parent = 0);

		virtual ~Reply();

		virtual QString hostName() const
		{
			return api->hostName();
		}

		void setResult(const DeviceInfo &di)
		{
			emit deviceFound(di);
		}

		void setFinished()
		{
			emit finished();
		}

		FroniusSolarApi *api;
		QMap<int, QString> serialInfo; // A place to store serial info for later use
	};

	class Api: public FroniusSolarApi
	{
	public:
		Api(const QString &hostName, int port, Reply *parent = 0) :
			FroniusSolarApi(hostName, port, parent) {}
	};

	struct ReplyToInverter {
		ReplyToInverter():
			reply(0),
			deviceFound(false) {}
		Reply *reply;
		InverterInfo inverter;
		bool deviceFound;
	};

	static QString fixUniqueId(const InverterInfo &inverterInfo);

	void checkFinished(Reply *reply);

	static QList<QString> mInvalidDevices;
	QHash<DetectorReply *, ReplyToInverter> mDetectorReplyToInverter;
	SunspecDetector *mSunspecDetector;
	const Settings *mSettings;
};

#endif // SOLAR_API_DETECTOR_H

#ifndef DBUSSETTINGSBRIDGETEST_H
#define DBUSSETTINGSBRIDGETEST_H

#include <QObject>

class QVariant;

class DbusSettingsBridgeTest : public QObject
{
	Q_OBJECT
public:
	explicit DbusSettingsBridgeTest(QObject *parent = 0);

private slots:
	void init();

	void cleanup();

	void addObjects();

	void changeAutoDetect();

	void changeAutoDetectRemote();

	void changeIPAddresses();

	void changeIPAddressesRemote();

	void changeKnownIPAddresses();

	void changeKnownIPAddressesRemote();

private:
	void changeIPAddresses(const char *property, const char *path);

	void changeIPAddressesRemote(const char *property, const char *path);

	void processEvents(int sleep, int count);

	void checkValue(const QVariant &actual, const QVariant &expected);
};

#endif // DBUSSETTINGSBRIDGETEST_H

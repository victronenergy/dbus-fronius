#include <QByteArray>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QString>
#include <QVariant>
#include <unistd.h>
#include "test_helper.h"

void qWait(int ms)
{
	QElapsedTimer timer;
	timer.start();
	do {
		QCoreApplication::processEvents(QEventLoop::AllEvents, ms);
		usleep(10000);
	} while (timer.elapsed() < ms);
}

void PrintTo(const QString &s, std::ostream *os)
{
	(*os) << s.toLatin1().data();
}

void PrintTo(const QVariant &s, std::ostream *os)
{
	PrintTo(s.toString(), os);
}

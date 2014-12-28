#include <QCoreApplication>
#include <QElapsedTimer>
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

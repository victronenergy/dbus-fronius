#include <QtGlobal>

static bool _debugLogging = false;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

#include <QLoggingCategory>

void initLogging(bool logDebug)
{
	QLoggingCategory::defaultCategory()->setEnabled(QtDebugMsg, logDebug);
	qSetMessagePattern("%{type} %{message}");
	_debugLogging = logDebug;
}
#else

#include <QTextStream>

static QTextStream out(stderr);

void messageHandler(QtMsgType type, const char *msg)
{
	// This is only used in QT4
	switch (type)
	{
	case QtWarningMsg:
		out << "WARN " << msg << endl;
		break;
	case QtCriticalMsg:
		out << "ERROR " << msg << endl;
		break;
	case QtDebugMsg:
	default:
		if (msg[0] == '@')
			out << "INFO " << (msg+1) << endl;
		else if (_debugLogging)
			out << "DEBUG " << msg << endl;
	}
}


void initLogging(bool logDebug)
{
	qInstallMsgHandler(messageHandler);
	_debugLogging = logDebug;
}
#endif

bool debugLogging()
{
	return _debugLogging;
}

#include <QCoreApplication>
#include <QTextStream>
#include "arguments.h"

Arguments::Arguments()
{
	QStringList argList = QCoreApplication::arguments();
	QString switchName;
	int listSize = argList.size();
	for (int i = 1; i < listSize; i++) {
		QString arg = argList.at(i);
		if (arg.startsWith('-')) {
			arg.remove(0,1);
			if (arg.startsWith('-'))
				arg.remove(0,1);
			if (!arg.isEmpty()) {
				if (switchName.isEmpty())
					switchName = arg;
				else {
					mArgList.insert(switchName, QString());
					switchName = arg;
				}
			}
		} else {
			mArgList.insert(switchName, arg);
			switchName.clear();
		}
	}
	if (!switchName.isEmpty())
		mArgList.insert(switchName, QString());
}

void Arguments::print()
{
	QTextStream out(stdout);
	QMap<QString, QString>::const_iterator i = mArgList.constBegin();
	while (i != mArgList.constEnd()) {
		out << "key = " << i.key() << " value = " << i.value() << "\n";
		++i;
	}
}

void Arguments::help()
{
	version();
	QTextStream out(stdout);
	out << endl;
	out << "Options:" << endl;
	QMap<QString, QString>::iterator i;
	for (i = mHelp.begin(); i != mHelp.end(); ++i) {
		out << i.key() << endl;
		out << "\t" << i.value() << endl;
	}
	out << endl;
}

void Arguments::version()
{
	QTextStream out(stdout);
	out << QCoreApplication::applicationName() << " v"
		<< QCoreApplication::applicationVersion() << endl;
}

void Arguments::addArg(const QString & arg, const QString & description)
{
	mHelp.insert(arg,description);
}

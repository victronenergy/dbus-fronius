#ifndef _LOGGING_H
#define _LOGGING_H

#include <QtDebug>

void setDebugLogging(bool);
void initLogging(bool);
bool debugLogging();

/* Before QT5 we won't have qInfo. Use preprocessor to prefix strings
   with an indicator character so the formatter can print it as
   info string */
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#define qInfo() qDebug() << "@"
#endif

#endif

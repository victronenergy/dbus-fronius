#ifndef SUNSPEC_TOOLS_H
#define SUNSPEC_TOOLS_H

#include <QString>
#include <QVector>

double getRawValue(const QVector<quint16> &values, int offset, int size);

double getScaledValue(const QVector<quint16> &values, int offset, int size,
					  int scaleOffset, bool isSigned);

double getScale(const QVector<quint16> &values, int offset);

double getFloat(const QVector<quint16> &values, int offset);

QString getString(const QVector<quint16> &values, int offset, int size);

#endif // SUNSPEC_TOOLS_H

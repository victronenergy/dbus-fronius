#include <qmath.h>
#include <qnumeric.h>
#include "sunspec_tools.h"

double getScaledValue(const QVector<quint16> &values, int offset, int size, int scaleOffset,
					  bool isSigned)
{
	Q_ASSERT(size > 0 && size < 3);
	double scale = getScale(values, scaleOffset);
	if (!qIsFinite(scale))
		return qQNaN();
	quint32 v = 0;
	switch (size) {
	case 1:
		v = values[offset];
		if (isSigned && v == 0x8000)
			return qQNaN();
		if (!isSigned && v == 0xFFFF)
			return qQNaN();
		break;
	case 2:
		v = static_cast<quint32>((values[offset] << 16) | values[offset + 1]);
		if (isSigned && v == 0x80000000u)
			return qQNaN();
		if (!isSigned && v == 0xFFFFFFFFu)
			return qQNaN();
		break;
	}
	double value = isSigned ?
		static_cast<double>(size==1?static_cast<qint16>(v):static_cast<qint32>(v)) :
		static_cast<double>(v);
	return value * scale;
}

double getFloat(const QVector<quint16> &values, int offset)
{
	// gcc 5.4 generates warning about strict aliasing when we compute a quint32 and cast its
	// address to a float pointer. If we use a union instead we do the same thing, but there is
	// no warning.
	union {
		quint32 v;
		float f;
	} vf;
	vf.v = static_cast<quint32>((values[offset] << 16) | values[offset + 1]);
	return static_cast<double>(vf.f);
}

QString getString(const QVector<quint16> &values, int offset, int size)
{
	QString result;
	for (int i=0; i<size; ++i) {
		result += QChar(values[offset + i] >> 8);
		result += QChar(values[offset + i] & 0xFF);
	}
	result.remove(QChar(0));
	return result;
}

double getScale(const QVector<quint16> &values, int offset)
{
	quint16 v = values[offset];
	if (v == 0x8000)
		return qQNaN();
	return qPow(10.0, static_cast<qint16>(v));
}

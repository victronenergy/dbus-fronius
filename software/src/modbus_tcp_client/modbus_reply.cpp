#include <QMetaEnum>
#include "modbus_reply.h"

ModbusReply::ModbusReply(QObject *parent) :
	QObject(parent),
	mError(NoException)
{
}

QString ModbusReply::toString() const
{
	QString s;
	if (mError != NoException) {
		const QMetaObject &mo = ModbusReply::staticMetaObject;
		int index = mo.indexOfEnumerator("ExceptionCode");
		QMetaEnum metaEnum = mo.enumerator(index);
		s += QString("Error: %2\t").arg(metaEnum.valueToKey(mError));
	}
	if (mRegisters.isEmpty())
		return s;
	s += "Registers: [";
	int i = 0;
	foreach (quint16 v, mRegisters) {
		s += QString::number(i);
		s += ':';
		s += "0x";
		s += QString::number(v, 16).toUpper();
		s += ", ";
		++i;
	}
	s.remove(s.size() - 2, 2);
	s += ']';
	return s;
}

void ModbusReply::setResult(const QVector<quint16> &registers)
{
	if (isFinished())
		return;
	Q_ASSERT(mRegisters.isEmpty());
	mRegisters = registers;
	mError = NoException;
	onFinished();
	Q_ASSERT(isFinished());
	emit finished();
}

void ModbusReply::setResult(ModbusReply::ExceptionCode error)
{
	if (isFinished())
		return;
	mError = error;
	onFinished();
	Q_ASSERT(isFinished());
	emit finished();
}

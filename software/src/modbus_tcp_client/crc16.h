#ifndef CRC16_H
#define CRC16_H

#include <QByteArray>
#include <stdint.h>

inline quint8 msb(quint16 d)
{
    return d >> 8;
}

inline quint8 lsb(quint16 d)
{
    return d & 0xFF;
}

inline quint16 toUInt16(quint8 msb, quint8 lsb)
{
    return static_cast<quint16>((msb << 8) | lsb);
}

inline quint16 toUInt16(const QByteArray &a, int offset)
{
    return toUInt16(static_cast<quint8>(a[offset]), static_cast<quint8>(a[offset + 1]));
}

/*!
 * Computes CRC16 checksum according to the Modbus TCU standard.
 */
class Crc16
{
public:
	Crc16();

	/*!
	 * @brief Returns the CRC16 computed over all bytes passed to the `add`
	 * functions since creation of the object or the last call to `reset`.
	 * @return The CRC16 checksum
	 */
	uint16_t getValue() const
	{
		return toUInt16(mCrcHi, mCrcLo);
	}

	void add(uint8_t byte);

	void add(const QByteArray &bytes);

	void reset()
	{
		mCrcLo = 0xFF;
		mCrcHi = 0xFF;
	}

	/*!
	 * Computes a CRC16 checksum
	 * @param bytes The bytes whose CRC16 will be computed.
	 * @return The CRC16 checksum
	 */
	static uint16_t getValue(const QByteArray &bytes);

private:
	uint8_t mCrcLo;
	uint8_t mCrcHi;
};

#endif // CRC16_H

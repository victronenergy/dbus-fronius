#ifndef LOCAL_IP_ADDRESS_GENERATOR_H
#define LOCAL_IP_ADDRESS_GENERATOR_H

#include <QHostAddress>
#include <QList>

/*!
 * @brief An iterator like object, which enumerates all IP addresses within the
 * local subnet (except the IP address of the localhost).
 * Example:
 * @code
 * LocalIpAddressGenerator g;
 * while (g.hasNext()) {
 *	useAddress(g.next());
 * }
 * @endcode
 */
class LocalIpAddressGenerator
{
public:
	LocalIpAddressGenerator();

	LocalIpAddressGenerator(const QList<QHostAddress> &priorityAddresses,
							bool priorityOnly = true);

	/*!
	 * @brief Returns the next address. Call @ref hasNext first to check if
	 * another address is aviable.
	 * @return the next address
	 */
	QHostAddress next();

	/*!
	 * @brief Checks if another address is available.
	 * @return true if another address is available.
	 */
	bool hasNext() const;

	void reset();

	bool priorityOnly() const;

	void setPriorityOnly(bool p);

	const QList<QHostAddress> &priorityAddresses() const;

	void setPriorityAddresses(const QList<QHostAddress> &addresses);

private:
	bool mPriorityOnly;
	quint32 mCurrent;
	quint32 mLast;
	quint32 mLocalHost;
	QList<QHostAddress> mPriorityAddresses;
	int mPriorityIndex;
};

#endif // LOCAL_IP_ADDRESS_GENERATOR_H

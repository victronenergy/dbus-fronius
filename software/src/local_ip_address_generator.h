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

	/*!
	 * \brief Resets the generator. The `next` function will return the first
	 * IP address after calling this function.
	 */
	void reset();

	/*!
	 * \brief Returns the percentage of work done.
	 * \return The progress percentage
	 */
	int progress() const;

	/*!
	 * \brief If true, the generator will only return value from the
	 * list returned by `priorityAddresses()`.
	 * \return
	 */
	bool priorityOnly() const;

	void setPriorityOnly(bool p);

	/*!
	 * \brief Returns the list of addresses that will be returned before
	 * the generator starts enumerating the IP addresses in the local network.
	 * \return
	 */
	const QList<QHostAddress> &priorityAddresses() const;

	void setPriorityAddresses(const QList<QHostAddress> &addresses);

	QHostAddress netMaskLimit() const;

	void setNetMaskLimit(const QHostAddress &limit);

private:
	bool mPriorityOnly;
	quint32 mFirst;
	quint32 mCurrent;
	quint32 mLast;
	quint32 mLocalHost;
	QList<QHostAddress> mPriorityAddresses;
	QHostAddress mNetMaskLimit;
	int mPriorityIndex;
};

#endif // LOCAL_IP_ADDRESS_GENERATOR_H

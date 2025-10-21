#ifndef LOCAL_IP_ADDRESS_GENERATOR_H
#define LOCAL_IP_ADDRESS_GENERATOR_H

#include <QHostAddress>
#include <QList>
#include <QSet>

class LocalIpAddressGenerator;

class Subnet
{
public:
	Subnet(LocalIpAddressGenerator *generator, quint32 first, quint32 last, quint32 localhost);
	bool hasNext() const;
	QHostAddress next();
	int size() const { return mLast - mFirst + 1; }
	int position() const;
private:
	LocalIpAddressGenerator *mGenerator;
	quint32 mFirst;
	quint32 mCurrent;
	quint32 mLast;
	quint32 mLocalHost;
};

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
	 * \param Number of IP addresses currently under investigation (will be deducted from the
	 * number of IP addresses that have been retrieved already).
	 * \return The progress percentage
	 */
	int progress(int activeCount) const;

	/*!
	 * \brief If true, the generator will only return values from the
	 * list returned set by `setPriorityAddresses()`.
	 * \return
	 */
	void setPriorityOnly(bool p);

	void setPriorityAddresses(const QList<QHostAddress> &addresses);

	const QSet<QHostAddress> exceptions() const;

private:
	bool mPriorityOnly;
	QList<Subnet> mSubnets;
	QList<QHostAddress> mPriorityAddresses;
	int mPriorityIndex;
	int mSubnetIndex;
};

#endif // LOCAL_IP_ADDRESS_GENERATOR_H

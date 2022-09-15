/* This file contains a couple of QT6 compatibility routines. Specifically,
   QT6 removed the routines that easily convert lists to sets because
   "people abuse the API to remove duplicates".

   Other compatibility routines may be added in future.
*/

#include <QSet>
#include <QList>

template<typename T> QSet<T> listToSet(const QList<T> &list)
{
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QSet<T>(list.begin(), list.end());
	#else
	return QSet<T>::fromList(list);
	#endif
}

template<typename T> QList<T> setToList(const QSet<T> &set)
{
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	return QList<T>(set.begin(), set.end());
	#else
	return set.toList();
	#endif
}

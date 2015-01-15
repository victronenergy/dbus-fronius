#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <iostream>

class QString;
class QVariant;

/*!
 * @brief Wait the specified interval while processing events
 * \param ms The interval in milliseconds.
 */
void qWait(int ms);

// Google test relies on PrintTo functions while logging failed tests. They
// should live in the namespace	where the printed classes are located. In this
// case the global namespace since all QT objects live there.
void PrintTo(const QString &s, std::ostream *os);

void PrintTo(const QVariant &s, std::ostream *os);

#endif // TEST_HELPER_H

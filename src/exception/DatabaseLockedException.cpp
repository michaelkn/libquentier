#include <quentier/exception/DatabaseLockedException.h>

namespace quentier {

DatabaseLockedException::DatabaseLockedException(const QString & message) :
    IQuentierException(message)
{}

const QString DatabaseLockedException::exceptionDisplayName() const
{
    return QString("DatabaseLockedException");
}

} // namespace quentier

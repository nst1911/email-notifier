#include "testbasepersistentstorage.h"

TestBasePersistentStorage::TestBasePersistentStorage(const QString &dir, QObject *parent)
    : BasePersistentStorage(dir, parent)
{

}

Result<QString> TestBasePersistentStorage::readPassword() const
{
    return m_readPassword;
}

QString TestBasePersistentStorage::writePassword(const QString &password) const
{
    Q_UNUSED(password);
    return m_writePassword;
}

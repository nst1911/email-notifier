#include "testbasepersistentstorage.h"

TestBasePersistentStorage::TestBasePersistentStorage(const QString &dir, QObject *parent)
    : BasePersistentStorage(dir, parent)
{

}

Result<QString> TestBasePersistentStorage::readPassword(IDaemon::PasswordType type) const
{
    Q_UNUSED(type);
    return {};
}

QString TestBasePersistentStorage::writePassword(IDaemon::PasswordType type, const QString &password)
{
    Q_UNUSED(type);
    Q_UNUSED(password);
    return {};
}

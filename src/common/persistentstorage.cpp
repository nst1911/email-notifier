#include "persistentstorage.h"
#include <QEventLoop>
#include <qt6keychain/keychain.h>

namespace
{

Result<QString> readValueFromSecureKeychain(const QString &key, bool autoDelete)
{
    if (key.isEmpty())
    {
        return Result<QString>::error("Can't read value: Key is empty");
    }
    QKeychain::ReadPasswordJob job(PROGRAM_NAME_STRING_LITERAL);
    job.setAutoDelete(autoDelete);
    job.setKey(key);

    QEventLoop loop;
    QObject::connect(&job, &QKeychain::ReadPasswordJob::finished, &loop, &QEventLoop::quit);

    job.start();
    loop.exec();

    return job.error() == QKeychain::Error::NoError ? Result<QString>::success(job.textData()) : Result<QString>::error(job.errorString());
}

QString writeValueToSecureKeychain(const QString &key, const QString &value)
{
    if (key.isEmpty())
    {
        return "Can't write value: Key is empty";
    }
    if (value.isEmpty())
    {
        return "Can't write value: Value is empty";
    }

    QKeychain::WritePasswordJob job(PROGRAM_NAME_STRING_LITERAL);
    job.setAutoDelete(false);
    job.setKey(key);
    job.setTextData(value);

    QEventLoop loop;
    QObject::connect(&job, &QKeychain::WritePasswordJob::finished,  &loop, &QEventLoop::quit);

    job.start();
    loop.exec();

    return job.error() == QKeychain::Error::NoError ? "" : job.errorString();
}

using PasswordType = IDaemon::PasswordType;
const QMap<PasswordType, QString> c_passwordKeyMap = {
    { PasswordType::DaemonConfiguration, "password"},
    { PasswordType::FetchMailboxes, "temporary_password"},
};

}

PersistentStorage::PersistentStorage(QObject *parent)
    : BasePersistentStorage(parent)
{

}

Result<QString> PersistentStorage::readPassword(PasswordType type) const
{
    static const QMap<PasswordType, bool> autoDeleteAfterReadMap = {
        { PasswordType::DaemonConfiguration, false},
        { PasswordType::FetchMailboxes, true},
    };
    return readValueFromSecureKeychain(c_passwordKeyMap[type], autoDeleteAfterReadMap[type]);
}

QString PersistentStorage::writePassword(PasswordType type, const QString &password)
{
    return writeValueToSecureKeychain(c_passwordKeyMap[type], password);
}



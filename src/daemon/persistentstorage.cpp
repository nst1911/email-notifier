#include "log.h"
#include "persistentstorage.h"
#include <QEventLoop>
#include <qt6keychain/keychain.h>

PersistentStorage::PersistentStorage(QObject *parent)
    : BasePersistentStorage(parent)
{
    logInfo() << "PersistentStorage instance created";
}

Result<QString> PersistentStorage::readPassword() const
{
    QKeychain::ReadPasswordJob job(PROGRAM_NAME_STRING_LITERAL);
    job.setAutoDelete(false);
    job.setKey("password");

    QEventLoop loop;
    connect(&job, &QKeychain::ReadPasswordJob::finished, &loop, &QEventLoop::quit);

    job.start();
    loop.exec();

    return job.error() == QKeychain::Error::NoError ? Result<QString>::success(job.textData()) : Result<QString>::error(job.errorString());
}

QString PersistentStorage::writePassword(const QString &password) const
{
    if (password.isEmpty())
    {
        return "Password is empty";
    }

    QKeychain::WritePasswordJob job(PROGRAM_NAME_STRING_LITERAL);
    job.setAutoDelete(false);
    job.setKey("password");
    job.setTextData(password);

    QEventLoop loop;
    connect(&job, &QKeychain::WritePasswordJob::finished,  &loop, &QEventLoop::quit);

    job.start();
    loop.exec();

    return job.error() == QKeychain::Error::NoError ? "" : job.errorString();
}

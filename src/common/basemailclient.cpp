#include "basemailclient.h"

BaseMailClient::BaseMailClient(QObject *parent)
    : IMailClient(parent)
{

}

Result<QStringList> BaseMailClient::fetchMailboxes(const Configuration &config)
{
    if (!isValid())
    {
        return Result<QStringList>::error("Mail client is invalid");
    }
    if (!config.isValid())
    {
        return Result<QStringList>::error("Configuration is invalid");
    }
    return fetchMailboxesImpl(config);
}

Result<LastMessageUIDs> BaseMailClient::fetchLastMessageUIDs(const Configuration &config, const QStringList &mailboxes)
{
    if (!isValid())
    {
        return Result<LastMessageUIDs>::error("Mail client is invalid");
    }
    if (!config.isValid())
    {
        return Result<LastMessageUIDs>::error("Configuration is invalid");
    }
    if (mailboxes.isEmpty())
    {
        return Result<LastMessageUIDs>::error("Mailboxes list is invalid");
    }

    QStringList errors;
    LastMessageUIDs uids;
    for (const QString &mailbox : mailboxes)
    {
        Result<quint64> result = fetchLastMessageUID(config, mailbox);
        if (result.success())
        {
            uids.insert(mailbox, result.data());
        }
        else
        {
            errors.append(result.errorMessage());
        }
    }

    return Result<LastMessageUIDs>(errors.join("; "), uids);
}

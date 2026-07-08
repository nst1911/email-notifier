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

Result<MessageInfoMap> BaseMailClient::fetchLastMessageInfo(const Configuration &config, const QStringList &mailboxes)
{
    if (!isValid())
    {
        return Result<MessageInfoMap>::error("Mail client is invalid");
    }
    if (!config.isValid())
    {
        return Result<MessageInfoMap>::error("Configuration is invalid");
    }
    if (mailboxes.isEmpty())
    {
        return Result<MessageInfoMap>::error("Mailboxes list is invalid");
    }

    QStringList errors;
    MessageInfoMap messageInfoMap;
    for (const QString &mailbox : mailboxes)
    {
        Result<MessageInfo> result = fetchLastMessageInfoFromMailbox(config, mailbox);
        if (result.success())
        {
            messageInfoMap.insert(mailbox, result.data());
        }
        else
        {
            errors.append(result.errorMessage());
        }
    }

    return Result<MessageInfoMap>(errors.join("; "), messageInfoMap);
}

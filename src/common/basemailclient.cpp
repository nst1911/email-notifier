#include "basemailclient.h"

BaseMailClient::BaseMailClient(QObject *parent)
    : IMailClient(parent)
{

}

bool BaseMailClient::isValid() const
{
    return isConfigValid(m_config);
}

Configuration BaseMailClient::configuration() const
{
    return m_config;
}

void BaseMailClient::setConfiguration(const Configuration &config)
{
    if (isConfigValid(config))
    {
        m_config = config;
    }
}

Result<QStringList> BaseMailClient::fetchMailboxes()
{
    if (!isValid())
    {
        return Result<QStringList>::error("Configuration is invalid");
    }
    return fetchMailboxesImpl();
}

Result<LastMessageUIDs> BaseMailClient::fetchLastMessageUIDs()
{
    if (!isValid())
    {
        return Result<LastMessageUIDs>::error("Configuration is invalid");
    }

    QStringList errors;
    LastMessageUIDs uids;
    for (const QString &mailbox : configuration().mailboxes)
    {
        Result<quint64> result = fetchLastMessageUID(mailbox);
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

bool BaseMailClient::isConfigValid(const Configuration &config) const
{
    return !config.host.isEmpty() &&
           config.port != 0 &&
           !config.login.isEmpty() &&
           !config.password.isEmpty() &&
           !config.mailboxes.isEmpty();
}

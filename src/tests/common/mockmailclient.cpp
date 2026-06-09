#include "mockmailclient.h"

MockMailClient::MockMailClient(QObject *parent)
    : IMailClient(parent)
{

}

bool MockMailClient::isValid() const
{
    return m_testData.isValid;
}

Result<QStringList> MockMailClient::fetchMailboxes(const Configuration &config)
{
    Q_UNUSED(config);
    return m_testData.fetchMailboxes;
}

Result<LastMessageUIDs> MockMailClient::fetchLastMessageUIDs(const Configuration &config, const QStringList &mailboxes)
{
    Q_UNUSED(config);
    Q_UNUSED(mailboxes);
    if (m_testData.fetchLastMessageUIDsQueue.isEmpty())
    {
        Result<QStringList>::error("empty");
    }
    Result<LastMessageUIDs> result = m_testData.fetchLastMessageUIDsQueue.dequeue();
    return result;
}

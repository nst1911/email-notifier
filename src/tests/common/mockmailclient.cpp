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

Result<MessageInfoMap> MockMailClient::fetchLastMessageInfo(const Configuration &config, const QStringList &mailboxes)
{
    Q_UNUSED(config);
    Q_UNUSED(mailboxes);
    if (m_testData.fetchLastMessageInfoQueue.isEmpty())
    {
        return Result<MessageInfoMap>::error("empty");
    }
    Result<MessageInfoMap> result = m_testData.fetchLastMessageInfoQueue.dequeue();
    return result;
}

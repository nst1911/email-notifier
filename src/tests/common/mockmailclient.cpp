#include "mockmailclient.h"

MockMailClient::MockMailClient(QObject *parent)
    : IMailClient(parent)
{

}

bool MockMailClient::isValid() const
{
    return m_testData.isValid;
}

Configuration MockMailClient::configuration() const
{
    return isValid() ? m_testData.configuration : Configuration{};
}

void MockMailClient::setConfiguration(const Configuration &config)
{
    Q_UNUSED(config)
}

Result<QStringList> MockMailClient::fetchMailboxes()
{
    return isValid() ? m_testData.fetchMailboxes : Result<QStringList>::error("error");
}

Result<LastMessageUIDs> MockMailClient::fetchLastMessageUIDs()
{
    if (m_testData.fetchLastMessageUIDsQueue.isEmpty() || !isValid())
    {
        return {};
    }
    Result<LastMessageUIDs> result = m_testData.fetchLastMessageUIDsQueue.dequeue();
    return result;
}

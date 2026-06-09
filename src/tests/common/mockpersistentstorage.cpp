#include "mockpersistentstorage.h"

MockPersistentStorage::MockPersistentStorage(QObject *parent)
    : IPersistentStorage(parent)
{

}

bool MockPersistentStorage::isValid() const
{
    return m_testData.isValid;
}

Result<Message> MockPersistentStorage::readErrorLogMessage() const
{
    return isValid()
        ? m_testData.readErrorLogMessage
        : Result<Message>::error("error");
}

QString MockPersistentStorage::writeErrorLogMessage(const Message &message)
{
    m_testData.writeErrorLogMessageCalled = true;
    return isValid() ? m_testData.writeErrorLogMessage : "error";
}

Result<Configuration> MockPersistentStorage::readConfiguration() const
{
    return isValid()
        ? m_testData.readConfiguration
        : Result<Configuration>::error("error");
}

QString MockPersistentStorage::writeConfiguration(const Configuration &configuration)
{
    Q_UNUSED(configuration);
    return isValid() ? m_testData.writeConfiguration : "error";
}

Result<LastMessageUIDs> MockPersistentStorage::readLastMessageUIDs() const
{
    return isValid()
        ? m_testData.readLastMessageUIDs
        : Result<LastMessageUIDs>::error("error");
}

QString MockPersistentStorage::writeLastMessageUIDs(const LastMessageUIDs &uids)
{
    Q_UNUSED(uids);
    return isValid() ? m_testData.writeLastMessageUIDs : "error";
}

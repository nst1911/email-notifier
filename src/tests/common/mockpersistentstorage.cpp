#include "mockpersistentstorage.h"

namespace
{
constexpr const char *c_isValidErrMsg = "isValid=false";
}

MockPersistentStorage::MockPersistentStorage(QObject *parent)
    : IPersistentStorage(parent)
{

}

bool MockPersistentStorage::isValid() const
{
    return m_testData.isValid;
}

Result<LogMessage> MockPersistentStorage::readErrorLogMessage() const
{
    return isValid() ? m_testData.readErrorLogMessage : Result<LogMessage>::error(c_isValidErrMsg);
}

QString MockPersistentStorage::writeErrorLogMessage(const LogMessage &message)
{
    m_testData.writeErrorLogMessageCalled = true;
    return isValid() ? m_testData.writeErrorLogMessage : c_isValidErrMsg;
}

Result<IDaemon::Configuration> MockPersistentStorage::readDaemonConfiguration() const
{
    return isValid()
        ? m_testData.readDaemonConfiguration
        : Result<IDaemon::Configuration>::error(c_isValidErrMsg);
}

QString MockPersistentStorage::writeDaemonConfiguration(const IDaemon::Configuration &configuration)
{
    Q_UNUSED(configuration);
    m_writeDaemonConfigurationCalled = true;
    return isValid() ? m_testData.writeDaemonConfiguration : c_isValidErrMsg;
}

Result<MessageInfoMap> MockPersistentStorage::readLastMessageInfo() const
{
    return isValid() ? m_testData.readLastMessageInfo : Result<MessageInfoMap>::error(c_isValidErrMsg);
}

QString MockPersistentStorage::writeLastMessageInfo(const MessageInfoMap &messageInfoMap)
{
    Q_UNUSED(messageInfoMap);
    m_writeLastMessageInfoCalled = true;
    return isValid() ? m_testData.writeLastMessageInfo : c_isValidErrMsg;
}

Result<QString> MockPersistentStorage::readPassword(IDaemon::PasswordType type) const
{
    Q_UNUSED(type);
    return isValid() ? m_testData.readPassword : Result<QString>::error(c_isValidErrMsg);
}

QString MockPersistentStorage::writePassword(IDaemon::PasswordType type, const QString &password)
{
    Q_UNUSED(type);
    Q_UNUSED(password);
    return isValid() ? m_testData.writePassword : c_isValidErrMsg;
}

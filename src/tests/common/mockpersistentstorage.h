#pragma once

#include "interfaces/ipersistentstorage.h"
#include <QMap>

class MockPersistentStorage : public IPersistentStorage
{
    Q_OBJECT

public:
    MockPersistentStorage(QObject *parent = nullptr);

    bool isValid() const override;

    Result<LogMessage> readErrorLogMessage() const override;
    QString writeErrorLogMessage(const LogMessage &message) override;

    Result<IDaemon::Configuration> readDaemonConfiguration() const override;
    QString writeDaemonConfiguration(const IDaemon::Configuration &configuration) override;

    Result<LastMessageUIDs> readLastMessageUIDs() const override;
    QString writeLastMessageUIDs(const LastMessageUIDs &uids) override;

    Result<QString> readPassword(IDaemon::PasswordType type) const override;
    QString writePassword(IDaemon::PasswordType type, const QString &password) override;

    struct TestData
    {
        bool isValid;
        Result<LogMessage> readErrorLogMessage;
        QString writeErrorLogMessage;
        bool writeErrorLogMessageCalled = false;
        Result<IDaemon::Configuration> readDaemonConfiguration;
        QString writeDaemonConfiguration;
        Result<LastMessageUIDs> readLastMessageUIDs;
        QString writeLastMessageUIDs;
        Result<QString> readPassword;
        QString writePassword;
    } m_testData;

    bool m_writeDaemonConfigurationCalled = false;
    bool m_writeLastMessageUIDsCalled = false;
};

Q_DECLARE_METATYPE(MockPersistentStorage::TestData);

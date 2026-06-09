#pragma once

#include "interfaces/ipersistentstorage.h"
#include <QMap>

class MockPersistentStorage : public IPersistentStorage
{
    Q_OBJECT

public:
    MockPersistentStorage(QObject *parent = nullptr);

    bool isValid() const override;

    Result<Message> readErrorLogMessage() const override;
    QString writeErrorLogMessage(const Message &message) override;

    Result<Configuration> readConfiguration() const override;
    QString writeConfiguration(const Configuration &configuration) override;

    Result<LastMessageUIDs> readLastMessageUIDs() const override;
    QString writeLastMessageUIDs(const LastMessageUIDs &uids) override;

    struct TestData
    {
        bool isValid;
        Result<Message> readErrorLogMessage;
        QString writeErrorLogMessage;
        bool writeErrorLogMessageCalled = false;
        Result<Configuration> readConfiguration;
        QString writeConfiguration;
        Result<LastMessageUIDs> readLastMessageUIDs;
        QString writeLastMessageUIDs;
    } m_testData;
};

Q_DECLARE_METATYPE(MockPersistentStorage::TestData);

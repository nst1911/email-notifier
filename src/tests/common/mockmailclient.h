#pragma once

#include "interfaces/imailclient.h"
#include <QQueue>

class MockMailClient : public IMailClient
{
    Q_OBJECT

public:
    MockMailClient(QObject *parent = nullptr);

    bool isValid() const override;

    Configuration configuration() const override;
    void setConfiguration(const Configuration &config) override;

    Result<QStringList> fetchMailboxes() override;
    Result<LastMessageUIDs> fetchLastMessageUIDs() override;

    struct TestData
    {
        bool isValid;
        Configuration configuration;
        Result<QStringList> fetchMailboxes;
        QQueue<Result<LastMessageUIDs>> fetchLastMessageUIDsQueue;
    } m_testData;
};

Q_DECLARE_METATYPE(MockMailClient::TestData);

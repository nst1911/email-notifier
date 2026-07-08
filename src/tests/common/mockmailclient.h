#pragma once

#include "interfaces/imailclient.h"
#include <QQueue>

class MockMailClient : public IMailClient
{
    Q_OBJECT

public:
    MockMailClient(QObject *parent = nullptr);

    bool isValid() const override;

    Result<QStringList> fetchMailboxes(const Configuration &config) override;
    Result<MessageInfoMap> fetchLastMessageInfo(const Configuration &config, const QStringList &mailboxes) override;

    struct TestData
    {
        bool isValid;
        Configuration configuration;
        Result<QStringList> fetchMailboxes;
        QQueue<Result<MessageInfoMap>> fetchLastMessageInfoQueue;
    } m_testData;
};

Q_DECLARE_METATYPE(MockMailClient::TestData);

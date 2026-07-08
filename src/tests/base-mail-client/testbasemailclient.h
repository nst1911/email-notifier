#pragma once

#include "basemailclient.h"

class TestBaseMailClient : public BaseMailClient
{
    Q_OBJECT

public:
    TestBaseMailClient(QObject *parent = nullptr);

    bool isValid() const override;

    Result<QStringList> m_fetchMailboxesImpl = Result<QStringList>::error("error");
    Result<MessageInfo> m_fetchLastMessageInfoFromMailbox = Result<MessageInfo>::error("error");

protected:
    Result<QStringList> fetchMailboxesImpl(const Configuration &config) override;
    Result<MessageInfo> fetchLastMessageInfoFromMailbox(const Configuration &config, const QString &mailbox) override;
};

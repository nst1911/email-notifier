#pragma once

#include "export.h"
#include "interfaces/imailclient.h"

class COMMON_VISIBILITY BaseMailClient : public IMailClient
{
    Q_OBJECT

public:
    BaseMailClient(QObject *parent = nullptr);

    Result<QStringList> fetchMailboxes(const Configuration &config) override;
    Result<MessageInfoMap> fetchLastMessageInfo(const Configuration &config, const QStringList &mailboxes) override;

protected:
    virtual Result<QStringList> fetchMailboxesImpl(const Configuration &config) = 0;
    virtual Result<MessageInfo> fetchLastMessageInfoFromMailbox(const Configuration &config, const QString &mailbox) = 0;
};

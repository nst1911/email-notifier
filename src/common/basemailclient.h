#pragma once

#include "export.h"
#include "interfaces/imailclient.h"

class COMMON_VISIBILITY BaseMailClient : public IMailClient
{
    Q_OBJECT

public:
    BaseMailClient(QObject *parent = nullptr);

    Result<QStringList> fetchMailboxes(const Configuration &config) override;
    Result<LastMessageUIDs> fetchLastMessageUIDs(const Configuration &config, const QStringList &mailboxes) override;

protected:
    virtual Result<QStringList> fetchMailboxesImpl(const Configuration &config) = 0;
    virtual Result<quint64> fetchLastMessageUID(const Configuration &config, const QString &mailboxes) = 0;

private:
    Configuration m_config;
};

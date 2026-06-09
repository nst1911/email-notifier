#pragma once

#include "interfaces/imailclient.h"
#include "export.h"

class COMMON_VISIBILITY BaseMailClient : public IMailClient
{
    Q_OBJECT

public:
    BaseMailClient(QObject *parent = nullptr);

    bool isValid() const override;

    Configuration configuration() const override;
    void setConfiguration(const Configuration &config) override;

    Result<QStringList> fetchMailboxes() override;
    Result<LastMessageUIDs> fetchLastMessageUIDs() override;

protected:
    bool isConfigValid(const Configuration &config) const;
    virtual Result<QStringList> fetchMailboxesImpl() = 0;
    virtual Result<quint64> fetchLastMessageUID(const QString &mailbox) = 0;

private:
    Configuration m_config;
};

#pragma once

#include "basemailclient.h"

class TestBaseMailClient : public BaseMailClient
{
    Q_OBJECT

public:
    TestBaseMailClient(QObject *parent = nullptr);

    bool isValid() const override;

    Result<QStringList> m_fetchMailboxesImpl = Result<QStringList>::error("error");
    Result<quint64> m_fetchLastMessageUID = Result<quint64>::error("error");

protected:
    Result<QStringList> fetchMailboxesImpl(const Configuration &config) override;
    Result<quint64> fetchLastMessageUID(const Configuration &config, const QString &mailbox) override;
};

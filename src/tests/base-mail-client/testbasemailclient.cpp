#include "testbasemailclient.h"

TestBaseMailClient::TestBaseMailClient(QObject *parent)
    : BaseMailClient(parent)
{

}

bool TestBaseMailClient::isValid() const
{
    return true;
}

Result<QStringList> TestBaseMailClient::fetchMailboxesImpl(const Configuration &config)
{
    Q_UNUSED(config);
    return m_fetchMailboxesImpl;
}

Result<quint64> TestBaseMailClient::fetchLastMessageUID(const Configuration &config, const QString &mailbox)
{
    Q_UNUSED(config);
    Q_UNUSED(mailbox);
    return m_fetchLastMessageUID;
}

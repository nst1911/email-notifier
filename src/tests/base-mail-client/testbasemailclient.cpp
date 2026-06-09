#include "testbasemailclient.h"

TestBaseMailClient::TestBaseMailClient(QObject *parent)
    : BaseMailClient(parent)
{

}

Result<QStringList> TestBaseMailClient::fetchMailboxesImpl()
{
    return m_fetchMailboxesImpl;
}

Result<quint64> TestBaseMailClient::fetchLastMessageUID(const QString &mailbox)
{
    Q_UNUSED(mailbox);
    return m_fetchLastMessageUID;
}


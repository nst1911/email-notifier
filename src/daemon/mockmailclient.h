#pragma once

#include "basemailclient.h"

// MockMailClient is used for debugging
//
// It emulates working of mail client by using data from file
// (results of fetchMailboxesImpl and fetchLastMessageInfo methods)
//
// To re-read data from file, restart the daemon service
//   systemctl --user restart com.github.nst1911.emailnotifier

class MockMailClient : public BaseMailClient
{
    Q_OBJECT

public:
    MockMailClient(QObject *parent = nullptr);

    bool isValid() const override;

protected:
    Result<QStringList> fetchMailboxesImpl(const Configuration &config) override;
    Result<MessageInfo> fetchLastMessageInfoFromMailbox(const Configuration &config, const QString &mailbox) override;

private:
    void readTestData();
    void printTestData();

    QList<Result<QStringList>> m_mailboxes;
    QMap<QString, QList<Result<MessageInfo>>> m_messageInfos;

    int m_currentMailboxIndex = 0;
    QMap<QString, int> m_currentMessageInfoIndex;
};

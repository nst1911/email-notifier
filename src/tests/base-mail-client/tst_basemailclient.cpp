#include "examples.h"
#include "testbasemailclient.h"
#include <QtTest>

class BaseMailClientQtTest : public QObject
{
    Q_OBJECT

private slots:
    void fetchMailboxes_data();
    void fetchMailboxes();

    void fetchLastMessageInfo_data();
    void fetchLastMessageInfo();
};

void BaseMailClientQtTest::fetchMailboxes_data()
{
    QTest::addColumn<IMailClient::Configuration>("config");
    QTest::addColumn<bool>("expectedResultSuccess");
    QTest::addColumn<QStringList>("expectedResultMailboxes");

    {
        QTest::newRow("Happy path")
            << Examples::validMailClientConfig()
            << true
            << Examples::validMailboxes();
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.host = "";

        QTest::newRow("Invalid host")
            << config
            << false
            << QStringList{};
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.port = 0;

        QTest::newRow("Invalid port")
            << config
            << false
            << QStringList{};
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.login = "";

        QTest::newRow("Invalid login")
            << config
            << false
            << QStringList{};
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.password = "";

        QTest::newRow("Invalid password")
            << config
            << false
            << QStringList{};
    }
}

void BaseMailClientQtTest::fetchMailboxes()
{
    // Assign
    QFETCH(IMailClient::Configuration, config);
    QFETCH(bool, expectedResultSuccess);
    QFETCH(QStringList, expectedResultMailboxes);

    TestBaseMailClient mailClient;
    mailClient.m_fetchMailboxesImpl = Result<QStringList>::success(Examples::validMailboxes());

    // Act
    Result<QStringList> result = mailClient.fetchMailboxes(config);

    // Assert
    QCOMPARE(result.errorMessage().isEmpty(), expectedResultSuccess);
    QCOMPARE(result.data(), expectedResultMailboxes);
}

void BaseMailClientQtTest::fetchLastMessageInfo_data()
{
    QTest::addColumn<IMailClient::Configuration>("config");
    QTest::addColumn<QStringList>("mailboxes");
    QTest::addColumn<bool>("expectedResultSuccess");
    QTest::addColumn<MessageInfoMap>("expectedResultMessageInfoMap");

    {
        MessageInfoMap messageInfoMap;
        for (const QString &mailbox : Examples::validMailboxes())
        {
            messageInfoMap.insert(mailbox, MessageInfo(6, false));
        }

        QTest::newRow("Happy path")
            << Examples::validMailClientConfig()
            << Examples::validMailboxes()
            << true
            << messageInfoMap;
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.host = "";

        QTest::newRow("Invalid host")
            << config
            << Examples::validMailboxes()
            << false
            << MessageInfoMap{};
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.port = 0;

        QTest::newRow("Invalid port")
            << config
            << Examples::validMailboxes()
            << false
            << MessageInfoMap{};
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.login = "";

        QTest::newRow("Invalid login")
            << config
            << Examples::validMailboxes()
            << false
            << MessageInfoMap{};
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.password = "";

        QTest::newRow("Invalid password")
            << config
            << Examples::validMailboxes()
            << false
            << MessageInfoMap{};
    }

    {
        QTest::newRow("Invalid mailboxes")
            << Examples::validMailClientConfig()
            << QStringList{}
            << false
            << MessageInfoMap{};
    }
}

void BaseMailClientQtTest::fetchLastMessageInfo()
{
    // Assign
    QFETCH(IMailClient::Configuration, config);
    QFETCH(QStringList, mailboxes);
    QFETCH(bool, expectedResultSuccess);
    QFETCH(MessageInfoMap, expectedResultMessageInfoMap);

    TestBaseMailClient mailClient;
    mailClient.m_fetchLastMessageInfoFromMailbox = Result<MessageInfo>::success(MessageInfo(6, false));

    // Act
    Result<MessageInfoMap> result = mailClient.fetchLastMessageInfo(config, mailboxes);

    // Assert
    QCOMPARE(result.errorMessage().isEmpty(), expectedResultSuccess);
    QCOMPARE(result.data(), expectedResultMessageInfoMap);
}

QTEST_GUILESS_MAIN(BaseMailClientQtTest)

#include "tst_basemailclient.moc"

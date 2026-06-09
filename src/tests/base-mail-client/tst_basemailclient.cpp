#include "examples.h"
#include "testbasemailclient.h"
#include <QtTest>

class BaseMailClientQtTest : public QObject
{
    Q_OBJECT

private slots:
    void fetchMailboxes_data();
    void fetchMailboxes();

    void fetchLastMessageUIDs_data();
    void fetchLastMessageUIDs();
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

void BaseMailClientQtTest::fetchLastMessageUIDs_data()
{
    QTest::addColumn<IMailClient::Configuration>("config");
    QTest::addColumn<QStringList>("mailboxes");
    QTest::addColumn<bool>("expectedResultSuccess");
    QTest::addColumn<LastMessageUIDs>("expectedResultLastMessageUIDs");

    {
        LastMessageUIDs lastMessageUIDs;
        for (const QString &mailbox : Examples::validMailboxes())
        {
            lastMessageUIDs.insert(mailbox, 6);
        }

        QTest::newRow("Happy path")
            << Examples::validMailClientConfig()
            << Examples::validMailboxes()
            << true
            << lastMessageUIDs;
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.host = "";

        QTest::newRow("Invalid host")
            << config
            << Examples::validMailboxes()
            << false
            << LastMessageUIDs{};
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.port = 0;

        QTest::newRow("Invalid port")
            << config
            << Examples::validMailboxes()
            << false
            << LastMessageUIDs{};
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.login = "";

        QTest::newRow("Invalid login")
            << config
            << Examples::validMailboxes()
            << false
            << LastMessageUIDs{};
    }

    {
        IMailClient::Configuration config = Examples::validMailClientConfig();
        config.password = "";

        QTest::newRow("Invalid password")
            << config
            << Examples::validMailboxes()
            << false
            << LastMessageUIDs{};
    }

    {
        QTest::newRow("Invalid mailboxes")
            << Examples::validMailClientConfig()
            << QStringList{}
            << false
            << LastMessageUIDs{};
    }
}

void BaseMailClientQtTest::fetchLastMessageUIDs()
{
    // Assign
    QFETCH(IMailClient::Configuration, config);
    QFETCH(QStringList, mailboxes);
    QFETCH(bool, expectedResultSuccess);
    QFETCH(LastMessageUIDs, expectedResultLastMessageUIDs);

    TestBaseMailClient mailClient;
    mailClient.m_fetchLastMessageUID = Result<quint64>::success(6);;

    // Act
    Result<LastMessageUIDs> result = mailClient.fetchLastMessageUIDs(config, mailboxes);

    // Assert
    QCOMPARE(result.errorMessage().isEmpty(), expectedResultSuccess);
    QCOMPARE(result.data(), expectedResultLastMessageUIDs);
}

QTEST_GUILESS_MAIN(BaseMailClientQtTest)

#include "tst_basemailclient.moc"

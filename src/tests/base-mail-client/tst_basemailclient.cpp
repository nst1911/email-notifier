#include "testbasemailclient.h"
#include "examples.h"
#include <QtTest>

class BaseMailClientQtTest : public QObject
{
    Q_OBJECT

private slots:
    void setConfiguration_data();
    void setConfiguration();

    void fetchMailboxes_data();
    void fetchMailboxes();

    void fetchLastMessageUIDs_data();
    void fetchLastMessageUIDs();
};

void BaseMailClientQtTest::setConfiguration_data()
{
    QTest::addColumn<Configuration>("config");
    QTest::addColumn<Configuration>("expectedConfig");

    Configuration happyPathConfig = Examples::validConfig();

    {
        QTest::newRow("Happy path") << happyPathConfig << happyPathConfig;
    }

    {
        Configuration config = happyPathConfig;
        config.host = "";

        QTest::newRow("Invalid host") << config << Configuration{};
    }

    {
        Configuration config = happyPathConfig;
        config.login = "";

        QTest::newRow("Invalid login") << config << Configuration{};
    }

    {
        Configuration config = happyPathConfig;
        config.password = "";

        QTest::newRow("Invalid password") << config << Configuration{};
    }

    {
        Configuration config = happyPathConfig;
        config.mailboxes = {};

        QTest::newRow("Invalid mailboxes") << config << Configuration{};
    }

}

void BaseMailClientQtTest::setConfiguration()
{
    // Assign
    QFETCH(Configuration, config);
    QFETCH(Configuration, expectedConfig);

    // Act
    TestBaseMailClient mailClient;
    mailClient.setConfiguration(config);

    // Assert
    QCOMPARE(expectedConfig, mailClient.configuration());
}

void BaseMailClientQtTest::fetchMailboxes_data()
{
    QTest::addColumn<Configuration>("config");
    QTest::addColumn<bool>("expectedResultSuccess");
    QTest::addColumn<QStringList>("expectedResultMailboxes");

    QTest::newRow("Happy path") << Examples::validConfig() << true << Examples::validMailboxes();
    QTest::newRow("Invalid config") << Configuration() << false << QStringList{};
}

void BaseMailClientQtTest::fetchMailboxes()
{
    // Assign
    QFETCH(Configuration, config);
    QFETCH(bool, expectedResultSuccess);
    QFETCH(QStringList, expectedResultMailboxes);

    TestBaseMailClient mailClient;
    mailClient.setConfiguration(config);
    mailClient.m_fetchMailboxesImpl = Result<QStringList>::success(Examples::validMailboxes());

    // Act
    Result<QStringList> result = mailClient.fetchMailboxes();

    // Assert
    QCOMPARE(result.errorMessage().isEmpty(), expectedResultSuccess);
    QCOMPARE(result.data(), expectedResultMailboxes);
}

void BaseMailClientQtTest::fetchLastMessageUIDs_data()
{
    QTest::addColumn<Configuration>("config");
    QTest::addColumn<bool>("expectedResultSuccess");
    QTest::addColumn<LastMessageUIDs>("expectedResultLastMessageUIDs");

    {
        Configuration config = Examples::validConfig();
        LastMessageUIDs lastMessageUIDs;
        for (const QString &mailbox : config.mailboxes)
        {
            lastMessageUIDs.insert(mailbox, 6);
        }

        QTest::newRow("Happy path") << config << true << lastMessageUIDs;
    }

    {
        Configuration config = Examples::validConfig();
        config.host = "";

        QTest::newRow("Invalid host") << config << false << LastMessageUIDs{};
    }

    {
        Configuration config = Examples::validConfig();
        config.port = 0;

        QTest::newRow("Invalid port") << config << false << LastMessageUIDs{};
    }

    {
        Configuration config = Examples::validConfig();
        config.login = "";

        QTest::newRow("Invalid login") << config << false << LastMessageUIDs{};
    }

    {
        Configuration config = Examples::validConfig();
        config.password = "";

        QTest::newRow("Invalid password") << config << false << LastMessageUIDs{};
    }

    {
        Configuration config = Examples::validConfig();
        config.mailboxes = {};

        QTest::newRow("Invalid mailboxes") << config << false << LastMessageUIDs{};
    }
}

void BaseMailClientQtTest::fetchLastMessageUIDs()
{
    // Assign
    QFETCH(Configuration, config);
    QFETCH(bool, expectedResultSuccess);
    QFETCH(LastMessageUIDs, expectedResultLastMessageUIDs);

    TestBaseMailClient mailClient;
    mailClient.setConfiguration(config);
    mailClient.m_fetchLastMessageUID = Result<quint64>::success(6);;

    // Act
    Result<LastMessageUIDs> result = mailClient.fetchLastMessageUIDs();

    // Assert
    QCOMPARE(result.errorMessage().isEmpty(), expectedResultSuccess);
    QCOMPARE(result.data(), expectedResultLastMessageUIDs);
}

QTEST_GUILESS_MAIN(BaseMailClientQtTest)

#include "tst_basemailclient.moc"

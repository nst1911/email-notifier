#include "commandlineinterface.h"
#include "mockdaemon.h"
#include "mockpasswordinputhandler.h"
#include "examples.h"
#include "raiideleter.h"
#include <QCoreApplication>
#include <QtTest>

Q_DECLARE_METATYPE(CommandLineInterface::ExitCode);

class CommandLineInterfaceQtTest : public QObject
{
    Q_OBJECT

private slots:
    void process_data();
    void process();

    void nullptrModules_data();
    void nullptrModules();

private:
    QStringList setupArgsExample() const;

    MockDaemon::TestData happyPathDaemonTestData() const;
    MockPasswordInputHandler::TestData happyPathPasswordInputHandlerTestData() const;

    QString exitCodeStr(CommandLineInterface::ExitCode code) const; // to make QTest print names of enum instead of integer numbers
};

void CommandLineInterfaceQtTest::process_data()
{
    QTest::addColumn<QStringList>("arguments");
    QTest::addColumn<MockDaemon::TestData>("daemonTestData");
    QTest::addColumn<MockPasswordInputHandler::TestData>("passwordInputHandlerTestData");
    QTest::addColumn<CommandLineInterface::ExitCode>("expectedExitCode");

    {
        QTest::newRow("Happy path (setup)")
            << setupArgsExample()
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QStringList arguments = {
            "email-notifier", "setup",
            "--mailboxes", "INBOX;SomeOtherMailbox",
            "--host", "imap.yandex.ru",
            "--login", "nst1911@yandex.ru",
            "--interval", "5m",
            "--port", "993"
        };

        QTest::newRow("Happy path (setup) - other order of arguments")
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QStringList arguments = {
            "email-notifier", "setup",
            "--host", "imap.yandex.ru",
            "--port", "993",
            "--login", "nst1911@yandex.ru",
            "--interval", "5m"
    };

        QTest::newRow("Happy path (setup) - default mailboxes arg")
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QStringList arguments = {
            "email-notifier", "setup",
            "--host", "imap.yandex.ru",
            "--port", "993",
            "--login", "nst1911@yandex.ru",
            "--mailboxes", "INBOX;SomeOtherMailbox"
        };

        QTest::newRow("Happy path (setup) - default interval arg")
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QStringList arguments = {
            "email-notifier", "setup",
            "--host", "imap.yandex.ru",
            "--login", "nst1911@yandex.ru",
            "--mailboxes", "INBOX;SomeOtherMailbox",
            "--interval", "5m",
        };

        QTest::newRow("Happy path (setup) - default port arg")
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QTest::newRow("Happy path (start)")
            << QStringList{"email-notifier", "start"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QTest::newRow("Happy path (start)")
            << QStringList{"email-notifier", "stop"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QTest::newRow("Happy path (status)")
            << QStringList{"email-notifier", "status"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QTest::newRow("Happy path (fetch-mailboxes)")
            << QStringList{"email-notifier", "fetch-mailboxes"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.isValid = false;

        QTest::newRow("Failed - Daemon is not valid")
            << QStringList{"email-notifier", "start"}
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::InvalidModules;
    }

    {
        MockPasswordInputHandler::TestData testData = happyPathPasswordInputHandlerTestData();
        testData.isValid = false;

        QTest::newRow("Failed - PasswordInputHandler is not valid")
            << QStringList{"email-notifier", "start"}
            << happyPathDaemonTestData()
            << testData
            << CommandLineInterface::InvalidModules;
    }

    {
        QTest::newRow("Failed - Parse arguments error")
            << QStringList{"email-notifier", "setup", "$mSAz", "--test123", "BNbnD"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        QTest::newRow("Failed - No command")
            << QStringList{"email-notifier"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::MissingRequiredCommands;
    }

    {
        QTest::newRow("Failed - Unknown command")
            << QStringList{"email-notifier", "testfffff"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::UnknownCommand;
    }

    {
        QTest::newRow("Failed - No options for setup command")
            << QStringList{"email-notifier", "setup"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        QStringList arguments = {
            "email-notifier", "setup",
            "--port", "993",
            "--login", "nst1911@yandex.ru",
            "--mailboxes", "INBOX;SomeOtherMailbox",
            "--interval", "5m",
        };
        QTest::newRow("Failed - No 'host' arg for setup command")
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        QStringList arguments = {
            "email-notifier", "setup",
            "--host", "imap.yandex.ru",
            "--port", "993",
            "--mailboxes", "INBOX;SomeOtherMailbox",
            "--interval", "5m",
        };
        QTest::newRow("Failed - No 'login' arg for setup command")
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.setup = "error";

        QTest::newRow("Failed - Daemon::setup failed")
            << setupArgsExample()
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.startMonitoring = "error";

        QTest::newRow("Failed - Daemon::startMonitoring failed")
            << QStringList{"email-notifier", "start"}
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.stopMonitoring = "error";

        QTest::newRow("Failed - Daemon::stopMonitoring failed")
            << QStringList{"email-notifier", "stop"}
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.status = Result<IDaemon::Status>::error("error");

        QTest::newRow("Failed - Daemon::status failed")
            << QStringList{"email-notifier", "status"}
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.fetchMailboxes = Result<QStringList>::error("error");

        QTest::newRow("Failed - Daemon::fetchMailboxes failed")
            << QStringList{"email-notifier", "fetch-mailboxes"}
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }
}

void CommandLineInterfaceQtTest::process()
{
    // Assign
    QFETCH(QStringList, arguments);
    QFETCH(MockDaemon::TestData, daemonTestData);
    QFETCH(MockPasswordInputHandler::TestData, passwordInputHandlerTestData);
    QFETCH(CommandLineInterface::ExitCode, expectedExitCode);

    MockDaemon* daemon = new MockDaemon();
    MockPasswordInputHandler *handler = new MockPasswordInputHandler();
    RAIIDeleter deleter({daemon, handler});

    daemon->m_testData = daemonTestData;
    handler->m_testData = passwordInputHandlerTestData;

    CommandLineInterface cli(daemon, handler);

    // Act
    CommandLineInterface::ExitCode exitCode = cli.process(arguments);

    // Assert
    QCOMPARE(exitCodeStr(exitCode), exitCodeStr(expectedExitCode));
}

void CommandLineInterfaceQtTest::nullptrModules_data()
{
    QTest::addColumn<MockDaemon*>("daemon");
    QTest::addColumn<MockPasswordInputHandler*>("handler");

    {
        MockDaemon *daemon = nullptr;
        MockPasswordInputHandler *handler = new MockPasswordInputHandler();

        QTest::newRow("Daemon is nullptr") << daemon << handler;
    }

    {
        MockDaemon *daemon = new MockDaemon();
        MockPasswordInputHandler *handler = nullptr;

        QTest::newRow("PasswordInputHandler is nullptr") << daemon << handler;
    }
}

void CommandLineInterfaceQtTest::nullptrModules()
{
    // Assign
    QFETCH(MockDaemon*, daemon);
    QFETCH(MockPasswordInputHandler*, handler);

    RAIIDeleter deleter({daemon, handler});

    CommandLineInterface cli(daemon, handler);

    // Act
    CommandLineInterface::ExitCode exitCode = cli.process(setupArgsExample());

    // Assert
    QVERIFY(!cli.isValid());
    QCOMPARE(exitCode, CommandLineInterface::InvalidModules);
}

QStringList CommandLineInterfaceQtTest::setupArgsExample() const
{
    return {
        "email-notifier", "setup",
        "--host", "imap.yandex.ru",
        "--port", "993",
        "--login", "nst1911@yandex.ru",
        "--mailboxes", "INBOX;SomeOtherMailbox",
        "--interval", "5m",
    };
}

MockDaemon::TestData CommandLineInterfaceQtTest::happyPathDaemonTestData() const
{
    MockDaemon::TestData testData;
    testData.isValid = true;
    testData.status = Result<IDaemon::Status>::success(Examples::validDaemonStatus());
    testData.setup = "";
    testData.startMonitoring = "";
    testData.stopMonitoring = "";
    testData.fetchMailboxes = Result<QStringList>::success(Examples::validMailboxes());
    return testData;
}

MockPasswordInputHandler::TestData CommandLineInterfaceQtTest::happyPathPasswordInputHandlerTestData() const
{
    MockPasswordInputHandler::TestData testData;
    testData.isValid = true;
    testData.enterPassword = Result<QString>::success("password");
    return testData;
}

QString CommandLineInterfaceQtTest::exitCodeStr(CommandLineInterface::ExitCode code) const
{
    using ExitCode = CommandLineInterface::ExitCode;
    static const QMap<ExitCode, QString> map = {
        { ExitCode::Success, "Success" },
        { ExitCode::InvalidModules, "InvalidModules" },
        { ExitCode::ParseArgumentsError, "ParseArgumentsError" },
        { ExitCode::MissingRequiredCommands, "MissingRequiredCommands" },
        { ExitCode::TooManyCommands, "TooManyCommands" },
        { ExitCode::UnknownCommand, "UnknownCommand" },
        { ExitCode::DaemonError, "DaemonError" },
        { ExitCode::PasswordInputHandlerError, "PasswordInputHandlerError" }
    };
    return map[code];
}

QTEST_GUILESS_MAIN(CommandLineInterfaceQtTest)

#include "tst_commandlineinterface.moc"

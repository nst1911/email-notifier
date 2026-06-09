#include "examples.h"
#include "raiideleter.h"
#include "testcommandlineinterface.h"
#include <QCoreApplication>
#include <QtTest>

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
    QStringList fetchMailboxesArgsExample() const;

    MockDaemon::TestData happyPathDaemonTestData() const;
    MockPasswordInputHandler::TestData happyPathPasswordInputHandlerTestData() const;

    QString exitCodeStr(ICommandLineInterface::ExitCode code) const; // to make QTest print names of enum instead of integer numbers
};

void CommandLineInterfaceQtTest::process_data()
{
    QTest::addColumn<ICommandLineInterface::DaemonServiceStatus>("daemonServiceStatus");
    QTest::addColumn<QStringList>("arguments");
    QTest::addColumn<MockDaemon::TestData>("daemonTestData");
    QTest::addColumn<MockPasswordInputHandler::TestData>("passwordInputHandlerTestData");
    QTest::addColumn<ICommandLineInterface::ExitCode>("expectedExitCode");

    {
        QTest::newRow("Happy path (setup)")
            << ICommandLineInterface::Active
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
            << ICommandLineInterface::Active
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
            << ICommandLineInterface::Active
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
            << ICommandLineInterface::Active
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
            << ICommandLineInterface::Active
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QTest::newRow("Happy path (start)")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "start"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QTest::newRow("Happy path (start)")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "stop"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QTest::newRow("Happy path (status)")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "status"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QTest::newRow("Happy path (fetch-mailboxes)")
            << ICommandLineInterface::Active
            << fetchMailboxesArgsExample()
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::Success;
    }

    {
        QTest::newRow("Failed - Parse arguments error")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "setup", "$mSAz", "--test123", "BNbnD"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        QTest::newRow("Failed - Unknown command")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "testfffff"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::UnknownCommand;
    }

    {
        QTest::newRow("Failed - No options for setup command")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "setup"}
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        QTest::newRow("Failed - No options for fetch-mailboxes command")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "fetch-mailboxes"}
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
            << ICommandLineInterface::Active
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
            << ICommandLineInterface::Active
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        QStringList arguments = {
            "email-notifier", "setup",
            "--host", "imap.yandex.ru",
            "--login", "nst1911@yandex.ru",
            "--interval", "1s",
        };
        QTest::newRow("Failed - 'interval' is less than minimum in setup command")
            << ICommandLineInterface::Active
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        QStringList arguments = {
            "email-notifier", "fetch-mailboxes",
            "--port", "993",
            "--login", "nst1911@yandex.ru"
        };
        QTest::newRow("Failed - No 'host' arg for fetch-mailboxes command")
            << ICommandLineInterface::Active
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        QStringList arguments = {
            "email-notifier", "fetch-mailboxes",
            "--host", "imap.yandex.ru",
            "--port", "993"
        };
        QTest::newRow("Failed - No 'login' arg for fetch-mailboxes command")
            << ICommandLineInterface::Active
            << arguments
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.setup = "error";

        QTest::newRow("Failed - Daemon::setup failed")
            << ICommandLineInterface::Active
            << setupArgsExample()
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.startMonitoring = "error";

        QTest::newRow("Failed - Daemon::startMonitoring failed")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "start"}
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.stopMonitoring = "error";

        QTest::newRow("Failed - Daemon::stopMonitoring failed")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "stop"}
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.fetchMailboxes = Result<QStringList>::error("error");

        QTest::newRow("Failed - Daemon::fetchMailboxes failed")
            << ICommandLineInterface::Active
            << fetchMailboxesArgsExample()
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.writePassword = "error";

        QTest::newRow("Failed - Daemon::writePassword failed when calling setup")
            << ICommandLineInterface::Active
            << setupArgsExample()
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.writePassword = "error";

        QTest::newRow("Failed - Daemon::writePassword failed when calling fetch-mailboxes")
            << ICommandLineInterface::Active
            << fetchMailboxesArgsExample()
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        MockDaemon::TestData testData = happyPathDaemonTestData();
        testData.isValid = false;

        QTest::newRow("Failed - Daemon is not valid")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "start"}
            << testData
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::InvalidModules;
    }

    {
        MockPasswordInputHandler::TestData testData = happyPathPasswordInputHandlerTestData();
        testData.enterPassword = Result<QString>::error("error");

        QTest::newRow("Failed - PasswordInputHandler::enterPassword failed when calling setup")
            << ICommandLineInterface::Active
            << setupArgsExample()
            << happyPathDaemonTestData()
            << testData
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        MockPasswordInputHandler::TestData testData = happyPathPasswordInputHandlerTestData();
        testData.enterPassword = Result<QString>::error("error");

        QTest::newRow("Failed - PasswordInputHandler::enterPassword failed when calling fetch-mailboxes")
            << ICommandLineInterface::Active
            << fetchMailboxesArgsExample()
            << happyPathDaemonTestData()
            << testData
            << CommandLineInterface::ParseArgumentsError;
    }

    {
        MockPasswordInputHandler::TestData testData = happyPathPasswordInputHandlerTestData();
        testData.isValid = false;

        QTest::newRow("Failed - PasswordInputHandler is not valid")
            << ICommandLineInterface::Active
            << QStringList{"email-notifier", "start"}
            << happyPathDaemonTestData()
            << testData
            << CommandLineInterface::InvalidModules;
    }

    {
        QTest::newRow("Failed - daemon service is inactive")
            << ICommandLineInterface::Inactive
            << setupArgsExample()
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

    {
        QTest::newRow("Failed - daemon service activity check timeout")
            << ICommandLineInterface::Timeout
            << setupArgsExample()
            << happyPathDaemonTestData()
            << happyPathPasswordInputHandlerTestData()
            << CommandLineInterface::DaemonError;
    }

}

void CommandLineInterfaceQtTest::process()
{
    // Assign
    QFETCH(ICommandLineInterface::DaemonServiceStatus, daemonServiceStatus);
    QFETCH(QStringList, arguments);
    QFETCH(MockDaemon::TestData, daemonTestData);
    QFETCH(MockPasswordInputHandler::TestData, passwordInputHandlerTestData);
    QFETCH(ICommandLineInterface::ExitCode, expectedExitCode);

    MockDaemon* daemon = new MockDaemon();
    MockPasswordInputHandler *handler = new MockPasswordInputHandler();
    RAIIDeleter deleter({daemon, handler});

    daemon->m_testData = daemonTestData;
    handler->m_testData = passwordInputHandlerTestData;

    TestCommandLineInterface cli(daemon, handler);
    cli.m_isDaemonServiceActive = daemonServiceStatus;

    // Act
    ICommandLineInterface::ExitCode exitCode = cli.process(arguments);

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

    TestCommandLineInterface cli(daemon, handler);

    // Act
    ICommandLineInterface::ExitCode exitCode = cli.process(setupArgsExample());

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

QStringList CommandLineInterfaceQtTest::fetchMailboxesArgsExample() const
{
    return {
        "email-notifier", "fetch-mailboxes",
        "--host", "imap.yandex.ru",
        "--port", "993",
        "--login", "nst1911@yandex.ru"
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
    testData.writePassword = "";
    return testData;
}

MockPasswordInputHandler::TestData CommandLineInterfaceQtTest::happyPathPasswordInputHandlerTestData() const
{
    MockPasswordInputHandler::TestData testData;
    testData.isValid = true;
    testData.enterPassword = Result<QString>::success("password");
    return testData;
}

QString CommandLineInterfaceQtTest::exitCodeStr(ICommandLineInterface::ExitCode code) const
{
    using ExitCode = ICommandLineInterface::ExitCode;
    static const QMap<ExitCode, QString> map = {
        { ExitCode::Success, "Success" },
        { ExitCode::InvalidModules, "InvalidModules" },
        { ExitCode::ParseArgumentsError, "ParseArgumentsError" },
        { ExitCode::TooManyCommands, "TooManyCommands" },
        { ExitCode::UnknownCommand, "UnknownCommand" },
        { ExitCode::DaemonError, "DaemonError" }
    };
    return map[code];
}

QTEST_GUILESS_MAIN(CommandLineInterfaceQtTest)

#include "tst_commandlineinterface.moc"

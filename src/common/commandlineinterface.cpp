#include "commandlineinterface.h"
#include "log.h"
#include "result.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QProcess>
#include <QRegularExpression>

namespace
{
constexpr const char *c_versionOpt = "version";
constexpr const char *c_helpOpt = "help";
constexpr const char *c_hostOpt = "host";
constexpr const char *c_portOpt = "port";
constexpr const char *c_loginOpt = "login";
constexpr const char *c_mailboxesOpt = "mailboxes";
constexpr const char *c_intervalOpt = "interval";

constexpr const char *c_setupCmd = "setup";
constexpr const char *c_startCmd = "start";
constexpr const char *c_stopCmd = "stop";
constexpr const char *c_statusCmd = "status";
constexpr const char *c_fetchMailboxesCmd = "fetch-mailboxes";

const QString c_parseErrorMsg = "Can't parse %1 from arguments.";

const QStringList c_defaultMailboxes = { "INBOX" };
constexpr const quint32 c_minMailRequestIntervalMsg = 60 * 1000; // 1 minute
constexpr const quint32 c_defaultMailRequestIntervalMs = 5 * 60 * 1000; // 5 minutes
constexpr const quint16 c_defaultPort = 993;
}

CommandLineInterface::CommandLineInterface(IDaemon *daemon, IPasswordInputHandler *handler, QObject *parent)
    : ICommandLineInterface(parent), m_daemon(daemon), m_passwordInputHandler(handler)
{
    m_executeCmdMap.insert(c_setupCmd, &CommandLineInterface::executeSetupCmd);
    m_executeCmdMap.insert(c_startCmd, &CommandLineInterface::executeStartCmd);
    m_executeCmdMap.insert(c_stopCmd, &CommandLineInterface::executeStopCmd);
    m_executeCmdMap.insert(c_statusCmd, &CommandLineInterface::executeStatusCmd);
    m_executeCmdMap.insert(c_fetchMailboxesCmd, &CommandLineInterface::executeFetchMailboxesCmd);
}

CommandLineInterface::ExitCode CommandLineInterface::process(const QStringList &arguments)
{
    static const QString helpMessage =
        "email-notifier daemon control utility.\n\n"
        "[Main options]\n"
        "  --help\n"
        "  --version\n\n"
        "[Commands]\n"
        "  setup             Setup daemon configuration\n"
        "    --host            IMAP server hostname\n"
        "    --port            IMAP server port (1-65535). Optional argument (if not set, default value is 993)\n"
        "    --login           Email account login\n"
        "    --mailboxes       Semicolon-separated list of mailboxes to monitor. Optional argument (if not set, default value is INBOX).\n"
        "    --interval        Mail check interval. Example: 1m30s, 5m, 1h, 3h20m. Optional argument (if not set, default value is 5m)\n"
        "  start             Activate daemon - start monitoring new mail. Daemon needs to be set up before calling it.\n"
        "  stop              Deactivate daemon - stop monitoring new mail\n"
        "  status            Get current status of daemon\n"
        "  fetch-mailboxes   Fetch available mailboxes at email account.\n"
        "    --host            IMAP server hostname\n"
        "    --port            IMAP server port (1-65535). Optional argument (if not set, default value is 993)\n"
        "    --login           Email account login\n\n"
        "[Usage]\n"
        "  email-notifier --help\n"
        "  email-notifier --version\n"
        "  email-notifier setup --host=\"imap.yandex.ru\" --login=\"nst1911@yandex.ru\"\n"
        "  email-notifier setup --host=\"imap.yandex.ru\" --port=993 --login=\"nst1911@yandex.ru\" --mailboxes=\"INBOX;SomeOtherMailbox\" --interval=\"3h20m\"\n"
        "  email-notifier start\n"
        "  email-notifier stop\n"
        "  email-notifier status\n"
        "  email-notifier fetch-mailboxes --host=\"imap.yandex.ru\" --login=\"nst1911@yandex.ru\"\n"
        "  email-notifier fetch-mailboxes --host=\"imap.yandex.ru\" --port=993 --login=\"nst1911@yandex.ru\"\n\n"
        "[Uninstallation]\n"
        "Run 'email-notifier-uninstaller' to uninstall the program.\n"
    ;

    static const QList<QCommandLineOption> options = {
        { { QString(c_helpOpt) }, "Show help" },
        { { QString(c_versionOpt) }, "Show version" },
        { { QString(c_hostOpt) }, "Email server hostname", c_hostOpt },
        { { QString(c_portOpt) }, "Email server port (1-65535)", c_portOpt },
        { { QString(c_loginOpt) }, "Email account login", c_loginOpt },
        { { QString(c_mailboxesOpt) }, "Semicolon-separated list of mailboxes to monitor", c_mailboxesOpt },
        { { QString(c_intervalOpt) }, "Check interval in minutes (positive integer)", c_intervalOpt }
    };
    for (const QCommandLineOption &opt : options)
    {
        m_parser.addOption(opt);
    }
    m_parser.addPositionalArgument("command", "Command to execute");

    if (!m_parser.parse(arguments))
    {
        logCritical() << m_parser.errorText();
        return ParseArgumentsError;
    }

    const QStringList positionalArgs = m_parser.positionalArguments();
    if (positionalArgs.size() < 1)
    {
        if (m_parser.isSet(c_versionOpt))
        {
            logInfo() << PROGRAM_VERSION_STRING_LITERAL;
            return Success;
        }
        else
        {
            logInfo() << helpMessage;
            return Success;
        }
    }

    switch (isDaemonServiceActive())
    {
    case Timeout:
        logCritical() << QString("Unexpected error: Checking if daemon service \"%1\" is alive timeout.").arg(SERVICE_NAME_STRING_LITERAL);
        return DaemonError;
    case Inactive:
        logCritical() << QString("Daemon service \"%1\" is inactive. You need to start the service by calling").arg(SERVICE_NAME_STRING_LITERAL);
        logCritical() << QString("  systemctl --user start %1").arg(SERVICE_NAME_STRING_LITERAL);
        return DaemonError;
    default:
        break;
    }

    if (!isValid())
    {
        logCritical() << "One of modules (daemon, password input handler) is invalid";
        return InvalidModules;
    }

    QString command = positionalArgs.first().toLower();

    ExecuteCmdMethod executeCmd = m_executeCmdMap.value(command, nullptr);
    if (!executeCmd)
    {
        logCritical() << QString("Unknown command \"%1\"").arg(command);
        return UnknownCommand;
    }

    return (this->*executeCmd)();
}

bool CommandLineInterface::isValid() const
{
    return (m_daemon && m_passwordInputHandler) &&
           (m_daemon->isValid() && m_passwordInputHandler->isValid());
}

ICommandLineInterface::DaemonServiceStatus CommandLineInterface::isDaemonServiceActive() const
{
    QProcess process;

    process.start("systemctl", QStringList() << "is-active" << "--user" << SERVICE_NAME_STRING_LITERAL);
    if (!process.waitForFinished(3000))
    {
        return Timeout;
    }

    return process.exitCode() == 0 ? Active : Inactive;
}

Result<quint32> CommandLineInterface::convertStringToMs(const QString &interval) const
{
    static const Result<quint32> convertErr = Result<quint32>::error("Invalid format. Valid examples: 1m30s, 5m, 1h, 3h20m");

    if (interval.isEmpty())
    {
        return convertErr;
    }

    // Pattern: digits followed by unit (s, m, h)
    static const QRegularExpression re("^(\\d+)\\s*([smh])$", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(interval);
    if (!match.hasMatch())
    {
        return convertErr;
    }

    bool ok;
    quint32 value = match.captured(1).toUInt(&ok);
    if (!ok || value == 0)
    {
        return convertErr;
    }

    QString unit = match.captured(2).toLower();
    if (unit.isEmpty())
    {
        return convertErr;
    }

    quint32 intervalMs = 0;
    if (unit == "s")
    {
        intervalMs = value * 1000;
    }
    else if (unit == "m")
    {
        intervalMs = value * 60 * 1000;
    }
    else if (unit == "h")
    {
        intervalMs = value * 60 * 60 * 1000;
    }
    else
    {
        return convertErr;
    }

    return Result<quint32>::success(intervalMs);
}

QString CommandLineInterface::convertMsToString(quint32 ms) const
{
    qint64 totalSeconds = ms / 1000;
    qint64 hours = totalSeconds / 3600;
    qint64 minutes = (totalSeconds % 3600) / 60;
    qint64 seconds = totalSeconds % 60;

    QStringList parts;

    if (hours > 0)
    {
        parts.append(QString::number(hours) + "h");
    }
    if (minutes > 0)
    {
        parts.append(QString::number(minutes) + "m");
    }
    if (seconds > 0)
    {
        parts.append(QString::number(seconds) + "s");
    }

    return parts.join("");
}

Result<IMailClient::Configuration> CommandLineInterface::parseMailClientConfig()
{
    QString host = m_parser.value(c_hostOpt);
    if (host.isEmpty())
    {
        return Result<IMailClient::Configuration>::error(c_parseErrorMsg.arg(c_hostOpt));
    }

    bool ok = false;
    quint16 port = 0;
    if (m_parser.isSet(c_portOpt))
    {
        int portInt = m_parser.value(c_portOpt).toInt(&ok);
        if (!ok || portInt <= 0 || portInt > 65535)
        {
            return Result<IMailClient::Configuration>::error(c_parseErrorMsg.arg(c_portOpt));
        }
        port = portInt;
    }
    else
    {
        port = c_defaultPort;
    }

    QString login = m_parser.value(c_loginOpt);
    if (login.isEmpty())
    {
        return Result<IMailClient::Configuration>::error(c_parseErrorMsg.arg(c_loginOpt));
    }

    logInfo() << "NOTE! You may be asked to enter your Linux account password. It needs to save your email account password to secure keyring.\n";

    Result<QString> passwordResult = m_passwordInputHandler->enterPassword();
    if (!passwordResult.success())
    {
        return Result<IMailClient::Configuration>::error(passwordResult.errorMessage());
    }

    IMailClient::Configuration config;
    config.host = host;
    config.port = port;
    config.login = login;
    config.password = passwordResult.data();

    return Result<IMailClient::Configuration>::success(config);
}

CommandLineInterface::ExitCode CommandLineInterface::executeSetupCmd()
{
    // Parsing --mailboxes from arguments
    QStringList mailboxes;
    if (m_parser.isSet(c_mailboxesOpt))
    {
        mailboxes = m_parser.value(c_mailboxesOpt).split(";", Qt::SkipEmptyParts);
        if (mailboxes.isEmpty())
        {
            logCritical() << c_parseErrorMsg.arg(c_mailboxesOpt);
            return ParseArgumentsError;
        }
    }
    else
    {
        mailboxes = c_defaultMailboxes;
    }

    // Parsing --interval from arguments
    quint32 interval = 0;
    if (m_parser.isSet(c_intervalOpt))
    {
        Result<quint32> intervalResult = convertStringToMs(m_parser.value(c_intervalOpt));
        if (!intervalResult.success())
        {
            logCritical() << c_parseErrorMsg.arg(c_intervalOpt) << intervalResult.errorMessage();
            return ParseArgumentsError;
        }

        if (intervalResult.data() < c_minMailRequestIntervalMsg)
        {
            logCritical() << QString("--%1 should be >= %2").arg(c_intervalOpt, convertMsToString(c_minMailRequestIntervalMsg));
            return ParseArgumentsError;
        }

        interval = intervalResult.data();
    }
    else
    {
        interval = c_defaultMailRequestIntervalMs;
    }

    // Parsing --host, --port, --login from arguments and ask user to enter a password
    Result<IMailClient::Configuration> mailClientResult = parseMailClientConfig();
    if (!mailClientResult.success())
    {
        logCritical() << mailClientResult.errorMessage();
        return ParseArgumentsError;
    }

    // Building configuration
    IDaemon::Configuration config;
    config.mailClient = mailClientResult.data();
    config.mailboxes = mailboxes;
    config.mailRequestIntervalMs = interval;

    // Write password to secure keyring
    if (QString passwordResult = m_daemon->writePassword(IDaemon::DaemonConfiguration, config.mailClient.password); !passwordResult.isEmpty())
    {
        logCritical() << passwordResult;
        return DaemonError;
    }

    // Calling Daemon's setup via D-Bus
    QString dbusCallErr = m_daemon->setup(config);
    if (!dbusCallErr.isEmpty())
    {
        logCritical() << dbusCallErr;
        return DaemonError;
    }

    logInfo() << "\nSettings have been applied successfully.";
    return Success;
}

CommandLineInterface::ExitCode CommandLineInterface::executeStartCmd()
{
    QString dbusCallErr = m_daemon->startMonitoring();
    if (!dbusCallErr.isEmpty())
    {
        logCritical() << dbusCallErr;
        return DaemonError;
    }

    logInfo() << "Monitoring has been started successfully.";
    return Success;
}

CommandLineInterface::ExitCode CommandLineInterface::executeStopCmd()
{
    QString stopMonitoringResult = m_daemon->stopMonitoring();
    if (!stopMonitoringResult.isEmpty())
    {
        logCritical() << stopMonitoringResult;
        return DaemonError;
    }

    logInfo() << "Monitoring has been stopped.";
    return Success;
}

CommandLineInterface::ExitCode CommandLineInterface::executeStatusCmd()
{
    // Calling Daemon's status via D-Bus
    Result<IDaemon::Status> statusResult = m_daemon->status();
    if (!statusResult.success())
    {
        logCritical() << statusResult.errorMessage() << "\n";
    }

    // Print status output
    const QString invalidField = "<invalid>";

    IDaemon::Configuration config = statusResult.data().configuration;
    QString imapServer = (!config.mailClient.host.isEmpty() && config.mailClient.port != 0)
        ? QString("%1:%2").arg(config.mailClient.host).arg(config.mailClient.port)
        : invalidField;
    QString login = !config.mailClient.login.isEmpty() ? config.mailClient.login : invalidField;
    QStringList mailboxes = !config.mailboxes.isEmpty() ? config.mailboxes : c_defaultMailboxes;
    quint32 interval = config.mailRequestIntervalMs != 0 ? config.mailRequestIntervalMs : c_defaultMailRequestIntervalMs;
    QString configurationMsg = QString(
        "  [Configuration]\n"
        "    IMAP server:              %1\n"
        "    Login:                    %2\n"
        "    Mailboxes:                %3\n"
        "    Mail check interval:      %4"
    ).arg(imapServer, login, mailboxes.join(", "), convertMsToString(interval));

    QString activatedMsg = QString(
        "  [Monitoring]\n"
        "    %1"
    ).arg(statusResult.data().isMonitoringActivated ? "Activated" : "Not activated");

    LogMessage lastError = statusResult.data().lastError;
    QString lastErrorMsg;
    if (lastError.isValid() && lastError.type == LogMessage::Error)
    {
        lastErrorMsg = QString(
            "  [Last error message]\n"
            "    Timestamp:  %1\n"
            "    Message:    %2"
        ).arg(lastError.timestamp.toString(Qt::ISODateWithMs), lastError.message);
    }

    QString statusMsg = QString("%1\n%2").arg(configurationMsg, activatedMsg);
    if (!lastErrorMsg.isEmpty())
    {
        statusMsg.append(QString("\n%3").arg(lastErrorMsg));
    }

    logInfo() << statusMsg;
    return Success;
}

CommandLineInterface::ExitCode CommandLineInterface::executeFetchMailboxesCmd()
{
    // Parsing --host, --port, --login from arguments and ask user to enter a password
    Result<IMailClient::Configuration> mailClientResult = parseMailClientConfig();
    if (!mailClientResult.success())
    {
        logCritical() << mailClientResult.errorMessage();
        return ParseArgumentsError;
    } 
    IMailClient::Configuration config = mailClientResult.data();

    // Write password to secure keyring
    if (QString passwordResult = m_daemon->writePassword(IDaemon::FetchMailboxes, config.password); !passwordResult.isEmpty())
    {
        logCritical() << passwordResult;
        return DaemonError;
    }

    // Calling Daemon's setup via D-Bus
    Result<QStringList> fetchMailboxesResult = m_daemon->fetchMailboxes(config);
    if (!fetchMailboxesResult.success())
    {
        logCritical() << fetchMailboxesResult.errorMessage();
        return DaemonError;
    }

    // Print fetchMailboxes result
    QStringList mailboxes = fetchMailboxesResult.data();
    if (mailboxes.isEmpty())
    {
        logInfo() << "\nNo available mailboxes.";
    }
    else
    {
        logInfo() << "\nAvailable mailboxes:";
        for (const QString &mailbox : fetchMailboxesResult.data())
        {
            logInfo() << "  " + mailbox;
        }
    }

    return Success;
}

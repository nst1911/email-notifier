#include "commandlineinterface.h"
#include "configuration.h"
#include "log.h"
#include "result.h"
#include <QCommandLineParser>
#include <QCoreApplication>
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

const QString c_parseErrorMsg = "Error: \"Can't parse %1 from arguments.\"";

const QStringList c_defaultMailboxes = { "INBOX" };
constexpr const quint32 c_defaultMailRequestIntervalMs = 5 * 60 * 1000; // 5 minutes
constexpr const quint16 c_defaultPort = 993;

} // namespace

CommandLineInterface::CommandLineInterface(IDaemon *daemon, IPasswordInputHandler *handler, QObject *parent)
    : IModule(parent), m_daemon(daemon), m_passwordInputHandler(handler)
{
    m_executeCmdMap.insert(c_setupCmd, &CommandLineInterface::executeSetupCmd);
    m_executeCmdMap.insert(c_startCmd, &CommandLineInterface::executeStartCmd);
    m_executeCmdMap.insert(c_stopCmd, &CommandLineInterface::executeStopCmd);
    m_executeCmdMap.insert(c_statusCmd, &CommandLineInterface::executeStatusCmd);
    m_executeCmdMap.insert(c_fetchMailboxesCmd, &CommandLineInterface::executeFetchMailboxesCmd);
}

CommandLineInterface::ExitCode CommandLineInterface::process(const QStringList &arguments)
{
    if (!isValid())
    {
        logCritical() << "CommandLineInterface is invalid";
        return InvalidModules;
    }

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
        "    --interval        Mail check interval. Example: 30s, 5m, 1h, 3h20min. Optional argument (if not set, default value is 5m)\n"
        "  start             Activate daemon - start monitoring new mail. Daemon needs to be set up before calling it.\n"
        "  stop              Deactivate daemon - stop monitoring new mail\n"
        "  status            Get current status of daemon\n"
        "  fetch-mailboxes   Fetch available mailboxes from email account. Daemon needs to be set up before calling it.\n\n"
        "[Usage]\n"
        "  email-notifier setup --host=\"imap.yandex.ru\" --login=\"nst1911@yandex.ru\"\n"
        "  email-notifier setup --host=\"imap.yandex.ru\" --port=993 --login=\"nst1911@yandex.ru\" --mailboxes=\"INBOX;SomeOtherMailbox\" --interval=\"3h20m\"\n"
        "  email-notifier start\n"
        "  email-notifier stop\n"
        "  email-notifier status\n"
        "  email-notifier fetch-mailboxes\n\n"
        "[Uninstallation]\n"
        "Run 'email-notifier-uninstaller' to uninstall the program.\n"
    ;

    QCommandLineParser parser;

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
        parser.addOption(opt);
    }
    parser.addPositionalArgument("command", "Command to execute");

    if (!parser.parse(arguments))
    {
        logCritical() << parser.errorText();
        return ParseArgumentsError;
    }

    const QStringList positionalArgs = parser.positionalArguments();
    if (positionalArgs.size() < 1)
    {
        if (parser.isSet(c_versionOpt))
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

    QString command = positionalArgs.first().toLower();

    ExecuteCmdFunc executeCmd = m_executeCmdMap.value(command, nullptr);
    if (!executeCmd)
    {
        logCritical() << QString("Unknown command \"%1\"").arg(command);
        return UnknownCommand;
    }

    return (this->*executeCmd)(parser);
}

bool CommandLineInterface::isValid() const
{
    if (!m_daemon || !m_passwordInputHandler)
    {
        return false;
    }
    if (!m_daemon->isValid() || !m_passwordInputHandler->isValid())
    {
        return false;
    }
    return true;
}

Result<quint32> CommandLineInterface::convertStringToMs(const QString &interval) const
{
    static const Result<quint32> convertErr = Result<quint32>::error("Invalid format (must be Ns, Nm, Nh where N - integer > 0).");

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
    if (ms < 0)
    {
        return "0s";
    }

    qint64 totalSeconds = ms / 1000;
    qint64 hours = totalSeconds / 3600;
    qint64 minutes = (totalSeconds % 3600) / 60;
    qint64 seconds = totalSeconds % 60;

    QStringList parts;

    if (hours > 0)
    {
        parts << QString::number(hours) + "h";
    }

    if (minutes > 0 || hours > 0)
    {
        parts << QString::number(minutes) + "m";
    }

    parts << QString::number(seconds) + "s";

    return parts.join("");
}

CommandLineInterface::ExitCode CommandLineInterface::executeSetupCmd(QCommandLineParser &parser)
{
    if (!parser.isSet(c_hostOpt) || !parser.isSet(c_loginOpt))
    {
        logCritical() << QString("No arguments (--%1, --%2) specified").arg(c_hostOpt, c_loginOpt);
        return ParseArgumentsError;
    }

    static const auto parseError = [](const QString &opt) {
        logCritical() << c_parseErrorMsg.arg(opt);
        return ParseArgumentsError;
    };

    bool ok = false;

    // Parsing '--host' from arguments
    QString host = parser.value(c_hostOpt);
    if (host.isEmpty())
    {
        return parseError(c_hostOpt);
    }

    // Parsing '--port' from arguments 
    quint16 port = 0;
    if (parser.isSet(c_portOpt))
    {
        int portInt = parser.value(c_portOpt).toInt(&ok);
        if (!ok || portInt <= 0 || portInt > 65535)
        {
            return parseError(c_portOpt);
        }
        port = portInt;
    }
    else
    {
        port = c_defaultPort;
    }

    // Parsing '--login' from arguments
    QString login = parser.value(c_loginOpt);
    if (login.isEmpty())
    {
        return parseError(c_loginOpt);
    }

    // Parsing '--mailboxes' from arguments
    QStringList mailboxes;
    if (parser.isSet(c_mailboxesOpt))
    {
        mailboxes = parser.value(c_mailboxesOpt).split(";", Qt::SkipEmptyParts);
        if (mailboxes.isEmpty())
        {
            return parseError(c_mailboxesOpt);
        }
    }
    else
    {
        mailboxes = c_defaultMailboxes;
    }

    // Parsing '--interval' from arguments
    quint32 interval = 0;
    if (parser.isSet(c_intervalOpt))
    {
        Result<quint32> intervalResult = convertStringToMs(parser.value(c_intervalOpt));
        if (!intervalResult.success())
        {
            logCritical() << QString("%1 (%2)").arg(c_parseErrorMsg.arg(c_intervalOpt)).arg(intervalResult.errorMessage());
            return ParseArgumentsError;
        }
        interval = intervalResult.data();
    }
    else
    {
        interval = c_defaultMailRequestIntervalMs;
    }

    // Read password (entered by user)
    Result<QString> passwordResult = m_passwordInputHandler->enterPassword();
    if (!passwordResult.success())
    {
        logCritical() << passwordResult.errorMessage();
        return PasswordInputHandlerError;
    }

    // Building configuration and call Daemon's setup() via DBus
    Configuration config;
    config.host = host;
    config.port = (quint16)port;
    config.login = login;
    config.password = passwordResult.data();
    config.mailboxes = mailboxes;
    config.mailRequestIntervalMs = interval;

    QString dbusCallErr = m_daemon->setup(config);
    if (!dbusCallErr.isEmpty())
    {
        logCritical() << dbusCallErr;
        return DaemonError;
    }

    logInfo() << "\nSettings have been applied successfully.";
    return Success;
}

CommandLineInterface::ExitCode CommandLineInterface::executeStartCmd(QCommandLineParser &parser)
{
    Q_UNUSED(parser);

    QString dbusCallErr = m_daemon->startMonitoring();
    if (!dbusCallErr.isEmpty())
    {
        logCritical() << dbusCallErr;
        return DaemonError;
    }

    logInfo() << "Monitoring has been started successfully.";
    return Success;
}

CommandLineInterface::ExitCode CommandLineInterface::executeStopCmd(QCommandLineParser &parser)
{
    Q_UNUSED(parser);

    QString stopMonitoringResult = m_daemon->stopMonitoring();
    if (!stopMonitoringResult.isEmpty())
    {
        logCritical() << stopMonitoringResult;
        return DaemonError;
    }

    logInfo() << "Monitoring has been stopped";
    return Success;
}

CommandLineInterface::ExitCode CommandLineInterface::executeStatusCmd(QCommandLineParser &parser)
{
    Q_UNUSED(parser);

    Result<IDaemon::Status> statusResult = m_daemon->status();
    if (!statusResult.success())
    {
        logCritical() << statusResult.errorMessage() << "\n";
    }

    const QString invalidField = "<invalid>";

    Configuration config = statusResult.data().configuration;
    QString imapServer = (!config.host.isEmpty() && config.port != 0)
        ? QString("%1:%2").arg(config.host).arg(config.port)
        : invalidField;
    QString login = !config.login.isEmpty() ? config.login : invalidField;
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

    Message lastError = statusResult.data().lastError;
    QString lastErrorMsg;
    if (lastError.isValid() && lastError.type == Message::Error)
    {
        lastErrorMsg = QString(
            "  [Last error message]\n"
            "    Timestamp:  %1\n"
            "    Message:    %2"
        ).arg(lastError.timestamp.toString(Qt::ISODateWithMs), lastError.message);
    }

    QString statusMsg = QString("%1\n%2").arg(configurationMsg).arg(activatedMsg);
    if (!lastErrorMsg.isEmpty())
    {
        statusMsg.append(QString("\n%3").arg(lastErrorMsg));
    }

    logInfo() << statusMsg;
    return Success;
}

CommandLineInterface::ExitCode CommandLineInterface::executeFetchMailboxesCmd(QCommandLineParser &parser)
{
    Q_UNUSED(parser);

    Result<QStringList> fetchMailboxesResult = m_daemon->fetchMailboxes();
    if (!fetchMailboxesResult.success())
    {
        logCritical() << fetchMailboxesResult.errorMessage();
        return DaemonError;
    }

    QString fetchMailboxesMsg = QString("Available mailboxes:\n");
    for (const QString &mailbox : fetchMailboxesResult.data())
    {
        fetchMailboxesMsg.append(QString("  %1\n").arg(mailbox));
    }

    logInfo() << fetchMailboxesMsg;

    return Success;
}

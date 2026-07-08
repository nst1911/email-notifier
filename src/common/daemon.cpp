#include "daemon.h"
#include "log.h"
#include <QDBusConnectionInterface>
#include <QDBusInterface>

namespace
{

constexpr const char *c_dbusServiceName = "com.github.emailnotifier";
constexpr const char *c_dbusObjectPath = "com/github/emailnotifier";

constexpr const char *c_startMonitoringErrorMsg = "Monitoring not started:";
constexpr const char *c_mailRequestErrorMsg = "Mail request failed:";
constexpr const char *c_fetchingMailboxesErrorMsg = "Fetching mailboxes failed:";
constexpr const char *c_nullptrMsg = "One or more modules are nullptr";
constexpr const char *c_setupErrorMsg = "Setting up daemon failed:";

const QString c_readFromStorageFailedErrorMsg = "Reading '%1' from storage failed. Error: %2"; // clazy:skip
const QString c_writeToStorageFailedErrorMsg = "Writing '%1' to storage failed. Error: %2"; // clazy:skip
const QString c_invalidModuleMsg = "%1 is not valid"; // clazy:skip

}

Daemon::Daemon(IMailClient *mailClient, IPersistentStorage *storage, INotificationManager *manager, QObject *parent)
    : IDaemon(parent),
      m_mailClient(mailClient),
      m_storage(storage),
      m_notificationMngr(manager)
{
    m_mailRequestTimer = new QTimer(this);
    connect(m_mailRequestTimer, &QTimer::timeout, this, [this]() {
        onMailRequestTimerTimeout(true);
    });

    connect(this, &Daemon::sendNotification, this, &Daemon::onSendNotification);

    if (checkModulesNullptr())
    {
        logCritical() << c_nullptrMsg;
        return;
    }

    Result<Configuration> result = m_storage->readDaemonConfiguration();
    if (result.success())
    {
        setup(result.data(), WhileConstructingDaemon);
    }
    else
    {
        logWarning() << result.errorMessage();
    }
}

Daemon::~Daemon()
{
    m_mailRequestTimer->stop();
}

void Daemon::registerDBusService()
{
    registerDBusMetaTypes();

    auto connection = QDBusConnection::sessionBus();
    if (connection.interface()->isServiceRegistered(DAEMON_DBUS_SERVICE_NAME))
    {
        logCritical() << QString("D-Bus service already registered: %1").arg(DAEMON_DBUS_SERVICE_NAME);
        return;
    }

    if (!connection.registerObject(DAEMON_DBUS_OBJECT_PATH, this, QDBusConnection::ExportAllSlots))
    {
        logCritical() << QString("Cannot register D-Bus object %1. Error: %2").arg(DAEMON_DBUS_OBJECT_PATH, connection.lastError().message());
        return;
    }

    if (!connection.registerService(DAEMON_DBUS_SERVICE_NAME))
    {
        logCritical() << QString("Cannot register D-Bus service: %1. Error: %2").arg(DAEMON_DBUS_SERVICE_NAME, connection.lastError().message());
        return;
    }

    logInfo() << QString("D-Bus service registered: %1").arg(DAEMON_DBUS_SERVICE_NAME);
}

IMailClient *Daemon::mailClient() const
{
    return m_mailClient;
}

IPersistentStorage *Daemon::persistentStorage() const
{
    return m_storage;
}

INotificationManager *Daemon::notificationManager() const
{
    return m_notificationMngr;
}

bool Daemon::isValid() const
{
    return !checkModulesNullptr();
}

bool Daemon::isSetUp() const
{
    return m_config.isValid();
}

Result<IDaemon::Status> Daemon::status() const
{
    if (checkModulesNullptr())
    {
        return Result<Status>::error(c_nullptrMsg);
    }
    if (!m_storage->isValid())
    {
        return Result<Status>::error(c_invalidModuleMsg.arg("Persistent storage"));
    }

    QStringList errors;

    Configuration config;
    Result<Configuration> configResult = m_storage->readDaemonConfiguration();
    if (!configResult.success())
    {
        errors.append(c_readFromStorageFailedErrorMsg.arg("configuration", configResult.errorMessage()));
    }
    else
    {
        config = configResult.data();
    }

    LogMessage lastError;
    Result<LogMessage> lastErrorResult = m_storage->readErrorLogMessage();
    if (!lastErrorResult.success())
    {
        errors.append(c_readFromStorageFailedErrorMsg.arg("lastError", lastErrorResult.errorMessage()));
    }
    else
    {
        lastError = lastErrorResult.data();
    }

    Status status;
    status.isMonitoringActivated = isMonitoringActivated();
    status.configuration = config;
    status.lastError = lastError;

    if (errors.isEmpty())
    {
        return Result<Status>::success(status);
    }

    return Result<Status>::error(errors.join("; "), status);
}

QString Daemon::startMonitoring()
{
    logInfo() << "Starting monitoring...";

    QString result;
    auto scopeGuard = qScopeGuard([this, &result]() {
        if (!result.isEmpty())
        {
            writeErrorLogMessage("startMonitoring", result);
        }
    });

    m_mailRequestTimer->stop();

    if (checkModulesNullptr())
    {
        result = c_nullptrMsg;
        return result;
    }

    if (!m_storage->isValid())
    {
        result = c_invalidModuleMsg.arg("Persistent storage");
        return result;
    }

    if (!m_mailClient->isValid())
    {
        result = c_invalidModuleMsg.arg("Mail client");
        return result;
    }

    if (!m_notificationMngr->isValid())
    {
        result = c_invalidModuleMsg.arg("Notification manager");
        return result;
    }

    if (!isSetUp())
    {
        result = "Daemon is not set up.";
        return result;
    }

    if (m_mailRequestTimer->interval() == 0)
    {
        result = "Invalid mail request interval.";
        return result;
    }

    QString mailRequestResult = onMailRequestTimerTimeout(false);
    if (!mailRequestResult.isEmpty())
    {
        result = mailRequestResult;
        return result;
    }

    logInfo() << "Monitoring started successfully.";
    m_mailRequestTimer->start();

    result = "";
    return result;
}

QString Daemon::stopMonitoring()
{
    m_mailRequestTimer->stop();
    return "";
}

QString Daemon::setup(const IDaemon::Configuration &config)
{
    return setup(config, Default);
}

Result<QStringList> Daemon::fetchMailboxes(const IMailClient::Configuration &config)
{
    Result<QStringList> result;
    auto scopeGuard = qScopeGuard([this, &result]() {
        if (!result.success())
        {
            writeErrorLogMessage("fetchMailboxes", result.errorMessage());
        }
    });

    if (checkModulesNullptr())
    {
        result = Result<QStringList>::error(c_nullptrMsg);
        return result;
    }

    Result<QString> passwordResult = m_storage->readPassword(FetchMailboxes);
    if (!passwordResult.success())
    {
        result = Result<QStringList>::error(c_readFromStorageFailedErrorMsg.arg("password", passwordResult.errorMessage()));
        return result;
    }

    IMailClient::Configuration configWithPassword = config; // have to create it because config is const&
    configWithPassword.password = passwordResult.data();
    if (!configWithPassword.isValid())
    {
        result = Result<QStringList>::error("Configuration is invalid.");
        return result;
    }

    logInfo() << QString("Fetching mailboxes from %1:%2 at user %3...").arg(configWithPassword.host).arg(configWithPassword.port).arg(configWithPassword.login);

    Result<QStringList> fetchResult = m_mailClient->fetchMailboxes(configWithPassword);
    if (!fetchResult.success())
    {
        result = Result<QStringList>::error(fetchResult.errorMessage());
        return result;
    }

    result = Result<QStringList>::success(fetchResult.data());
    return result;
}

QString Daemon::writePassword(PasswordType type, const QString &password)
{
    QString result;
    auto scopeGuard = qScopeGuard([this, &result]() {
        if (!result.isEmpty())
        {
            writeErrorLogMessage("writePassword", result);
        }
    });

    if (checkModulesNullptr())
    {
        result = c_nullptrMsg;
        return result;
    }

    if (!m_storage->isValid())
    {
        result = c_invalidModuleMsg.arg("Persistent storage");
        return result;
    }

    result = m_storage->writePassword(type, password);
    return result;
}

QString Daemon::onMailRequestTimerTimeout(bool writeErrMsg)
{
    Notification notification;
    auto createErrorNotification = [](const QString &msg) {
        Notification notification;
        notification.summary = "Error occured";
        notification.body = msg;
        notification.urgency = Notification::Error;
        return notification;
    };

    auto scopeGuard = qScopeGuard([this, &notification, &writeErrMsg] {
        if (notification.isValid())
        {
            bool success = notification.urgency != Notification::Error;
            if (!success)
            {
                if (writeErrMsg)
                {
                    writeErrorLogMessage("onMailRequestTimerTimeout", notification.body);
                }
                m_mailRequestTimer->stop();
            }
            emit sendNotification(success, notification);
        }
        emit mailRequestFinished();
    });

    logInfo() << "Performing mail request...";

    if (checkModulesNullptr())
    {
        notification = createErrorNotification(QString("%1 \"%2\"").arg(c_mailRequestErrorMsg, c_nullptrMsg));
        return notification.body;
    }

    // Fetch last message UIDs from mail server

    Result<MessageInfoMap> fetchLastMessageInfoResult = m_mailClient->fetchLastMessageInfo(m_config.mailClient, m_config.mailboxes);
    if (!fetchLastMessageInfoResult.success())
    {
        const QString msg = QString("%1 \"%2\"")
            .arg(c_mailRequestErrorMsg)
            .arg(QString("Fetching lastMessageInfo failed. Error: %1").arg(fetchLastMessageInfoResult.errorMessage()));
        notification = createErrorNotification(msg);
        return notification.body;
    }

    Result<MessageInfoMap> readLastMessageInfoResult = m_storage->readLastMessageInfo();
    if (!readLastMessageInfoResult.success())
    {
        const QString msg = c_readFromStorageFailedErrorMsg.arg("lastMessageInfo", readLastMessageInfoResult.errorMessage());
        notification = createErrorNotification(msg);
        return notification.body;
    }

    MessageInfoMap oldLastMessageInfoMap = readLastMessageInfoResult.data();
    MessageInfoMap newLastMessageInfoMap = fetchLastMessageInfoResult.data();

    logDebug() << "oldLastMessageInfoMap =" << oldLastMessageInfoMap << "; newLastMessageInfoMap =" << newLastMessageInfoMap;

    if (QString writeLastMessageInfoResult = m_storage->writeLastMessageInfo(newLastMessageInfoMap); !writeLastMessageInfoResult.isEmpty())
    {
        const QString msg = QString("%1 \"%2\"")
            .arg(c_mailRequestErrorMsg)
            .arg(c_writeToStorageFailedErrorMsg.arg("lastMessageInfo", writeLastMessageInfoResult));
        notification = createErrorNotification(msg);
        return notification.body;
    }

    if (oldLastMessageInfoMap.isEmpty())
    {
        logInfo() << "Mail request finished: \"New messages not found (first request after setting up daemon)\"";
        return "";
    }

    QStringList mailboxesWithUpdates = compareLastMessageInfo(oldLastMessageInfoMap, newLastMessageInfoMap);
    if (mailboxesWithUpdates.isEmpty())
    {
        logInfo() << "Mail request finished: \"New messages not found\"";
        return "";
    }

    logInfo() << "Mail request finished: \"Found new message(s)\"";

    notification.summary = "New message(s) have been received";
    notification.body = QString("Check \"%1\" mailbox").arg(mailboxesWithUpdates.join(", "));
    notification.urgency = Notification::Info;

    return "";
}

void Daemon::onSendNotification(bool success, const Notification &notification)
{
    Q_UNUSED(success); // need for tests

    QString result;
    auto scopeGuard = qScopeGuard([this, &result]() {
        if (!result.isEmpty())
        {
            writeErrorLogMessage("onSendNotification", result);
        }
    });

    if (!m_notificationMngr->sendNotification(notification))
    {
        result = "Sending notification failed";
        logCritical() << result;
    }
}

QString Daemon::setup(const IDaemon::Configuration &config, SetupMode mode)
{
    QString result;
    auto scopeGuard = qScopeGuard([this, &result, &mode]() {
        if (!result.isEmpty() && mode == Default)
        {
            writeErrorLogMessage("setup", result);
        }
    });

    if (checkModulesNullptr())
    {
        result = c_nullptrMsg;
        return result;
    }
    if (isMonitoringActivated())
    {
        result = "Monitoring is already activated";
        return result;
    }
    if (!m_storage->isValid())
    {
        result = c_invalidModuleMsg.arg("Persistent storage");
        return result;
    }

    Result<QString> passwordResult = m_storage->readPassword(DaemonConfiguration);
    if (!passwordResult.success())
    {
        result = c_readFromStorageFailedErrorMsg.arg("password", passwordResult.errorMessage());
        return result;
    }

    IDaemon::Configuration configWithPasword = config;
    configWithPasword.mailClient.password = passwordResult.data();
    if (!configWithPasword.isValid())
    {
        result = "Invalid " + configWithPasword.invalidFields().join(", ");
        return result;
    }
    m_config = configWithPasword;

    m_mailRequestTimer->setInterval(m_config.mailRequestIntervalMs);

    if (mode == Default)
    {
        if (QString configResult = m_storage->writeDaemonConfiguration(m_config); !configResult.isEmpty())
        {
            result = c_writeToStorageFailedErrorMsg.arg("configuration", configResult);
            return result;
        }

        // Clear last message info
        if (QString writeLastMessageInfoResult = m_storage->writeLastMessageInfo({}); !writeLastMessageInfoResult.isEmpty())
        {
            result = c_writeToStorageFailedErrorMsg.arg("lastMessageInfo", writeLastMessageInfoResult);
            return result;
        }
    }

    logInfo() << "Daemon is set up.";

    result = "";
    return result;
}

bool Daemon::isMonitoringActivated() const
{
    return m_mailRequestTimer->isActive();
}

void Daemon::writeErrorLogMessage(const QString &funcName, const QString &message)
{
    const QString formattedMsg = QString("(%1) %2").arg(funcName, message);
    logCritical() << formattedMsg;

    if (!m_storage)
    {
        return;
    }
    if (!m_storage->isValid())
    {
        return;
    }

    LogMessage msg;
    msg.message = formattedMsg;
    msg.timestamp = QDateTime::currentDateTime();
    msg.type = LogMessage::Error;

    if (QString result = m_storage->writeErrorLogMessage(msg); !result.isEmpty())
    {
        logCritical() << c_writeToStorageFailedErrorMsg.arg("errorLogMessage", result);
        return;
    }
}

QStringList Daemon::compareLastMessageInfo(const MessageInfoMap &oldMap, const MessageInfoMap &newMap) const
{
    QStringList mailboxes;

    for (auto it = newMap.constBegin(); it != newMap.constEnd(); ++it)
    {
        if (!oldMap.contains(it.key()))
        {
            continue;
        }
        if (it.value() != oldMap.value(it.key()) && !it.value().seen)
        {
            mailboxes.append(it.key());
            break;
        }
    }

    return mailboxes;
}

bool Daemon::checkModulesNullptr() const
{
    return !m_mailClient || !m_storage || !m_notificationMngr;
}

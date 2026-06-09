#include "daemondbusinterface.h"
#include "dbus.h"
#include <QDBusReply>

DaemonDBusInterface::DaemonDBusInterface(QObject *parent)
    : IDaemon(parent)
{
    registerDBusMetaTypes();

    m_daemonDBusInterface = new QDBusInterface(
        DAEMON_DBUS_SERVICE_NAME,
        DAEMON_DBUS_OBJECT_PATH,
        DAEMON_DBUS_INTERFACE_NAME,
        QDBusConnection::sessionBus()
    );
    if (!m_daemonDBusInterface->isValid())
    {
        m_dbusInitError = QString("D-Bus interface \"%1\" is invalid: \"%2\"").arg(DAEMON_DBUS_SERVICE_NAME, m_daemonDBusInterface->lastError().message());
    }
}

bool DaemonDBusInterface::isValid() const
{
    return m_dbusInitError.isEmpty();
}

Result<IDaemon::Status> DaemonDBusInterface::status() const
{
    return callDBusMethod<Result<IDaemon::Status>>("status");
}

QString DaemonDBusInterface::setup(const IDaemon::Configuration &config)
{
    return callDBusMethod<QString>("setup", QVariant::fromValue(config));
}

QString DaemonDBusInterface::startMonitoring()
{
    return callDBusMethod<QString>("startMonitoring");
}

QString DaemonDBusInterface::stopMonitoring()
{
    return callDBusMethod<QString>("stopMonitoring");
}

Result<QStringList> DaemonDBusInterface::fetchMailboxes(const IMailClient::Configuration &config)
{
    return callDBusMethod<Result<QStringList>>("fetchMailboxes", QVariant::fromValue(config));
}

QString DaemonDBusInterface::writePassword(PasswordType type, const QString &password)
{
    return callDBusMethod<QString>("writePassword", QVariant::fromValue(type), QVariant::fromValue(password));
}


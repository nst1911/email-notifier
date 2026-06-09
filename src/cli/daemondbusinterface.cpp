#include "daemondbusinterface.h"
#include "dbus.h"
#include "log.h"
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
        QString msg = QString("D-Bus interface \"%1\" is invalid: \"%2\"").arg(DAEMON_DBUS_SERVICE_NAME, m_daemonDBusInterface->lastError().message());
        logCritical() << msg;
        m_dbusInitError.append(msg);
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

QString DaemonDBusInterface::setup(const Configuration &config)
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

Result<QStringList> DaemonDBusInterface::fetchMailboxes()
{
    return callDBusMethod<Result<QStringList>>("fetchMailboxes");
}




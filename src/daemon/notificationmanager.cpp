#include "log.h"
#include "notificationmanager.h"
#include <QDBusReply>
#include <QThread>

namespace
{
constexpr const char *c_serviceName = "org.freedesktop.Notifications";
constexpr const char *c_objectPath = "/org/freedesktop/Notifications";
constexpr const char *c_interfaceName = c_serviceName;
}

NotificationManager::NotificationManager(QObject *parent)
    : INotificationManager{parent}
{
    m_dbusInitError = QString("D-Bus interface \"%1\" is not initialized.").arg(c_interfaceName);
}

void NotificationManager::registerDBusInterface()
{
    // org.freedesktop.Notifications might not be properly activated or available at the time daemon starts
    // so we might have to wait a little

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.isConnected())
    {
        m_dbusInitError = "Can't connect to D-Bus session bus.";
        logCritical() << m_dbusInitError;
        return;
    }

    int retries = 0;
    const int MAX_RETRIES = 10;
    const int RETRY_DELAY_MS = 1000;

    while (retries < MAX_RETRIES)
    {
        QDBusInterface *dbusInterface = new QDBusInterface(
            c_serviceName,
            c_objectPath,
            c_interfaceName,
            connection,
            this
        );
        if (dbusInterface->isValid())
        {
            logDebug() << "Successful at try" << retries;
            m_dbusInitError.clear();
            m_dbusInterface = dbusInterface;
            return;
        }
        logDebug() << "Try" << retries << "failed";

        QThread::msleep(RETRY_DELAY_MS);
        retries++;

        dbusInterface->deleteLater();
    }

    m_dbusInitError = QString("Failed to connect to D-Bus interface %1: %2").arg(c_interfaceName).arg(connection.lastError().message());
    logCritical() << m_dbusInitError;
}

bool NotificationManager::isValid() const
{
    return m_dbusInitError.isEmpty();
}

bool NotificationManager::sendNotification(const Notification &notification)
{
    if (!isValid())
    {
        logCritical() << m_dbusInitError;
        return false;
    }

    QDBusReply<uint> reply = m_dbusInterface->call(
        "Notify",
        PROGRAM_NAME_STRING_LITERAL,
        0u,                   // replaces_id (0 means new notification)
        QString(),            // app_icon
        notification.summary,
        notification.body,
        QStringList(),        // actions
        QVariantMap(),        // hints
        5000                  // expire_timeout (ms)
    );

    if (!reply.isValid())
    {
        logCritical() << QString("Error calling D-Bus method Notify (org.freedesktop.Notifications): %1").arg(reply.error().message());
        return false;
    }

    return true;
}

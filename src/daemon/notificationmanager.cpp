#include "notificationmanager.h"
#include "log.h"
#include <QDBusReply>

namespace
{
constexpr const char *c_serviceName = "org.freedesktop.Notifications";
constexpr const char *c_objectPath = "/org/freedesktop/Notifications";
constexpr const char *c_interfaceName = c_serviceName;
}

NotificationManager::NotificationManager(QObject *parent)
    : INotificationManager{parent}
{
    m_dbusInitError = QString("D-Bus interface \"%1\" is not initialized").arg(c_interfaceName);
    logInfo() << "NotificationManager instance created";
}

void NotificationManager::registerDBusInterface()
{
    m_dbusInterface = new QDBusInterface(
        c_serviceName,
        c_objectPath,
        c_interfaceName,
        QDBusConnection::sessionBus(),
        this
    );
    if (m_dbusInterface->isValid())
    {
        m_dbusInitError.clear();
    }
    else
    {
        m_dbusInitError = QString("Failed to connect to D-Bus interface \"%1\": \"%2\"").arg(c_interfaceName).arg(QDBusConnection::sessionBus().lastError().message());
        logCritical() << m_dbusInitError;
    }
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
        logCritical() << QString("Error calling D-Bus method Notify (org.freedesktop.Notifications): \"%1\"").arg(reply.error().message());
        return false;
    }

    return true;
}

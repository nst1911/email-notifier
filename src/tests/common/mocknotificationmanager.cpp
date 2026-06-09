#include "mocknotificationmanager.h"

MockNotificationManager::MockNotificationManager(QObject *parent)
    : INotificationManager(parent)
{

}

bool MockNotificationManager::isValid() const
{
    return m_testData.isValid;
}

bool MockNotificationManager::sendNotification(const Notification &notification)
{
    return m_testData.sendNotification;
}

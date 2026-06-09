#pragma once

#include "interfaces/inotificationmanager.h"
#include <QDBusInterface>

class NotificationManager : public INotificationManager
{
    Q_OBJECT

public:
    NotificationManager(QObject *parent = nullptr);

    void registerDBusInterface();

    bool isValid() const override;
    bool sendNotification(const Notification &notification) override;

private:
    QDBusInterface *m_dbusInterface;
    QString m_dbusInitError;
};

#pragma once

#include "imodule.h"
#include "notification.h"

class INotificationManager : public IModule
{
    Q_OBJECT

public:
    INotificationManager(QObject *parent = nullptr) : IModule(parent) {}
    virtual ~INotificationManager() {}

    virtual bool sendNotification(const Notification &notification) = 0;
};

#pragma once

#include "interfaces/inotificationmanager.h"

class MockNotificationManager : public INotificationManager
{
    Q_OBJECT

public:
    MockNotificationManager(QObject *parent = nullptr);

    bool isValid() const override;
    bool sendNotification(const Notification &notification) override;

    struct TestData
    {
        bool isValid;
        bool sendNotification;
    } m_testData;
};

Q_DECLARE_METATYPE(MockNotificationManager::TestData);

#pragma once

#include "configuration.h"
#include "message.h"
#include "result.h"
#include "imodule.h"
#include <QStringList>

class IDaemon : public IModule
{
    Q_OBJECT

public:
    IDaemon(QObject *parent = nullptr) : IModule(parent) {}
    virtual ~IDaemon() {}

    struct Status
    {
        Configuration configuration;
        bool isMonitoringActivated;
        Message lastError;
        bool operator==(const Status &other) const
        {
            return configuration == other.configuration &&
                   isMonitoringActivated == other.isMonitoringActivated &&
                   lastError == other.lastError;
        }
    };

    virtual Result<Status> status() const = 0;

    virtual QString setup(const Configuration &config) = 0;
    virtual QString startMonitoring() = 0;
    virtual QString stopMonitoring() = 0;
    virtual Result<QStringList> fetchMailboxes() = 0;
};

Q_DECLARE_METATYPE(IDaemon::Status);

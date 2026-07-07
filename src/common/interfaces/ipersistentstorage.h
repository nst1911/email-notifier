#pragma once

#include "idaemon.h"
#include "logmessage.h"
#include "result.h"

class IPersistentStorage : public IModule
{
    Q_OBJECT

public:
    IPersistentStorage(QObject *parent = nullptr) : IModule(parent) {}
    virtual ~IPersistentStorage() {}

    virtual Result<LogMessage> readErrorLogMessage() const = 0;
    virtual QString writeErrorLogMessage(const LogMessage &message) = 0;

    virtual Result<IDaemon::Configuration> readDaemonConfiguration() const = 0;
    virtual QString writeDaemonConfiguration(const IDaemon::Configuration &configuration) = 0;

    virtual Result<QString> readPassword(IDaemon::PasswordType type) const = 0;
    virtual QString writePassword(IDaemon::PasswordType type, const QString &password) = 0;

    virtual Result<LastMessageUIDs> readLastMessageUIDs() const = 0;
    virtual QString writeLastMessageUIDs(const LastMessageUIDs &uids) = 0;
};

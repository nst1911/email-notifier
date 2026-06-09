#pragma once

#include "configuration.h"
#include "result.h"
#include "lastmessageuids.h"
#include "message.h"
#include "imodule.h"

class IPersistentStorage : public IModule
{
    Q_OBJECT

public:
    IPersistentStorage(QObject *parent = nullptr) : IModule(parent) {}
    virtual ~IPersistentStorage() {}

    virtual Result<Message> readErrorLogMessage() const = 0;
    virtual QString writeErrorLogMessage(const Message &message) = 0;

    virtual Result<Configuration> readConfiguration() const = 0;
    virtual QString writeConfiguration(const Configuration &configuration) = 0;

    virtual Result<LastMessageUIDs> readLastMessageUIDs() const = 0;
    virtual QString writeLastMessageUIDs(const LastMessageUIDs &uids) = 0;
};

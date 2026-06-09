#pragma once

#include "configuration.h"
#include "result.h"
#include "imodule.h"
#include "lastmessageuids.h"
#include <QStringList>
#include <QMap>

class IMailClient : public IModule
{
    Q_OBJECT

public:
    IMailClient(QObject *parent = nullptr) : IModule(parent) {}
    virtual ~IMailClient() {}

    virtual Configuration configuration() const = 0;
    virtual void setConfiguration(const Configuration &config) = 0;

    virtual Result<QStringList> fetchMailboxes() = 0;
    virtual Result<LastMessageUIDs> fetchLastMessageUIDs() = 0;
};

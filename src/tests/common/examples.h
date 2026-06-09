#pragma once

#include "configuration.h"
#include "lastmessageuids.h"
#include "message.h"
#include "interfaces/idaemon.h"

namespace Examples
{

QStringList validMailboxes()
{
    return { "mailbox1", "mailbox2", "mailbox3" };
}

Configuration validConfig()
{
    Configuration config;
    config.host = "host";
    config.port = 67;
    config.login = "login";
    config.password = "password";
    config.mailboxes = validMailboxes();
    config.mailRequestIntervalMs = 100;
    return config;
}

LastMessageUIDs validLastMessageUIDs()
{
    LastMessageUIDs lastMessageUIDs;
    int i = 0;
    for (const QString &mailbox : validMailboxes())
    {
        lastMessageUIDs[mailbox] = i + 1;
        i++;
    }
    return lastMessageUIDs;
}

Message validErrorLogMessage()
{
    Message message;
    message.message = "test";
    message.timestamp = QDateTime(QDate(1996, 11, 26), QTime(7, 40, 33, 255));
    message.type = Message::Error;
    return message;
}

IDaemon::Status validDaemonStatus()
{
    IDaemon::Status status;
    status.configuration = Examples::validConfig();
    status.isMonitoringActivated = true;
    status.lastError = Examples::validErrorLogMessage();
    return status;
}

}

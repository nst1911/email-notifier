#pragma once

#include "interfaces/idaemon.h"
#include "interfaces/imailclient.h"
#include "message.h"

namespace Examples
{

QStringList validMailboxes()
{
    return { "mailbox1", "mailbox2", "mailbox3" };
}

IMailClient::Configuration validMailClientConfig()
{
    IMailClient::Configuration config;
    config.host = "imap.yandex.ru";
    config.port = 67;
    config.login = "nst1911@yandex.ru";
    config.password = "password123fffff";
    return config;
}

IDaemon::Configuration validDaemonConfig()
{
    IDaemon::Configuration config;
    config.mailClient = validMailClientConfig();
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
    message.message = "error";
    message.timestamp = QDateTime(QDate(1996, 11, 26), QTime(7, 40, 33, 255));
    message.type = Message::Error;
    return message;
}

IDaemon::Status validDaemonStatus()
{
    IDaemon::Status status;
    status.configuration = Examples::validDaemonConfig();
    status.isMonitoringActivated = true;
    status.lastError = Examples::validErrorLogMessage();
    return status;
}

}

#include "dbus.h"
#include <QtDBus/QtDBus>

QDBusArgument &operator<<(QDBusArgument &argument, const Message &message)
{
    argument.beginStructure();
    argument
        << message.message
        << message.timestamp
        << static_cast<int>(message.type);
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const Configuration &config)
{
    argument.beginStructure();
    argument
        << config.host
        << config.port
        << config.login
        << config.password
        << config.mailboxes
        << config.mailRequestIntervalMs;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const IDaemon::Status &status)
{
    argument.beginStructure();
    argument
        << status.isMonitoringActivated
        << status.configuration
        << status.lastError;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const Result<IDaemon::Status> &result)
{
    argument.beginStructure();
    argument << result.errorMessage() << result.data();
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const Result<QStringList> &result)
{
    argument.beginStructure();
    argument << result.errorMessage() << result.data();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Message &message)
{
    int type = 0;

    argument.beginStructure();
    argument
        >> message.message
        >> message.timestamp
        >> type;
    argument.endStructure();

    message.type = static_cast<Message::Type>(type);

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Configuration &config)
{
    argument.beginStructure();
    argument
        >> config.host
        >> config.port
        >> config.login
        >> config.password
        >> config.mailboxes
        >> config.mailRequestIntervalMs;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, IDaemon::Status &status)
{
    argument.beginStructure();
    argument
        >> status.isMonitoringActivated
        >> status.configuration
        >> status.lastError;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Result<IDaemon::Status> &result)
{
    QString errorMessage;
    IDaemon::Status status;

    argument.beginStructure();
    argument >> errorMessage >> status;
    argument.endStructure();

    result = Result<IDaemon::Status>(errorMessage, status);

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Result<QStringList> &result)
{
    QString errorMessage;
    QStringList data;

    argument.beginStructure();
    argument >> errorMessage >> data;
    argument.endStructure();

    result = Result<QStringList>(errorMessage, data);

    return argument;
}

void registerDBusMetaTypes()
{
    qDBusRegisterMetaType<Configuration>();
    qDBusRegisterMetaType<IDaemon::Status>();
    qDBusRegisterMetaType<Result<IDaemon::Status>>();
    qDBusRegisterMetaType<Result<QStringList>>();
}

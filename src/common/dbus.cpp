#include "dbus.h"
#include <QtDBus/QtDBus>

QDBusArgument &operator<<(QDBusArgument &argument, const LogMessage &message)
{
    argument.beginStructure();
    argument
        << message.message
        << message.timestamp
        << static_cast<int>(message.type);
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, IDaemon::PasswordType passwordType)
{
    argument.beginStructure();
    argument << static_cast<int>(passwordType);
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const IMailClient::Configuration &config)
{
    // do not pass password for safety. password would be read from secure keyring.
    argument.beginStructure();
    argument
        << config.host
        << config.port
        << config.login;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const IDaemon::Configuration &config)
{
    argument.beginStructure();
    argument
        << config.mailClient
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
    argument
        << result.errorMessage()
        << result.data();
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const Result<QStringList> &result)
{
    argument.beginStructure();
    argument
        << result.errorMessage()
        << result.data();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, LogMessage &message)
{
    int type = 0;

    argument.beginStructure();
    argument
        >> message.message
        >> message.timestamp
        >> type;
    argument.endStructure();

    message.type = static_cast<LogMessage::Type>(type);

    return argument;
}

const QDBusArgument& operator>>(const QDBusArgument& argument, IDaemon::PasswordType& passwordType)
{
    int passwordTypeInt;
    argument.beginStructure();
    argument >> passwordTypeInt;
    argument.endStructure();

    passwordType = static_cast<IDaemon::PasswordType>(passwordTypeInt);

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, IMailClient::Configuration &config)
{
    // do not pass password for safety. password would be read from secure keyring.
    argument.beginStructure();
    argument
        >> config.host
        >> config.port
        >> config.login;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, IDaemon::Configuration &config)
{
    argument.beginStructure();
    argument
        >> config.mailClient
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
    argument
        >> errorMessage
        >> status;
    argument.endStructure();

    result = Result<IDaemon::Status>(errorMessage, status);

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Result<QStringList> &result)
{
    QString errorMessage;
    QStringList data;

    argument.beginStructure();
    argument
        >> errorMessage
        >> data;
    argument.endStructure();

    result = Result<QStringList>(errorMessage, data);

    return argument;
}

void registerDBusMetaTypes()
{
    qDBusRegisterMetaType<IMailClient::Configuration>();
    qDBusRegisterMetaType<IDaemon::PasswordType>();
    qDBusRegisterMetaType<IDaemon::Configuration>();
    qDBusRegisterMetaType<IDaemon::Status>();
    qDBusRegisterMetaType<Result<IDaemon::Status>>();
    qDBusRegisterMetaType<Result<QStringList>>();
}

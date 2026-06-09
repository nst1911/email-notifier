#pragma once

#include "interfaces/idaemon.h"
#include <QDBusArgument>

#define DAEMON_DBUS_SERVICE_NAME   SERVICE_NAME_STRING_LITERAL
#define DAEMON_DBUS_OBJECT_PATH    "/com/github/nst1911/emailnotifier"
#define DAEMON_DBUS_INTERFACE_NAME DAEMON_DBUS_SERVICE_NAME

void registerDBusMetaTypes();

QDBusArgument &operator<<(QDBusArgument &argument, const Message &message);
QDBusArgument &operator<<(QDBusArgument &argument, const Configuration &config);
QDBusArgument &operator<<(QDBusArgument &argument, const IDaemon::Status &status);
QDBusArgument &operator<<(QDBusArgument &argument, const Result<IDaemon::Status> &result);
QDBusArgument &operator<<(QDBusArgument &argument, const Result<QStringList> &result);

const QDBusArgument &operator>>(const QDBusArgument &argument, Message &message);
const QDBusArgument &operator>>(const QDBusArgument &argument, Configuration &config);
const QDBusArgument &operator>>(const QDBusArgument &argument, IDaemon::Status &status);
const QDBusArgument &operator>>(const QDBusArgument &argument, Result<IDaemon::Status> &result);
const QDBusArgument &operator>>(const QDBusArgument &argument, Result<QStringList> &result);

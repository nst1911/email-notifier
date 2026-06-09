#pragma once

#include "interfaces/imodule.h"

class ICommandLineInterface : public IModule
{
    Q_OBJECT

public:
    enum ExitCode
    {
        Success = 0,
        InvalidModules,
        ParseArgumentsError,
        TooManyCommands,
        UnknownCommand,
        DaemonError
    };
    enum DaemonServiceStatus
    {
        Active,
        Inactive,
        Timeout
    };

    ICommandLineInterface(QObject *parent = nullptr) : IModule(parent) {};

    virtual DaemonServiceStatus isDaemonServiceActive() const = 0;
    virtual ExitCode process(const QStringList &arguments) = 0;
};

Q_DECLARE_METATYPE(ICommandLineInterface::DaemonServiceStatus);
Q_DECLARE_METATYPE(ICommandLineInterface::ExitCode);

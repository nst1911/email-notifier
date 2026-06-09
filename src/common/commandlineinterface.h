#pragma once

#include "configuration.h"
#include "result.h"
#include "interfaces/idaemon.h"
#include "interfaces/ipasswordinputhandler.h"
#include <QObject>
#include <QStringList>
#include <QCommandLineParser>

class CommandLineInterface : public IModule
{
    Q_OBJECT

public:
    enum ExitCode
    {
        Success = 0,
        InvalidModules,
        ParseArgumentsError,
        MissingRequiredCommands,
        TooManyCommands,
        UnknownCommand,
        DaemonError,
        PasswordInputHandlerError
    };

    CommandLineInterface(IDaemon *daemon, IPasswordInputHandler *handler, QObject *parent = nullptr);

    bool isValid() const override;

    ExitCode process(const QStringList &arguments);

private:
    Result<quint32> convertStringToMs(const QString& interval) const;
    QString convertMsToString(quint32 ms) const;

    ExitCode executeSetupCmd(QCommandLineParser &parser);
    ExitCode executeStartCmd(QCommandLineParser &parser);
    ExitCode executeStopCmd(QCommandLineParser &parser);
    ExitCode executeStatusCmd(QCommandLineParser &parser);
    ExitCode executeFetchMailboxesCmd(QCommandLineParser &parser);

    typedef CommandLineInterface::ExitCode(CommandLineInterface::*ExecuteCmdFunc)(QCommandLineParser&);
    QMap<QString, ExecuteCmdFunc> m_executeCmdMap;

    IDaemon *m_daemon = nullptr;
    IPasswordInputHandler *m_passwordInputHandler = nullptr;
};

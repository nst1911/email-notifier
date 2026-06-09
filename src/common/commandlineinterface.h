#pragma once

#include "interfaces/icommandlineinterface.h"
#include "interfaces/idaemon.h"
#include "interfaces/ipasswordinputhandler.h"
#include "result.h"
#include <QCommandLineParser>
#include <QObject>
#include <QStringList>

class CommandLineInterface : public ICommandLineInterface
{
    Q_OBJECT

public:
    CommandLineInterface(IDaemon *daemon, IPasswordInputHandler *handler, QObject *parent = nullptr);

    bool isValid() const override;
    DaemonServiceStatus isDaemonServiceActive() const override;

    ExitCode process(const QStringList &arguments) override;

private:
    Result<quint32> convertStringToMs(const QString& interval) const;
    QString convertMsToString(quint32 ms) const;

    Result<IMailClient::Configuration> parseMailClientConfig();

    ExitCode executeSetupCmd();
    ExitCode executeStartCmd();
    ExitCode executeStopCmd();
    ExitCode executeStatusCmd();
    ExitCode executeFetchMailboxesCmd();

    typedef CommandLineInterface::ExitCode(CommandLineInterface::*ExecuteCmdMethod)();
    QMap<QString, ExecuteCmdMethod> m_executeCmdMap;

    IDaemon *m_daemon = nullptr;
    IPasswordInputHandler *m_passwordInputHandler = nullptr;
    QCommandLineParser m_parser;
};

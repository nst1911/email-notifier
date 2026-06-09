#include "commandlineinterface.h"
#include "daemondbusinterface.h"
#include "passwordinputhandler.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(CLI_NAME_LITERAL);
    app.setOrganizationName(AUTHOR_EMAIL_STRING_LITERAL);
    app.setApplicationVersion(PROGRAM_VERSION_STRING_LITERAL);

    DaemonDBusInterface daemon;
    PasswordInputHandler passwordInputHandler;

    CommandLineInterface cli(&daemon, &passwordInputHandler);
    return cli.process(app.arguments());
}

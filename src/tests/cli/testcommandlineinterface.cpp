#include "testcommandlineinterface.h"

TestCommandLineInterface::TestCommandLineInterface(MockDaemon *daemon, MockPasswordInputHandler *handler, QObject *parent)
    : CommandLineInterface(daemon, handler, parent)
{

}

ICommandLineInterface::DaemonServiceStatus TestCommandLineInterface::isDaemonServiceActive() const
{
    return m_isDaemonServiceActive;
}

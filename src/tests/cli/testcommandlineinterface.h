#pragma once

#include "commandlineinterface.h"
#include "mockdaemon.h"
#include "mockpasswordinputhandler.h"

class TestCommandLineInterface : public CommandLineInterface
{
    Q_OBJECT

public:
    TestCommandLineInterface(MockDaemon *daemon, MockPasswordInputHandler *handler, QObject *parent = nullptr);

    DaemonServiceStatus isDaemonServiceActive() const override;
    DaemonServiceStatus m_isDaemonServiceActive = Active;
};

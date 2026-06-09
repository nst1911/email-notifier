#pragma once

#include "result.h"
#include "imodule.h"

class IPasswordInputHandler : public IModule
{
    Q_OBJECT

public:
    IPasswordInputHandler(QObject *parent = nullptr) : IModule(parent) {}
    virtual ~IPasswordInputHandler() {}
    virtual Result<QString> enterPassword() = 0;
};

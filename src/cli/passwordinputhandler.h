#pragma once

#include "interfaces/ipasswordinputhandler.h"

class PasswordInputHandler : public IPasswordInputHandler
{
    Q_OBJECT

public:
    PasswordInputHandler(QObject *parent = nullptr);

    bool isValid() const override;
    Result<QString> enterPassword() override;
};

#pragma once

#include "interfaces/ipasswordinputhandler.h"

class MockPasswordInputHandler : public IPasswordInputHandler
{
    Q_OBJECT

public:
    MockPasswordInputHandler(QObject *parent = nullptr) : IPasswordInputHandler(parent) {}

    bool isValid() const override;

    Result<QString> enterPassword() override;

    struct TestData
    {
        bool isValid;
        Result<QString> enterPassword;
    } m_testData;
};

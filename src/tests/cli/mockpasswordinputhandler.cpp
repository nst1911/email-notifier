#include "mockpasswordinputhandler.h"

bool MockPasswordInputHandler::isValid() const
{
    return m_testData.isValid;
}

Result<QString> MockPasswordInputHandler::enterPassword()
{
    return m_testData.enterPassword;
}

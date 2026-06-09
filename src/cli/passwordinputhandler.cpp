#include "passwordinputhandler.h"
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <string>

class TerminalGuard
{
public:
    TerminalGuard()
    {
        if (tcgetattr(STDIN_FILENO, &oldt_) != 0)
        {
            throw std::runtime_error("Failed to get terminal attributes");
        }
        newt_ = oldt_;
        newt_.c_lflag &= ~ECHO;
        if (tcsetattr(STDIN_FILENO, TCSADRAIN, &newt_) != 0)
        {
            throw std::runtime_error("Failed to set terminal attributes");
        }
    }
    ~TerminalGuard()
    {
        tcsetattr(STDIN_FILENO, TCSADRAIN, &oldt_);
    }

    TerminalGuard(const TerminalGuard&) = delete;
    TerminalGuard& operator=(const TerminalGuard&) = delete;

private:
    termios oldt_, newt_;
};

PasswordInputHandler::PasswordInputHandler(QObject *parent)
    : IPasswordInputHandler(parent)
{

}

bool PasswordInputHandler::isValid() const
{
    // TODO: can't find any invalid state for this
    return true;
}

Result<QString> PasswordInputHandler::enterPassword()
{
    static const int maxAttempts = 3;
    static const size_t maxPasswordLen = 1024;

    for (int attempts = 0; attempts < maxAttempts; ++attempts)
    {
        std::string password;
        try
        {
            std::cout
                << "NOTE! You may be asked to enter your Linux account password. It needs to save your email account password to secure keyring.\n\n"
                << "Enter email account password: "
                << std::flush;

            {
                TerminalGuard guard;
                if (!std::getline(std::cin, password))
                {
                    break;
                }
            }

            if (password.length() > maxPasswordLen)
            {
                std::cout << "\nPassword too long (max " << maxPasswordLen << " characters)\n";
                continue;
            }
            if (password.find_first_not_of(" \t\n\r") == std::string::npos) // Trim whitespace
            {
                std::cout << "\nPassword cannot be empty or whitespace only\n";
                continue;
            }

            std::cout << '\n';

            return Result<QString>::success(QString::fromStdString(password));
        }
        catch (const std::exception& e)
        {
            return Result<QString>::error(QString("Terminal configuration failed: \"%1\"").arg(e.what()));
        }
    }

    return Result<QString>::error("Too many invalid password attempts");
}

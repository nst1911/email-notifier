#include "imailclient.h"

IMailClient::Configuration::Configuration(const QJsonObject &obj)
{
    // skip password intentionally to avoid its leakage
    host = obj["host"].toString();
    port = obj["port"].toString().toUShort();
    login = obj["login"].toString();
}

bool IMailClient::Configuration::isValid() const
{
    return !host.isEmpty() && port != 0 && !login.isEmpty() && !password.isEmpty();
}

QStringList IMailClient::Configuration::invalidFields() const
{
    if (isValid())
    {
        return {};
    }

    QStringList fields;

    if (host.isEmpty())
    {
        fields.append("host");
    }
    if (port == 0)
    {
        fields.append("port");
    }
    if (login.isEmpty())
    {
        fields.append("login");
    }
    if (password.isEmpty())
    {
        fields.append("password");
    }

    return fields;
}

bool IMailClient::Configuration::operator==(const Configuration &other) const
{
    return host == other.host && port == other.port && login == other.login && password == other.password;
}

IMailClient::Configuration::operator QJsonObject() const
{
    // skip password intentionally to avoid its leakage
    return QJsonObject{
        {"host", host },
        {"port", QString::number(port) },
        {"login", login }
    };
}

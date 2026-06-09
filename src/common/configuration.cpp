#include "configuration.h"
#include <QMap>
#include <QJsonDocument>

Configuration::Configuration(const QJsonObject &obj)
{
    host = obj["host"].toString();
    port = obj["port"].toString().toUShort();
    login = obj["login"].toString();
    mailboxes = obj["mailboxes"].toString().split(";", Qt::SkipEmptyParts);
    mailRequestIntervalMs = obj["mailRequestIntervalMs"].toString().toULongLong();
}

bool Configuration::isValid() const
{
    return !host.isEmpty() &&
           port != 0 &&
           !login.isEmpty() &&
           !password.isEmpty() &&
           !mailboxes.isEmpty() &&
           mailRequestIntervalMs != 0;
}

bool Configuration::operator==(const Configuration &other) const
{
    return host == other.host &&
           port == other.port &&
           login == other.login &&
           password == other.password &&
           mailboxes == other.mailboxes &&
           mailRequestIntervalMs == other.mailRequestIntervalMs;
}

Configuration::operator QJsonObject() const
{
    // skip password intentionally to avoid its leakage
    return QJsonObject{
        {"host", host },
        {"port", QString::number(port) },
        {"login", login },
        {"mailboxes", mailboxes.join(";") },
        {"mailRequestIntervalMs", QString::number(mailRequestIntervalMs) }
    };
}

QString Configuration::toString() const
{
    return QJsonDocument(operator QJsonObject()).toJson(QJsonDocument::Compact);
}


#include "idaemon.h"
#include <QJsonDocument>

IDaemon::Configuration::Configuration(const QJsonObject &obj)
    : mailClient(IMailClient::Configuration(obj["mailClient"].toObject())),
      mailboxes(obj["mailboxes"].toString().split(";", Qt::SkipEmptyParts)),
      mailRequestIntervalMs(obj["mailRequestIntervalMs"].toString().toULong())
{
}

bool IDaemon::Configuration::isValid() const
{
    return mailClient.isValid() &&
           !mailboxes.isEmpty() &&
           mailRequestIntervalMs != 0;
}

QStringList IDaemon::Configuration::invalidFields() const
{
    if (isValid())
    {
        return {};
    }

    QStringList fields = mailClient.invalidFields();

    if (mailboxes.isEmpty())
    {
        fields.append("mailboxes");
    }
    if (mailRequestIntervalMs == 0)
    {
        fields.append("mailRequestIntervalMs");
    }

    return fields;
}

bool IDaemon::Configuration::operator==(const IDaemon::Configuration &other) const
{
    return mailClient == other.mailClient &&
           mailboxes == other.mailboxes &&
           mailRequestIntervalMs == other.mailRequestIntervalMs;
}

IDaemon::Configuration::operator QJsonObject() const
{
    // skip password intentionally to avoid its leakage
    return QJsonObject{
        {"mailClient", QJsonObject(mailClient) },
        {"mailboxes", mailboxes.join(";") },
        {"mailRequestIntervalMs", QString::number(mailRequestIntervalMs) }
    };
}

QString IDaemon::Configuration::toString() const
{
    QJsonObject jsonObj = operator QJsonObject();
    jsonObj["password"] = !mailClient.password.isEmpty() ? "<hidden>" : "";
    return QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
}

bool IDaemon::Status::operator==(const Status &other) const
{
    return other.configuration == other.configuration &&
           isMonitoringActivated == other.isMonitoringActivated &&
           lastError == other.lastError;
}

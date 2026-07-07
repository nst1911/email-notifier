#include "logmessage.h"
#include <QMap>
#include <QJsonDocument>

QString LogMessage::typeToStr(Type type)
{
    static const QMap<Type, QString> map = {
        { Info, "info" },
        { Warning, "warning" },
        { Error, "error" },
    };
    return map.value(type);
}

LogMessage::Type LogMessage::strToType(const QString &str)
{
    static const QMap<QString, Type> map = {
        { "info", Info },
        { "warning", Warning },
        { "error", Error }
    };
    return map.value(str, Unknown);
}

LogMessage::LogMessage(const QJsonObject &obj)
{
    message = obj["message"].toString();
    timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODateWithMs);
    type = strToType(obj["type"].toString());
}

bool LogMessage::isValid() const
{
    return !message.isEmpty() && timestamp.isValid();
}

bool LogMessage::operator==(const LogMessage &other) const
{
    return message == other.message && timestamp == other.timestamp;
}

LogMessage::operator QJsonObject() const
{
    return QJsonObject {
        { "message", message },
        { "timestamp", timestamp.toString(Qt::ISODateWithMs) },
        { "type", typeToStr(type) }
    };
}

QString LogMessage::toString() const
{
    return QJsonDocument(operator QJsonObject()).toJson(QJsonDocument::Compact);
}

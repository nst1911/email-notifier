#include "message.h"
#include <QMap>
#include <QJsonDocument>

QString Message::typeToStr(Type type)
{
    static const QMap<Type, QString> map = {
        { Info, "info" },
        { Warning, "warning" },
        { Error, "error" },
    };
    return map.value(type);
}

Message::Type Message::strToType(const QString &str)
{
    static const QMap<QString, Type> map = {
        { "info", Info },
        { "warning", Warning },
        { "error", Error }
    };
    return map.value(str, Unknown);
}

Message::Message(const QJsonObject &obj)
{
    message = obj["message"].toString();
    timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODateWithMs);
    type = strToType(obj["type"].toString());
}

bool Message::isValid() const
{
    return !message.isEmpty() && timestamp.isValid();
}

bool Message::operator==(const Message &other) const
{
    return message == other.message && timestamp == other.timestamp;
}

Message::operator QJsonObject() const
{
    return QJsonObject {
        { "message", message },
        { "timestamp", timestamp.toString(Qt::ISODateWithMs) },
        { "type", typeToStr(type) }
    };
}

QString Message::toString() const
{
    return QJsonDocument(operator QJsonObject()).toJson(QJsonDocument::Compact);
}

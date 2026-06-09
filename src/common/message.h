#pragma once

#include <QString>
#include <QDateTime>
#include <QJsonObject>

struct Message
{
    enum Type
    {
        Unknown = 0,
        Info,
        Warning,
        Error
    };
    static QString typeToStr(Type type);
    static Type strToType(const QString &str);

    Message() = default;
    Message(const QJsonObject &obj);

    bool isValid() const;
    bool operator==(const Message &other) const;
    explicit operator QJsonObject() const;

    QString toString() const;

    QString message;
    QDateTime timestamp;
    Type type;
};

Q_DECLARE_METATYPE(Message);

#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QString>

struct LogMessage
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

    LogMessage() = default;
    LogMessage(const QJsonObject &obj);

    bool isValid() const;
    bool operator==(const LogMessage &other) const;
    explicit operator QJsonObject() const;

    QString toString() const;

    QString message;
    QDateTime timestamp;
    Type type;
};

Q_DECLARE_METATYPE(LogMessage);

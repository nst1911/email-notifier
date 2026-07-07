#pragma once

#include <QJsonObject>
#include <QMap>
#include <QMetaType>

struct MessageInfo
{
    quint64 uid = 0;
    bool seen = false;

    MessageInfo() = default;
    MessageInfo(const QJsonObject &obj);

    bool operator==(const MessageInfo &other) const;
    explicit operator QJsonObject() const;

    QString toString() const;
};

typedef QMap<QString, MessageInfo> MessageInfoMap;

Q_DECLARE_METATYPE(MessageInfo);

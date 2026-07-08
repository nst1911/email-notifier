#pragma once

#include <QJsonObject>
#include <QMap>
#include <QMetaType>

struct MessageInfo
{
    quint64 uid = 0;
    bool seen = false;

    MessageInfo() = default;
    MessageInfo(quint64 uid, bool seen);
    MessageInfo(const QJsonObject &obj);

    bool operator==(const MessageInfo &other) const;
    bool operator!=(const MessageInfo &other) const;
    explicit operator QJsonObject() const;

    QString toString() const;
};

QDebug operator<<(QDebug debug, const MessageInfo& type);

typedef QMap<QString, MessageInfo> MessageInfoMap; // key is a mailbox

Q_DECLARE_METATYPE(MessageInfo);

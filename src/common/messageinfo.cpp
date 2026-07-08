#include "messageinfo.h"
#include <QJsonDocument>

MessageInfo::MessageInfo(quint64 uid, bool seen)
    : uid(uid),
      seen(seen)
{

}

MessageInfo::MessageInfo(const QJsonObject &obj)
    : uid(obj["uid"].toString().toULongLong()),
      seen(obj["seen"].toBool())
{

}

bool MessageInfo::operator==(const MessageInfo &other) const
{
    return uid == other.uid && seen == other.seen;
}

bool MessageInfo::operator!=(const MessageInfo &other) const
{
    return !operator==(other);
}

MessageInfo::operator QJsonObject() const
{
    return QJsonObject{{"uid", QString::number(uid)}, {"seen", seen }};
}

QString MessageInfo::toString() const
{
    return QJsonDocument(operator QJsonObject()).toJson(QJsonDocument::Compact);
}

QDebug operator<<(QDebug debug, const MessageInfo& info)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << info.toString();
    return debug;
}

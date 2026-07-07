#include "messageinfo.h"
#include <QJsonDocument>

MessageInfo::MessageInfo(const QJsonObject &obj)
{
    uid = obj["uid"].toString().toULongLong();
    seen = obj["seen"].toBool();
}

bool MessageInfo::operator==(const MessageInfo &other) const
{
    return uid == other.uid && seen == other.seen;
}

MessageInfo::operator QJsonObject() const
{
    return QJsonObject{{"uid", QString::number(uid)}, {"seen", seen }};
}

QString MessageInfo::toString() const
{
    return QJsonDocument(operator QJsonObject()).toJson(QJsonDocument::Compact);
}

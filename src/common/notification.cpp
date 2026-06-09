#include "notification.h"
#include <QMap>

bool Notification::isValid() const
{
    return !summary.isEmpty() && !body.isEmpty();
}

bool Notification::operator==(const Notification &other) const
{
    return summary == other.summary && body == other.body && urgency == other.urgency;
}

QString Notification::toString() const
{
    static const QMap<Urgency, QString> map = {
        {Info, "Info"},
        {Error, "Error"}
    };
    return QString("(summary=\"%1\", body=\"%2\", urgency=%3)").arg(summary).arg(body).arg(map[urgency]);
}

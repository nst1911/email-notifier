#pragma once

#include <QMetaType>
#include <QString>

struct Notification
{
    enum Urgency
    {
        Info,
        Error
    };

    QString summary;
    QString body;
    Urgency urgency = Info;

    bool isValid() const;
    bool operator==(const Notification &other) const;

    QString toString() const;
};

Q_DECLARE_METATYPE(Notification);

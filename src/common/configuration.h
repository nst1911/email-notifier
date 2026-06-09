#pragma once

#include <QStringList>
#include <QMetaType>
#include <QJsonObject>

struct Configuration
{
    Configuration() = default;
    Configuration(const QJsonObject &obj);

    bool isValid() const;
    bool operator==(const Configuration &other) const;
    explicit operator QJsonObject() const;

    QString toString() const;

    QString host;
    quint16 port = 0;
    QString login;
    QString password;
    QStringList mailboxes;
    quint32 mailRequestIntervalMs = 0;
};

Q_DECLARE_METATYPE(Configuration);

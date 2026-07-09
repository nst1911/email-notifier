#pragma once

#include "imodule.h"
#include "messageinfo.h"
#include "result.h"
#include <QJsonObject>
#include <QMap>
#include <QStringList>

class IMailClient : public IModule
{
    Q_OBJECT

public:
    struct Configuration
    {
        QString host;
        quint16 port = 0;
        QString login;
        QString password;

        Configuration() = default;
        Configuration(const Configuration&) = default;
        Configuration& operator=(const Configuration&) = default;
        ~Configuration() = default;

        Configuration(const QJsonObject &obj);

        bool isValid() const;
        QStringList invalidFields() const;

        bool operator==(const Configuration &other) const;
        explicit operator QJsonObject() const;
    };

    IMailClient(QObject *parent = nullptr) : IModule(parent) {}
    virtual ~IMailClient() {}

    virtual Result<QStringList> fetchMailboxes(const Configuration &config) = 0;
    virtual Result<MessageInfoMap> fetchLastMessageInfo(const Configuration &config, const QStringList &mailboxes) = 0;
};

Q_DECLARE_METATYPE(IMailClient::Configuration);

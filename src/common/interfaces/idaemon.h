#pragma once

#include "imailclient.h"
#include "imodule.h"
#include "message.h"
#include "result.h"
#include <QJsonObject>
#include <QStringList>

class IDaemon : public IModule
{
    Q_OBJECT

public:
    struct Configuration
    {
        IMailClient::Configuration mailClient;
        QStringList mailboxes;
        quint32 mailRequestIntervalMs = 0;

        Configuration() = default;
        Configuration(const Configuration&) = default;
        Configuration& operator=(const Configuration&) = default;
        ~Configuration() = default;

        Configuration(const QJsonObject &obj);

        bool isValid() const;

        QStringList invalidFields() const;

        bool operator==(const Configuration &other) const;
        explicit operator QJsonObject() const;

        QString toString() const;
    };

    struct Status
    {
        IDaemon::Configuration configuration;
        bool isMonitoringActivated;
        Message lastError;
        bool operator==(const Status &other) const;
    };

    enum PasswordType {
        DaemonConfiguration, // password that you enter when calling setup (it is stored permanently)
        FetchMailboxes       // password that you enter when calling fetch-mailboxes (it is deleted when fetch-mailboxes finished)
    };

    IDaemon(QObject *parent = nullptr) : IModule(parent) {}
    virtual ~IDaemon() {}

    virtual Result<Status> status() const = 0;

    virtual QString setup(const IDaemon::Configuration &config) = 0;
    virtual QString startMonitoring() = 0;
    virtual QString stopMonitoring() = 0;
    virtual Result<QStringList> fetchMailboxes(const IMailClient::Configuration &config) = 0;

    virtual QString writePassword(IDaemon::PasswordType type, const QString &password) = 0;
};

Q_DECLARE_METATYPE(IDaemon::PasswordType);
Q_DECLARE_METATYPE(IDaemon::Configuration);
Q_DECLARE_METATYPE(IDaemon::Status);

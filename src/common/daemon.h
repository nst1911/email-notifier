#pragma once

#include "dbus.h"
#include "interfaces/idaemon.h"
#include "interfaces/imailclient.h"
#include "interfaces/inotificationmanager.h"
#include "interfaces/ipersistentstorage.h"
#include <QTimer>

class Daemon : public IDaemon
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DAEMON_DBUS_INTERFACE_NAME)

public:
    Daemon(IMailClient *mailClient, IPersistentStorage *storage, INotificationManager *manager, QObject *parent = nullptr);
    ~Daemon();

    void registerDBusService();

    IMailClient *mailClient() const;
    IPersistentStorage *persistentStorage() const;
    INotificationManager *notificationManager() const;

    bool isValid() const override;

    bool isSetUp() const;

public slots:
    Result<Status> status() const override; // clazy:skip

    QString setup(const IDaemon::Configuration &config) override;

    QString startMonitoring() override;
    QString stopMonitoring() override;
    Result<QStringList> fetchMailboxes(const IMailClient::Configuration &config) override;

    QString writePassword(IDaemon::PasswordType type, const QString &password) override;

signals:
    void mailRequestFinished();
    void sendNotification(bool success, const Notification &notification);

private slots:
    QString onMailRequestTimerTimeout(bool writeErrMsg);
    void onSendNotification(bool success, const Notification &notification);

private:
    enum SetupMode
    {
        Default,
        WhileConstructingDaemon
    };

    QString setup(const IDaemon::Configuration &config, SetupMode options);

    bool isMonitoringActivated() const;

    void writeErrorLogMessage(const QString &funcName, const QString &message);

    QStringList compareLastMessageInfo(const MessageInfoMap &oldMap, const MessageInfoMap &newMap) const;

    bool checkModulesNullptr() const;

    IMailClient *m_mailClient;
    IPersistentStorage *m_storage;
    INotificationManager *m_notificationMngr;

    Configuration m_config;

    QTimer *m_mailRequestTimer;
};

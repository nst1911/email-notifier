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

public slots:
    Result<Status> status() const override; // clazy:skip

    QString setup(const Configuration &config) override;

    QString startMonitoring() override;
    QString stopMonitoring() override;
    Result<QStringList> fetchMailboxes() override;

signals:
    void mailRequestFinished();
    void sendNotification(bool success, const Notification &notification);

private slots:
    void onMailRequestTimerTimeout();
    void onSendNotification(bool success, const Notification &notification);

private:
    bool isMonitoringActivated() const;

    void writeErrorLogMessage(const QString &funcName, const QString &message);

    QStringList mailboxesWhereLastMessageUIDsNotEqual(const LastMessageUIDs &uids) const;
    QStringList compareLastMessageUIDs(const LastMessageUIDs &oldUids, const LastMessageUIDs &newUids) const;

    bool checkModulesNullptr() const;

    IMailClient *m_mailClient;
    IPersistentStorage *m_storage;
    INotificationManager *m_notificationMngr;

    QTimer *m_mailRequestTimer;
};

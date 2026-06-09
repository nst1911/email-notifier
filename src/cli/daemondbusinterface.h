#pragma once

#include "interfaces/idaemon.h"
#include <QDBusInterface>
#include <QDBusReply>

class DaemonDBusInterface : public IDaemon
{
    Q_OBJECT

public:
    DaemonDBusInterface(QObject *parent = nullptr);

    bool isValid() const override;

    Result<Status> status() const override;

    QString setup(const IDaemon::Configuration &config) override;
    QString startMonitoring() override;
    QString stopMonitoring() override;
    Result<QStringList> fetchMailboxes(const IMailClient::Configuration &config) override;
    QString writePassword(IDaemon::PasswordType type, const QString &password) override;

private:
    QDBusInterface *m_daemonDBusInterface = nullptr;

    template<typename T, typename... Args>
    T callDBusMethod(const QString& methodName, Args&&... args) const
    {
        if (!isValid())
        {
            if constexpr (std::is_same_v<T, QString>)
                return m_dbusInitError;
            else
                return T::error(m_dbusInitError);
        }
        QDBusReply<T> reply = m_daemonDBusInterface->call(methodName, std::forward<Args>(args)...);
        if (!reply.isValid())
        {
            const QString msg = QString("Calling D-Bus method \"%1\" failed: \"%2\"").arg(methodName).arg(reply.error().message());
            if constexpr (std::is_same_v<T, QString>)
                return msg;
            else
                return T::error(msg);
        }
        return reply.value();
    }

    QString m_dbusInitError;
};

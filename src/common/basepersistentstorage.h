#pragma once

#include "export.h"
#include "interfaces/ipersistentstorage.h"

class COMMON_VISIBILITY BasePersistentStorage : public IPersistentStorage
{
    Q_OBJECT

public:
    BasePersistentStorage(QObject *parent = nullptr);

    QString configFilePath() const;
    QString lastMessageUIDsFilePath() const;
    QString errorLogMessageFilePath() const;

    bool isValid() const override;

    Result<LogMessage> readErrorLogMessage() const override;
    QString writeErrorLogMessage(const LogMessage &message) override;

    Result<IDaemon::Configuration> readDaemonConfiguration() const override;
    QString writeDaemonConfiguration(const IDaemon::Configuration &configuration) override;

    Result<LastMessageUIDs> readLastMessageUIDs() const override;
    QString writeLastMessageUIDs(const LastMessageUIDs &uids) override;

protected:
    BasePersistentStorage(const QString &settingsDir, QObject *parent = nullptr);

private:
    QString settingsFilePath(const QString &fileName) const;
    QString m_settingsDir;

    QStringList settingsFilePathList() const;
};

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

    Result<Message> readErrorLogMessage() const override;
    QString writeErrorLogMessage(const Message &message) override;

    Result<Configuration> readConfiguration() const override;
    QString writeConfiguration(const Configuration &configuration) override;

    Result<LastMessageUIDs> readLastMessageUIDs() const override;
    QString writeLastMessageUIDs(const LastMessageUIDs &uids) override;

protected:
    BasePersistentStorage(const QString &settingsDir, QObject *parent = nullptr);

    virtual Result<QString> readPassword() const = 0;
    virtual QString writePassword(const QString &password) const = 0;

private:
    QString settingsFilePath(const QString &fileName) const;
    QString m_settingsDir;

    QStringList settingsFilePathList() const;
};

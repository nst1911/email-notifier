#include "basepersistentstorage.h"
#include "jsonhelper.h"
#include "log.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>

namespace
{
const QString c_settingsDefaultDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + PROGRAM_NAME_STRING_LITERAL;
constexpr const char *c_configFileName = "configuration.json";
constexpr const char *c_lastMessageUIDsFileName = "lastmessageuids.json";
constexpr const char *c_errorLogMessageFileName = "error.json";
}

BasePersistentStorage::BasePersistentStorage(QObject *parent)
    : BasePersistentStorage(c_settingsDefaultDir, parent)
{

}

bool BasePersistentStorage::isValid() const
{
    for (const QString &filePath : settingsFilePathList())
    {
        if (!QFile(filePath).exists())
        {
            return false;
        }
    }
    return true;
}

Result<LogMessage> BasePersistentStorage::readErrorLogMessage() const
{
    Result<QJsonObject> readJsonObjResult = JsonHelper::readObject(errorLogMessageFilePath());
    if (!readJsonObjResult.success())
    {
        return Result<LogMessage>::error(readJsonObjResult.errorMessage());
    }
    return Result<LogMessage>::success(readJsonObjResult.data());
}

QString BasePersistentStorage::writeErrorLogMessage(const LogMessage &message)
{
    if (message.type != LogMessage::Error)
    {
        return "Log message type is not Error";
    }
    return JsonHelper::writeObject(QJsonObject(message), errorLogMessageFilePath());
}

Result<IDaemon::Configuration> BasePersistentStorage::readDaemonConfiguration() const
{
    Result<QJsonObject> readJsonObjResult = JsonHelper::readObject(configFilePath());
    if (!readJsonObjResult.success())
    {
        return Result<IDaemon::Configuration>::error(readJsonObjResult.errorMessage());
    }
    return Result<IDaemon::Configuration>::success(readJsonObjResult.data());
}

QString BasePersistentStorage::writeDaemonConfiguration(const IDaemon::Configuration &configuration)
{
    return JsonHelper::writeObject(QJsonObject(configuration), configFilePath());
}

Result<LastMessageUIDs> BasePersistentStorage::readLastMessageUIDs() const
{
    Result<QJsonObject> readJsonObjResult = JsonHelper::readObject(lastMessageUIDsFilePath());
    if (!readJsonObjResult.success())
    {
        return Result<LastMessageUIDs>::error(readJsonObjResult.errorMessage());
    }

    QJsonObject jsonObj = readJsonObjResult.data();
    LastMessageUIDs uids;
    bool ok = false;

    for (const QString &mailbox : jsonObj.keys())
    {
        quint64 uid = jsonObj.value(mailbox).toString().toULongLong(&ok);
        if (!ok)
        {
            return Result<LastMessageUIDs>::error("Error while converting UID to quint64");
        }
        uids.insert(mailbox, uid);
    }
    return Result<LastMessageUIDs>::success(uids);
}

QString BasePersistentStorage::writeLastMessageUIDs(const LastMessageUIDs &uids)
{
    QJsonObject jsonObj;
    for (auto it = uids.cbegin(); it != uids.cend(); ++it)
    {
        jsonObj.insert(it.key(), QString::number(it.value()));
    }
    return JsonHelper::writeObject(jsonObj, lastMessageUIDsFilePath());
}

BasePersistentStorage::BasePersistentStorage(const QString &settingsDir, QObject *parent)
    : IPersistentStorage(parent), m_settingsDir(settingsDir)
{
    QDir dir(m_settingsDir);
    if (!dir.exists() && !dir.mkpath("."))
    {
        logCritical() << "Failed to create directory" << m_settingsDir;
        return;
    }
    for (const QString &filePath : settingsFilePathList())
    {
        if (QFile(filePath).exists())
        {
            continue;
        }
        QString error = JsonHelper::writeObject({}, filePath);
        if (!error.isEmpty())
        {
            logCritical() << QString("Failed to create file %1. Error: %2").arg(filePath, error);
        }
    }
}

QString BasePersistentStorage::settingsFilePath(const QString &fileName) const
{
    return !m_settingsDir.isEmpty() ? QString("%1/%2").arg(m_settingsDir, fileName) : "";
}

QStringList BasePersistentStorage::settingsFilePathList() const
{
    return { configFilePath(), lastMessageUIDsFilePath(), errorLogMessageFilePath() };
}

QString BasePersistentStorage::configFilePath() const
{
    return settingsFilePath(c_configFileName);
}

QString BasePersistentStorage::lastMessageUIDsFilePath() const
{
    return settingsFilePath(c_lastMessageUIDsFileName);
}

QString BasePersistentStorage::errorLogMessageFilePath() const
{
    return settingsFilePath(c_errorLogMessageFileName);
}

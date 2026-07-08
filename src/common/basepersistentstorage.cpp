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
constexpr const char *c_lastMessageInfoFileName = "lastmessageinfo.json";
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

Result<MessageInfoMap> BasePersistentStorage::readLastMessageInfo() const
{
    Result<QJsonObject> readJsonObjResult = JsonHelper::readObject(lastMessageInfoFilePath());
    if (!readJsonObjResult.success())
    {
        return Result<MessageInfoMap>::error(readJsonObjResult.errorMessage());
    }

    QJsonObject jsonObj = readJsonObjResult.data();
    MessageInfoMap messageInfoMap;
    bool ok = false;

    for (const QString &mailbox : jsonObj.keys())
    {
        messageInfoMap.insert(mailbox, MessageInfo(jsonObj[mailbox].toObject()));
    }
    return Result<MessageInfoMap>::success(messageInfoMap);
}

QString BasePersistentStorage::writeLastMessageInfo(const MessageInfoMap &messageInfoMap)
{
    QJsonObject jsonObj;
    for (auto it = messageInfoMap.cbegin(); it != messageInfoMap.cend(); ++it)
    {
        jsonObj.insert(it.key(), QJsonObject(it.value()));
    }
    return JsonHelper::writeObject(jsonObj, lastMessageInfoFilePath());
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
    return { configFilePath(), lastMessageInfoFilePath(), errorLogMessageFilePath() };
}

QString BasePersistentStorage::configFilePath() const
{
    return settingsFilePath(c_configFileName);
}

QString BasePersistentStorage::lastMessageInfoFilePath() const
{
    return settingsFilePath(c_lastMessageInfoFileName);
}

QString BasePersistentStorage::errorLogMessageFilePath() const
{
    return settingsFilePath(c_errorLogMessageFileName);
}

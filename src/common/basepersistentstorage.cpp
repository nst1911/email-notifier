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

Result<Message> BasePersistentStorage::readErrorLogMessage() const
{
    Result<QJsonObject> readJsonObjResult = JsonHelper::readObject(errorLogMessageFilePath());
    if (!readJsonObjResult.success())
    {
        return Result<Message>::error(readJsonObjResult.errorMessage());
    }
    return Result<Message>::success(readJsonObjResult.data());
}

QString BasePersistentStorage::writeErrorLogMessage(const Message &message)
{
    if (message.type != Message::Error)
    {
        return "Log message type is not Error";
    }
    return JsonHelper::writeObject(QJsonObject(message), errorLogMessageFilePath());
}

Result<Configuration> BasePersistentStorage::readConfiguration() const
{
    Result<QJsonObject> readJsonObjResult = JsonHelper::readObject(configFilePath());
    if (!readJsonObjResult.success())
    {
        return Result<Configuration>::error(readJsonObjResult.errorMessage());
    }

    Configuration config(readJsonObjResult.data());

    Result<QString> passwordResult = readPassword();
    if (!passwordResult.success())
    {
        return Result<Configuration>::error(passwordResult.errorMessage(), config);
    }

    config.password = passwordResult.data();
    return Result<Configuration>::success(config);
}

QString BasePersistentStorage::writeConfiguration(const Configuration &configuration)
{
    QString writeJsonObjResult = JsonHelper::writeObject(QJsonObject(configuration), configFilePath());
    if (!writeJsonObjResult.isEmpty())
    {
        return writeJsonObjResult;
    }

    return writePassword(configuration.password);
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
        logCritical() << "Failed to create directory:" << m_settingsDir;
        return;
    }
    for (const QString &filePath : settingsFilePathList())
    {
        if (QFile(filePath).exists())
        {
            continue;
        }
        QString error = JsonHelper::writeObject({}, filePath);
        if (error.isEmpty())
        {
            logInfo() << "File created:" << filePath;
        }
        else
        {
            logCritical() << QString("Failed to create file: \"%1\". Error: \"%2\"").arg(filePath).arg(error);
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

#include "jsonhelper.h"
#include <QFile>
#include <QJsonDocument>

Result<QJsonObject> JsonHelper::readObject(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return Result<QJsonObject>::error("Failed to open file: " + file.errorString());
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        return Result<QJsonObject>::error("JSON parse error: " + parseError.errorString());
    }

    if (!jsonDoc.isObject())
    {
        return Result<QJsonObject>::error("JSON document is not an object");
    }

    return Result<QJsonObject>::success(jsonDoc.object());
}

QString JsonHelper::writeObject(const QJsonObject &object, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        return "Failed to open file" + file.errorString();
    }

    QByteArray jsonData = QJsonDocument(object).toJson();

    qint64 bytesWritten = file.write(jsonData);
    file.close();

    return bytesWritten == jsonData.size() ? "" : QString("Failed to write all bytes. Expected: %1, written: %2").arg(jsonData.size()).arg(bytesWritten);
}

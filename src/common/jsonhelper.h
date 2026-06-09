#pragma once

#include "result.h"
#include <QJsonObject>

namespace JsonHelper
{
Result<QJsonObject> readObject(const QString &filePath);
QString writeObject(const QJsonObject &object, const QString &filePath);
}

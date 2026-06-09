#pragma once

#include <QDebug>

#define logDebug()    qDebug().noquote()
#define logInfo()     qInfo().noquote()
#define logWarning()  qWarning().noquote()
#define logCritical() qCritical().noquote()

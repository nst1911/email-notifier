#pragma once

#include <QObject>

class IModule : public QObject
{
    Q_OBJECT

public:
    IModule(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IModule() {}

    virtual bool isValid() const = 0;
};

#pragma once

#include <QObject>

class RAIIDeleter
{
public:
    RAIIDeleter(const QList<QObject*> &objects) : m_objects(objects) {}
    ~RAIIDeleter() { for (QObject *obj : std::as_const(m_objects)) if (obj) obj->deleteLater(); }
    
private:
    QList<QObject*> m_objects;
};

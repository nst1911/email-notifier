#pragma once

#include "basepersistentstorage.h"

class PersistentStorage : public BasePersistentStorage
{
    Q_OBJECT

public:
    PersistentStorage(QObject *parent = nullptr);

protected:
    Result<QString> readPassword() const override;
    QString writePassword(const QString &password) const override;
};

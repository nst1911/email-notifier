#pragma once

#include "basepersistentstorage.h"

class PersistentStorage : public BasePersistentStorage
{
    Q_OBJECT

public:
    PersistentStorage(QObject *parent = nullptr);

    Result<QString> readPassword(IDaemon::PasswordType type) const override;
    QString writePassword(IDaemon::PasswordType type, const QString &password) override;
};

#pragma once

#include "basepersistentstorage.h"

class TestBasePersistentStorage : public BasePersistentStorage
{
    Q_OBJECT

public:
    TestBasePersistentStorage(const QString &dir, QObject *parent = nullptr);

    Result<QString> readPassword(IDaemon::PasswordType type) const override;
    QString writePassword(IDaemon::PasswordType type, const QString &password) override;
};

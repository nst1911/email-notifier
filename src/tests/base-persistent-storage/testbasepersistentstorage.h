#pragma once

#include "basepersistentstorage.h"

class TestBasePersistentStorage : public BasePersistentStorage
{
    Q_OBJECT

public:
    TestBasePersistentStorage(const QString &dir, QObject *parent = nullptr);

    Result<QString> m_readPassword;
    QString m_writePassword;

protected:
    Result<QString> readPassword() const override;
    QString writePassword(const QString &password) const override;
};

#pragma once

#include "interfaces/idaemon.h"

class MockDaemon : public IDaemon
{
    Q_OBJECT

public:
    MockDaemon(QObject *parent = nullptr) : IDaemon(parent) {}

    bool isValid() const override;

    Result<Status> status() const override;

    QString setup(const Configuration &config) override;
    QString startMonitoring() override;
    QString stopMonitoring() override;
    Result<QStringList> fetchMailboxes(const IMailClient::Configuration &config) override;
    QString writePassword(IDaemon::PasswordType type, const QString &password) override;

    struct TestData
    {
        bool isValid;
        Result<Status> status;
        QString setup;
        QString startMonitoring;
        QString stopMonitoring;
        Result<QStringList> fetchMailboxes;
        QString writePassword;
    } m_testData;
};



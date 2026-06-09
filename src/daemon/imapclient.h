#pragma once

#include "basemailclient.h"
#include <curl/curl.h>

class IMAPClient : public BaseMailClient
{
    Q_OBJECT

public:
    IMAPClient(QObject *parent = nullptr);
    ~IMAPClient();

    bool isValid() const override;

    void setConfiguration(const Configuration &config) override;

    Result<QStringList> fetchMailboxes() override;
    Result<LastMessageUIDs> fetchLastMessageUIDs() override;

protected:
    Result<QStringList> fetchMailboxesImpl() override;
    Result<quint64> fetchLastMessageUID(const QString &mailbox) override;

private:
    static QString buildIMAPUrl(const Configuration &config);
    static size_t writeCallback(char *contents, size_t size, size_t nmemb, QString *output);

    CURL *m_curl = nullptr;
    QString m_curlInitError;
};

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

    Result<QStringList> fetchMailboxes(const Configuration &config) override;
    Result<LastMessageUIDs> fetchLastMessageUIDs(const Configuration &config, const QStringList &mailboxes) override;

protected:
    Result<QStringList> fetchMailboxesImpl(const Configuration &config) override;
    Result<quint64> fetchLastMessageUID(const Configuration &config, const QString &mailbox) override;

private:
    static QString buildIMAPUrl(const Configuration &config);
    static size_t writeCallback(char *contents, size_t size, size_t nmemb, QString *output);

    struct CurlWrapper
    {
        CurlWrapper();
        ~CurlWrapper();
        CURL *ptr = nullptr;
    };
    QString m_curlGlobalInitError;
};

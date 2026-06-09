#include "imapclient.h"
#include "log.h"
#include <QRegularExpression>

namespace
{

void curlLogSuccess(const QString &request, const QString &url)
{
    logInfo() << QString("%1 to %2 is successful").arg(request, url);
}

QString curlLogFailed(const QString &request, const QString &url, CURLcode res)
{
    QString msg = QString("%1 to %2 is failed: \"%3\"").arg(request, url, curl_easy_strerror(res));
    logCritical() << msg;
    return msg;
}

}

IMAPClient::IMAPClient(QObject *parent)
    : BaseMailClient(parent)
{
    CURLcode globalInit = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (globalInit != CURLE_OK)
    {
        m_curlGlobalInitError = QString("curl_global_init failed: \"%1\"").arg(curl_easy_strerror(globalInit));
        logCritical() << m_curlGlobalInitError;
    }
}

IMAPClient::~IMAPClient()
{
    curl_global_cleanup();
}

bool IMAPClient::isValid() const
{
    return m_curlGlobalInitError.isEmpty();
}

Result<QStringList> IMAPClient::fetchMailboxes(const Configuration &config)
{
    return m_curlGlobalInitError.isEmpty()
        ? BaseMailClient::fetchMailboxes(config)
        : Result<QStringList>::error(m_curlGlobalInitError);
}

Result<LastMessageUIDs> IMAPClient::fetchLastMessageUIDs(const Configuration &config, const QStringList &mailboxes)
{
    return m_curlGlobalInitError.isEmpty()
        ? BaseMailClient::fetchLastMessageUIDs(config, mailboxes)
        : Result<LastMessageUIDs>::error(m_curlGlobalInitError);
}

Result<QStringList> IMAPClient::fetchMailboxesImpl(const Configuration &config)
{
    CurlWrapper curl;
    if (!curl.ptr)
    {
        return Result<QStringList>::error("curl_easy_init failed");
    }

    curl_easy_setopt(curl.ptr, CURLOPT_USERNAME, qUtf8Printable(config.login));
    curl_easy_setopt(curl.ptr, CURLOPT_PASSWORD, qUtf8Printable(config.password));

    QString url = buildIMAPUrl(config);
    curl_easy_setopt(curl.ptr, CURLOPT_URL, qUtf8Printable(url));

    QString customRequest = "LIST \"\" \"*\"";
    curl_easy_setopt(curl.ptr, CURLOPT_CUSTOMREQUEST, qUtf8Printable(customRequest));

    QString responseData;
    curl_easy_setopt(curl.ptr, CURLOPT_WRITEDATA, &responseData);

    CURLcode res = curl_easy_perform(curl.ptr);
    if (res == CURLE_OK)
    {
        curlLogSuccess(customRequest, url);
    }
    else
    {
        return Result<QStringList>::error(curlLogFailed(customRequest, url, res));
    }

    QStringList mailboxes;
    QStringList lines = responseData.split("\r\n");
    for (const QString &line : lines)
    {
        // Remove trailing \r\n
        QString trimmed = line.trimmed();

        // Parse IMAP LIST response format: * LIST (\Attributes) "|" "Folder Name"
        if (!trimmed.startsWith("* LIST"))
        {
            continue;
        }

        QString mailbox = trimmed.split(' ').last();
        if (!mailbox.isEmpty())
        {
            mailboxes.append(mailbox);
        }
    }

    return Result<QStringList>::success(mailboxes);
}

Result<quint64> IMAPClient::fetchLastMessageUID(const Configuration &config, const QString &mailbox)
{
    CurlWrapper curl;
    if (!curl.ptr)
    {
        return Result<quint64>::error("curl_easy_init failed");
    }

    curl_easy_setopt(curl.ptr, CURLOPT_USERNAME, qUtf8Printable(config.login));
    curl_easy_setopt(curl.ptr, CURLOPT_PASSWORD, qUtf8Printable(config.password));

    QString url = QString("%1/%2").arg(buildIMAPUrl(config), mailbox);
    curl_easy_setopt(curl.ptr, CURLOPT_URL, qUtf8Printable(url));

    // Have to use a very large number (999999999) instead of UID SEARCH *
    // because Exchange's IMAP implementation doesn't support the * wildcard in SEARCH command
    QString customRequest = "UID SEARCH 999999999:*";
    curl_easy_setopt(curl.ptr, CURLOPT_CUSTOMREQUEST, qUtf8Printable(customRequest));

    QString responseData;
    curl_easy_setopt(curl.ptr, CURLOPT_WRITEDATA, &responseData);

    CURLcode res = curl_easy_perform(curl.ptr);
    if (res == CURLE_OK)
    {
        curlLogSuccess(customRequest, url);
        qDebug() << "responseData" << responseData; // qDebug instead logDebug to avoid printing \r\n to logs
    }
    else
    {
        return Result<quint64>::error(curlLogFailed(customRequest, url, res));
    }

    static const QRegularExpression searchRegex("SEARCH\\s+(\\d+)");
    QRegularExpressionMatch match = searchRegex.match(responseData);

    if (match.hasMatch())
    {
        QStringList uidStrings = match.captured(1).split(' ', Qt::SkipEmptyParts);
        if (!uidStrings.isEmpty())
        {
            bool ok = false;
            quint64 uid = uidStrings.last().toULongLong(&ok);
            return ok ? Result<quint64>::success(uid) : Result<quint64>::error(QString("%1 response parsing error").arg(customRequest));
        }
    }

    // No messages in mailbox
    return Result<quint64>::success(0);
}

QString IMAPClient::buildIMAPUrl(const Configuration &config)
{
    return QString("imaps://%1:%2").arg(config.host).arg(config.port);
}

size_t IMAPClient::writeCallback(char *contents, size_t size, size_t nmemb, QString *output)
{
    if (!output || !contents)
    {
        logCritical() << "IMAPClient::writeCallback: output or contents ptr is null";
        return 0;
    }
    size_t totalSize = size * nmemb;
    QString* response = static_cast<QString*>(output);
    response->append(QString::fromUtf8(static_cast<char*>(contents), totalSize));
    return totalSize;
}

IMAPClient::CurlWrapper::CurlWrapper()
{
    ptr = curl_easy_init();
    if (ptr)
    {
        curl_easy_setopt(ptr, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(ptr, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(ptr, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(ptr, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(ptr, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(ptr, CURLOPT_WRITEFUNCTION, &IMAPClient::writeCallback);
#ifndef NDEBUG
        curl_easy_setopt(ptr, CURLOPT_VERBOSE, 1L);
#endif
    }
}

IMAPClient::CurlWrapper::~CurlWrapper()
{
    curl_easy_cleanup(ptr);
}

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
        m_curlInitError = QString("curl_global_init failed: \"%1\"").arg(curl_easy_strerror(globalInit));
        logCritical() << m_curlInitError;
        return;
    }

    m_curl = curl_easy_init();
    if (!m_curl)
    {
        m_curlInitError = "curl_easy_init failed";
        logCritical() << m_curlInitError;
        return;
    }

    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(m_curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L); // Verify SSL certificate
    curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &IMAPClient::writeCallback);

    logInfo() << "IMAPClient instance created";
}

IMAPClient::~IMAPClient()
{
    curl_easy_cleanup(m_curl);
    curl_global_cleanup();
}

bool IMAPClient::isValid() const
{
    return m_curlInitError.isEmpty() && BaseMailClient::isValid();
}

void IMAPClient::setConfiguration(const Configuration &config)
{
    if (!m_curlInitError.isEmpty())
    {
        logInfo() << "Failed to initialize libcurl";
        return;
    }

    BaseMailClient::setConfiguration(config);

    curl_easy_setopt(m_curl, CURLOPT_USERNAME, qUtf8Printable(configuration().login));
    curl_easy_setopt(m_curl, CURLOPT_PASSWORD, qUtf8Printable(configuration().password));
}

Result<QStringList> IMAPClient::fetchMailboxes()
{
    return m_curlInitError.isEmpty()
        ? BaseMailClient::fetchMailboxes()
        : Result<QStringList>::error(m_curlInitError);
}

Result<LastMessageUIDs> IMAPClient::fetchLastMessageUIDs()
{
    return m_curlInitError.isEmpty()
               ? BaseMailClient::fetchLastMessageUIDs()
               : Result<LastMessageUIDs>::error(m_curlInitError);
}

Result<QStringList> IMAPClient::fetchMailboxesImpl()
{
    QString url = buildIMAPUrl(configuration());
    curl_easy_setopt(m_curl, CURLOPT_URL, qUtf8Printable(url));

    QString customRequest = "LIST \"\" \"*\"";
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, qUtf8Printable(customRequest));

    QString responseData;
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &responseData);

    CURLcode res = curl_easy_perform(m_curl);
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

Result<quint64> IMAPClient::fetchLastMessageUID(const QString &mailbox)
{
    QString url = QString("%1/%2").arg(buildIMAPUrl(configuration()), mailbox);
    curl_easy_setopt(m_curl, CURLOPT_URL, qUtf8Printable(url));

    // Use custom request to send SEARCH immediately after SELECT
    // Note: This works because libcurl maintains the connection
    QString customRequest = "SEARCH UID 1:*";
    curl_easy_setopt(m_curl, CURLOPT_CUSTOMREQUEST, qUtf8Printable(customRequest));

    QString responseData;
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &responseData);

    CURLcode res = curl_easy_perform(m_curl);
    if (res == CURLE_OK)
    {
        curlLogSuccess(customRequest, url);
    }
    else
    {
        return Result<quint64>::error(curlLogFailed(customRequest, url, res));
    }

    // Parse response - look for SEARCH results (ignoring SELECT responses)
    static const QRegularExpression searchRegex("\\* SEARCH (.+)");
    QRegularExpressionMatch match = searchRegex.match(responseData);

    if (match.hasMatch())
    {
        QStringList uidStrings = match.captured(1).split(' ', Qt::SkipEmptyParts);
        if (!uidStrings.isEmpty())
        {
            Result<quint64>::success(uidStrings.last().toULongLong());
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
        logCritical() << "output or contents ptr is null";
        return 0;
    }
    size_t totalSize = size * nmemb;
    QString* response = static_cast<QString*>(output);
    response->append(QString::fromUtf8(static_cast<char*>(contents), totalSize));
    return totalSize;
}


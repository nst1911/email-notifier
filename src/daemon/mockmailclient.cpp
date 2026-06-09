#include "jsonhelper.h"
#include "log.h"
#include "mockmailclient.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QStandardPaths>

namespace
{
const QString c_dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + PROGRAM_NAME_STRING_LITERAL;
const QString c_testDataFile = c_dir + "/mocktestdata.json";

void writeTestDataExample()
{
    // Generate test data like this:
    // {
    //     "fetchLastMessageUID": {
    //         "INBOX": [
    //             1,
    //             2,
    //             3,
    //             -1  <= fetchLastMessageUID would return error
    //         ],
    //         "INBOX1": [
    //             1,
    //             2,
    //             3,
    //             -1
    //         ],
    //         "INBOX2": [
    //             1,
    //             2,
    //             3,
    //             -1
    //         ]
    //     },
    //     "fetchMailboxes": [
    //         {
    //             "mailboxes": [
    //                 "INBOX",
    //                 "INBOX1",
    //                 "INBOX2"
    //             ],
    //             "success": true
    //         },
    //         {
    //             "mailboxes": [
    //                 "INBOX",
    //                 "INBOX1",
    //                 "INBOX2"
    //             ],
    //             "success": false <= fetchMailboxes would return error
    //         }
    //     ]
    // }


    const QStringList mailboxesExample = { "INBOX", "INBOX1", "INBOX2" };

    QJsonObject rootObj;

    // fetchMailboxes
    QJsonArray mailboxesArr;
    for (const QString &mailbox : mailboxesExample)
    {
        mailboxesArr.append(mailbox);
    }

    QJsonObject mailboxesSuccessObj;
    mailboxesSuccessObj["mailboxes"] = mailboxesArr;
    mailboxesSuccessObj["success"] = true;

    QJsonObject mailboxesFailObj;
    mailboxesFailObj["mailboxes"] = mailboxesArr;
    mailboxesFailObj["success"] = false;

    QJsonArray fetchMailboxesArr;
    fetchMailboxesArr.append(mailboxesSuccessObj);
    fetchMailboxesArr.append(mailboxesFailObj);

    rootObj["fetchMailboxes"] = fetchMailboxesArr;

    // fetchLastMessageUID
    QJsonObject fetchLastMessageUIDObj;
    for (const QString &mailbox : mailboxesExample)
    {
        QJsonArray uidsJsonArr;
        for (int i = 0; i < 3; ++i)
        {
            uidsJsonArr.append(i + 1);
        }
        uidsJsonArr.append(-1);
        fetchLastMessageUIDObj[mailbox] = uidsJsonArr;
    }
    rootObj["fetchLastMessageUID"] = fetchLastMessageUIDObj;

    // Write to file
    if (QString error = JsonHelper::writeObject(rootObj, c_testDataFile); !error.isEmpty())
    {
        logCritical() << error;
    }
}

}

MockMailClient::MockMailClient(QObject *parent)
    : BaseMailClient(parent)
{
    QDir dir(c_dir);
    if (!dir.exists() && !dir.mkpath("."))
    {
        logCritical() << "Failed to create directory:" << c_dir;
        return;
    }

    if (!QFile(c_testDataFile).exists())
    {
        writeTestDataExample();
    }

    readTestData();
    printTestData();
}


bool MockMailClient::isValid() const
{
    return true;
}

Result<QStringList> MockMailClient::fetchMailboxesImpl(const Configuration &config)
{
    Q_UNUSED(config);

    if (m_mailboxes.isEmpty())
    {
        return Result<QStringList>::error("No test data available");
    }

    const Result<QStringList> &data = m_mailboxes[m_currentMailboxIndex];
    m_currentMailboxIndex = (m_currentMailboxIndex + 1) % m_mailboxes.size();
    return data;
}

Result<quint64> MockMailClient::fetchLastMessageUID(const Configuration &config, const QString &mailbox)
{
    Q_UNUSED(config);

    if (!m_uids.contains(mailbox))
    {
        return Result<quint64>::error("Mailbox not found");
    }

    QList<Result<quint64>> uidList = m_uids[mailbox];
    if (uidList.isEmpty())
    {
        return Result<quint64>::error("No UID data for mailbox");
    }

    int& index = m_currentUidsIndex[mailbox];
    Result<quint64> result = uidList[index];
    index = (index + 1) % uidList.size();
    return result;
}

void MockMailClient::readTestData()
{
    Result<QJsonObject> result = JsonHelper::readObject(c_testDataFile);
    if (!result.success())
    {
        logCritical() << result.errorMessage();
        return;
    }

    QJsonObject rootJsonObj = result.data();

    // Data for fetchMailboxes
    for (const QJsonValue &mailboxesJsonVal : rootJsonObj["fetchMailboxes"].toArray())
    {
        QJsonObject mailboxesJsonObj = mailboxesJsonVal.toObject();

        if (!mailboxesJsonObj["success"].toBool())
        {
            m_mailboxes.append(Result<QStringList>::error("error"));
            continue;
        }

        QStringList mailboxes;
        for (const QJsonValue &mailboxJsonVal : mailboxesJsonObj["mailboxes"].toArray())
        {
            mailboxes.append(mailboxJsonVal.toString());
        }
        m_mailboxes.append(Result<QStringList>::success(mailboxes));
    }

    // Data for fetchLastMessageUID
    QJsonObject uidJsonObj = rootJsonObj["fetchLastMessageUID"].toObject();
    for (auto it = uidJsonObj.constBegin(); it != uidJsonObj.constEnd(); ++it)
    {
        for (const QJsonValue &uidJsonVal : it.value().toArray())
        {
            int uid = uidJsonVal.toInt(-1) ;
            Result<quint64> result = (uid == -1) ? Result<quint64>::error("error") : Result<quint64>::success(uid);
            m_uids[it.key()].append(result);
        }
    }
}

void MockMailClient::printTestData()
{
    static const QString error = "<ERROR>";

    QStringList fetchMailboxesTestDataStr;
    for (const Result<QStringList> &result : m_mailboxes)
    {
        fetchMailboxesTestDataStr.append(QString("[%1]").arg(result.success() ? result.data().join(",") : error));
    }
    logInfo() << "m_mailboxes:" << fetchMailboxesTestDataStr.join(", ");

    QStringList fetchLastMessageUIDTestDataStr;
    for (auto it = m_uids.constBegin(); it != m_uids.constEnd(); ++it)
    {
        QString mailbox = it.key();
        QStringList uids;
        for (const Result<quint64> &result : it.value())
        {
            uids.append(result.success() ? QString::number(result.data()) : error);
        }
        fetchLastMessageUIDTestDataStr.append(QString("[%1:(%2)]").arg(mailbox).arg(uids.join(",")));
    }
    logInfo() << "m_uids:" << fetchLastMessageUIDTestDataStr.join(", ");
}

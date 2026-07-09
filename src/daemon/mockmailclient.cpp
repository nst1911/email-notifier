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
    //     "fetchLastMessageInfo": {
    //         "INBOX": [
    //             {
    //                 "seen": false,
    //                 "uid": "1"
    //             },
    //             {
    //                 "seen": false,
    //                 "uid": "2"
    //             },
    //             {
    //                 "seen": false,
    //                 "uid": "3"
    //             },
    //             {
    //                 <= would return error
    //             }
    //         ],
    //         "INBOX1": [
    //             {
    //                 "seen": false,
    //                 "uid": "1"
    //             },
    //             {
    //                 "seen": false,
    //                 "uid": "2"
    //             },
    //             {
    //                 "seen": false,
    //                 "uid": "3"
    //             },
    //             {
    //                 <= would return error
    //             }
    //         ],
    //         "INBOX2": [
    //             {
    //                 "seen": false,
    //                 "uid": "1"
    //             },
    //             {
    //                 "seen": false,
    //                 "uid": "2"
    //             },
    //             {
    //                 "seen": false,
    //                 "uid": "3"
    //             },
    //             {
    //                 <= would return error
    //             }
    //         ]
    //     },
    //     "fetchMailboxes": [
    //      {
    //          "mailboxes": [
    //              "INBOX",
    //              "INBOX1",
    //              "INBOX2"
    //          ],
    //          "success": true
    //      },
    //      {
    //          "mailboxes": [
    //              "INBOX",
    //              "INBOX1",
    //              "INBOX2"
    //          ],
    //          "success": false <= would return error
    //      }
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

    // fetchLastMessageInfo
    QJsonObject fetchLastMessageInfoObj;
    for (const QString &mailbox : mailboxesExample)
    {
        QJsonArray lastMessageInfoArr;
        for (int i = 0; i < 3; ++i)
        {
            lastMessageInfoArr.append(QJsonObject(MessageInfo(i+1, false)));
        }
        lastMessageInfoArr.append(QJsonObject()); // for error case
        fetchLastMessageInfoObj[mailbox] = lastMessageInfoArr;
    }
    rootObj["fetchLastMessageInfo"] = fetchLastMessageInfoObj;

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

Result<MessageInfo> MockMailClient::fetchLastMessageInfoFromMailbox(const Configuration &config, const QString &mailbox)
{
    Q_UNUSED(config);

    if (!m_messageInfos.contains(mailbox))
    {
        return Result<MessageInfo>::error("Mailbox not found");
    }

    QList<Result<MessageInfo>> messageInfoList = m_messageInfos[mailbox];
    if (messageInfoList.isEmpty())
    {
        return Result<MessageInfo>::error("No message info for mailbox");
    }

    int& index = m_currentMessageInfoIndex[mailbox];
    Result<MessageInfo> result = messageInfoList[index];
    index = (index + 1) % messageInfoList.size();
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

    // Data for fetchLastMessageInfo
    QJsonObject messageInfoRootObj = rootJsonObj["fetchLastMessageInfo"].toObject();
    for (auto it = messageInfoRootObj.constBegin(); it != messageInfoRootObj.constEnd(); ++it)
    {
        for (const QJsonValue &messageInfoVal : it.value().toArray())
        {
            QJsonObject messageInfoObj = messageInfoVal.toObject();

            Result<MessageInfo> result = (!messageInfoObj.isEmpty())
                ? Result<MessageInfo>::success(MessageInfo(messageInfoObj))
                : Result<MessageInfo>::error("error");

            m_messageInfos[it.key()].append(result);
        }
    }
}

void MockMailClient::printTestData()
{
    static const QString error = "<ERROR>";

    QStringList fetchMailboxesStr;
    for (const Result<QStringList> &result : m_mailboxes)
    {
        fetchMailboxesStr.append(QString("[%1]").arg(result.success() ? result.data().join(",") : error));
    }
    logInfo() << "m_mailboxes:" << fetchMailboxesStr.join("; ");

    QStringList fetchLastMessageInfoStr;
    for (auto it = m_messageInfos.constBegin(); it != m_messageInfos.constEnd(); ++it)
    {
        QString mailbox = it.key();
        QStringList fetchLastMessageInfoFromMailboxStrList;
        for (const Result<MessageInfo> &result : it.value())
        {
            fetchLastMessageInfoFromMailboxStrList.append(result.success() ? result.data().toString(): error);
        }
        fetchLastMessageInfoStr.append(QString("[%1:(%2)]").arg(mailbox, fetchLastMessageInfoFromMailboxStrList.join("; ")));
    }
    logInfo() << "m_messageInfos:" << fetchLastMessageInfoStr.join(", ");
}

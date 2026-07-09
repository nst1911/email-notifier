#include "examples.h"
#include "jsonhelper.h"
#include "testbasepersistentstorage.h"
#include <QtTest>

Q_DECLARE_METATYPE(Result<QString>)

class BasePersistentStorageQtTest : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void createDirIfNotExists();
    void createFilesIfNotExist();

    void readErrorLogMessage();
    void readErrorLogMessageInvalidFile();

    void writeErrorLogMessage_data();
    void writeErrorLogMessage();
    void writeErrorLogMessageInvalidFile();

    void readDaemonConfiguration();
    void readConfigurationInvalidFile();

    void writeDaemonConfiguration();
    void writeConfigurationInvalidFile();

    void readLastMessageInfo();
    void readLastMessageInfoInvalidFile();

    void writeLastMessageInfo();
    void writeLastMessageInfoInvalidFile();

private:
    QJsonObject lastMessageInfoJsonObj(const MessageInfoMap &messageInfoMap) const;
    void createTestDir();
    void deleteTestDir();

    const QString c_testDir = QString("/tmp/%1").arg(PROGRAM_NAME_STRING_LITERAL);
};

void BasePersistentStorageQtTest::init()
{
    createTestDir();
}

void BasePersistentStorageQtTest::cleanup()
{
    deleteTestDir();
}

void BasePersistentStorageQtTest::createDirIfNotExists()
{
    // Assign
    deleteTestDir();

    // Act
    TestBasePersistentStorage storage(c_testDir);

    // Assert
    QVERIFY(QDir(c_testDir).exists());
}

void BasePersistentStorageQtTest::createFilesIfNotExist()
{
    // Assign
    deleteTestDir();

    // Act
    TestBasePersistentStorage storage(c_testDir);

    // Assert
    QVERIFY(QFile(storage.configFilePath()).exists());
    QVERIFY(QFile(storage.lastMessageInfoFilePath()).exists());
    QVERIFY(QFile(storage.errorLogMessageFilePath()).exists());
}

void BasePersistentStorageQtTest::readErrorLogMessage()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);

    JsonHelper::writeObject(QJsonObject(Examples::validErrorLogMessage()), storage.errorLogMessageFilePath());

    // Act
    Result<LogMessage> result = storage.readErrorLogMessage();

    // Assert
    QVERIFY(result.success());
    QCOMPARE(result.data(), Examples::validErrorLogMessage());
}

void BasePersistentStorageQtTest::readErrorLogMessageInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    Result<LogMessage> result = storage.readErrorLogMessage();

    // Assert
    QVERIFY(!result.success());
}

void BasePersistentStorageQtTest::writeErrorLogMessage_data()
{
    QTest::addColumn<LogMessage>("message");
    QTest::addColumn<bool>("expectedResult");
    QTest::addColumn<QJsonObject>("expectedJsonFileContents");

    {
        QTest::newRow("Happy path") << Examples::validErrorLogMessage() << true << QJsonObject(Examples::validErrorLogMessage());
    }

    {
        LogMessage message = Examples::validErrorLogMessage();
        message.type = LogMessage::Unknown;

        QTest::newRow("Invalid message type") << message << false << QJsonObject();
    }
}

void BasePersistentStorageQtTest::writeErrorLogMessage()
{
    // Assign
    QFETCH(LogMessage, message);
    QFETCH(bool, expectedResult);
    QFETCH(QJsonObject, expectedJsonFileContents);

    TestBasePersistentStorage storage(c_testDir);

    // Act
    QString result = storage.writeErrorLogMessage(message);

    // Assert
    QCOMPARE(result.isEmpty(), expectedResult);

    Result<QJsonObject> readResult = JsonHelper::readObject(storage.errorLogMessageFilePath());
    QCOMPARE(readResult.data(), expectedJsonFileContents);
}

void BasePersistentStorageQtTest::writeErrorLogMessageInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    QString result = storage.writeErrorLogMessage(Examples::validErrorLogMessage());

    // Assert
    QVERIFY(!result.isEmpty());
}

void BasePersistentStorageQtTest::readDaemonConfiguration()
{
    // Assign
    QJsonObject configFileContents(Examples::validDaemonConfig());

    IDaemon::Configuration configWithoutPassword = Examples::validDaemonConfig(); // password does not store in JSON file
    configWithoutPassword.mailClient.password = "";

    TestBasePersistentStorage storage(c_testDir);

    JsonHelper::writeObject(configFileContents, storage.configFilePath());

    // Act
    Result<IDaemon::Configuration> result = storage.readDaemonConfiguration();

    // Assert
    QVERIFY(result.success());
    QCOMPARE(result.data(), configWithoutPassword);
}

void BasePersistentStorageQtTest::readConfigurationInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    Result<IDaemon::Configuration> result = storage.readDaemonConfiguration();

    // Assert
    QVERIFY(!result.success());
}

void BasePersistentStorageQtTest::writeDaemonConfiguration()
{
    // Assign
    IDaemon::Configuration config = Examples::validDaemonConfig();

    TestBasePersistentStorage storage(c_testDir);

    // Act
    QString result = storage.writeDaemonConfiguration(config);

    // Assert
    QVERIFY(result.isEmpty());

    Result<QJsonObject> readResult = JsonHelper::readObject(storage.configFilePath());
    QVERIFY(readResult.success());
    QCOMPARE(readResult.data(), QJsonObject(config));
}

void BasePersistentStorageQtTest::writeConfigurationInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    QString result = storage.writeDaemonConfiguration(Examples::validDaemonConfig());

    // Assert
    QVERIFY(!result.isEmpty());
}

void BasePersistentStorageQtTest::readLastMessageInfo()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);

    MessageInfoMap messageInfo = Examples::validLastMessageInfo();
    QJsonObject messageInfoJsonObj = lastMessageInfoJsonObj(messageInfo);

    Q_ASSERT(JsonHelper::writeObject(messageInfoJsonObj, storage.lastMessageInfoFilePath()).isEmpty());

    // Act
    Result<MessageInfoMap> result = storage.readLastMessageInfo();

    // Assert
    QVERIFY(result.success());
    QCOMPARE(result.data(), messageInfo);
}

void BasePersistentStorageQtTest::readLastMessageInfoInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    Result<MessageInfoMap> result = storage.readLastMessageInfo();

    // Assert
    QVERIFY(!result.success());
}

void BasePersistentStorageQtTest::writeLastMessageInfo()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);

    MessageInfoMap messageInfo = Examples::validLastMessageInfo();
    QJsonObject messageInfoJsonObj = lastMessageInfoJsonObj(messageInfo);

    // Act
    QString result = storage.writeLastMessageInfo(messageInfo);

    // Assert
    QVERIFY(result.isEmpty());

    Result<QJsonObject> readResult = JsonHelper::readObject(storage.lastMessageInfoFilePath());
    QVERIFY(readResult.success());
    QCOMPARE(readResult.data(), messageInfoJsonObj);
}

void BasePersistentStorageQtTest::writeLastMessageInfoInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    QString result = storage.writeLastMessageInfo(Examples::validLastMessageInfo());

    // Assert
    QVERIFY(!result.isEmpty());
}

QJsonObject BasePersistentStorageQtTest::lastMessageInfoJsonObj(const MessageInfoMap &messageInfoMap) const
{
    QJsonObject jsonObj;
    for (auto it = messageInfoMap.cbegin(); it != messageInfoMap.cend(); ++it)
    {
        jsonObj.insert(it.key(), QJsonObject(it.value()));
    }
    return jsonObj;
}

void BasePersistentStorageQtTest::createTestDir()
{
    deleteTestDir();
    QDir(c_testDir).mkpath(".");
}

void BasePersistentStorageQtTest::deleteTestDir()
{
    QDir(c_testDir).removeRecursively();
}

QTEST_GUILESS_MAIN(BasePersistentStorageQtTest)

#include "tst_basepersistentstorage.moc"

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

    void readErrorLogMessage_data();
    void readErrorLogMessage();
    void readErrorLogMessageInvalidFile();

    void writeErrorLogMessage_data();
    void writeErrorLogMessage();
    void writeErrorLogMessageInvalidFile();

    void readConfiguration_data();
    void readConfiguration();
    void readConfigurationInvalidFile();

    void writeConfiguration_data();
    void writeConfiguration();
    void writeConfigurationInvalidFile();

    void readLastMessageUID();
    void readLastMessageUIDInvalidFile();

    void writeLastMessageUIDs();
    void writeLastMessageUIDsInvalidFile();

private:
    QJsonObject lastMessageUIDsJsonObj(const LastMessageUIDs &uids) const;
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
    QVERIFY(QFile(storage.lastMessageUIDsFilePath()).exists());
    QVERIFY(QFile(storage.errorLogMessageFilePath()).exists());
}

void BasePersistentStorageQtTest::readErrorLogMessage_data()
{
    QTest::addColumn<QJsonObject>("errorLogMessageFileContents");
    QTest::addColumn<bool>("expectedResultSuccess");
    QTest::addColumn<Message>("expectedResultMessage");

    {
        QTest::newRow("Happy path") << QJsonObject(Examples::validErrorLogMessage()) << true << Examples::validErrorLogMessage();
    }

    {
        Message message = Examples::validErrorLogMessage();
        message.type = Message::Unknown;

        QTest::newRow("Invalid message type") << QJsonObject(message) << false << Message{};
    }
}

void BasePersistentStorageQtTest::readErrorLogMessage()
{
    // Assign
    QFETCH(QJsonObject, errorLogMessageFileContents);
    QFETCH(bool, expectedResultSuccess);
    QFETCH(Message, expectedResultMessage);

    TestBasePersistentStorage storage(c_testDir);

    JsonHelper::writeObject(errorLogMessageFileContents, storage.errorLogMessageFilePath());

    // Act
    Result<Message> result = storage.readErrorLogMessage();

    // Assert
    QCOMPARE(result.success(), expectedResultSuccess);
    QCOMPARE(result.data(), expectedResultMessage);
}

void BasePersistentStorageQtTest::readErrorLogMessageInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    Result<Message> result = storage.readErrorLogMessage();

    // Assert
    QVERIFY(!result.success());
}

void BasePersistentStorageQtTest::writeErrorLogMessage_data()
{
    QTest::addColumn<Message>("message");
    QTest::addColumn<bool>("expectedResult");
    QTest::addColumn<QJsonObject>("expectedJsonFileContents");

    {
        QTest::newRow("Happy path") << Examples::validErrorLogMessage() << true << QJsonObject(Examples::validErrorLogMessage());
    }

    {
        Message message = Examples::validErrorLogMessage();
        message.type = Message::Unknown;

        QTest::newRow("Invalid message type") << message << false << QJsonObject();
    }
}

void BasePersistentStorageQtTest::writeErrorLogMessage()
{
    // Assign
    QFETCH(Message, message);
    QFETCH(bool, expectedResult);
    QFETCH(QJsonObject, expectedJsonFileContents);

    TestBasePersistentStorage storage(c_testDir);

    // Act
    QString result = storage.writeErrorLogMessage(message);

    // Assert
    QCOMPARE(result.isEmpty(), expectedResult);

    Result<QJsonObject> readResult = JsonHelper::readObject(storage.errorLogMessageFilePath());
    QCOMPARE(readResult.success(), expectedResult);
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

void BasePersistentStorageQtTest::readConfiguration_data()
{
    QTest::addColumn<QJsonObject>("configFileContents");
    QTest::addColumn<Result<QString>>("readPassword");
    QTest::addColumn<bool>("expectedResultSuccess");
    QTest::addColumn<Configuration>("expectedResultConfig");

    Result<QString> happyPathReadPassword = Result<QString>::success(Examples::validConfig().password);

    {
        QTest::newRow("Happy path") << QJsonObject(Examples::validConfig()) << happyPathReadPassword << true << Examples::validConfig();
    }

    {
        Configuration config = Examples::validConfig();
        config.password = "";

        QTest::newRow("Read password error") << QJsonObject(Examples::validConfig()) << Result<QString>::error("error") << false << config;
    }
}

void BasePersistentStorageQtTest::readConfiguration()
{
    // Assign
    QFETCH(QJsonObject, configFileContents);
    QFETCH(Result<QString>, readPassword);
    QFETCH(bool, expectedResultSuccess);
    QFETCH(Configuration, expectedResultConfig);

    TestBasePersistentStorage storage(c_testDir);
    storage.m_readPassword = readPassword;

    JsonHelper::writeObject(configFileContents, storage.configFilePath());

    // Act
    Result<Configuration> result = storage.readConfiguration();

    // Assert
    QCOMPARE(result.success(), expectedResultSuccess);
    QCOMPARE(result.data(), expectedResultConfig);
}

void BasePersistentStorageQtTest::readConfigurationInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    Result<Configuration> result = storage.readConfiguration();

    // Assert
    QVERIFY(!result.success());
}

void BasePersistentStorageQtTest::writeConfiguration_data()
{
    QTest::addColumn<Configuration>("config");
    QTest::addColumn<QString>("writePassword");
    QTest::addColumn<bool>("expectedResult");

    {
        Configuration config = Examples::validConfig();

        QTest::newRow("Happy path") << config << "" << true;
    }

    {
        Configuration config = Examples::validConfig();

        QTest::newRow("Write password error") << config << "error" << false;
    }
}

void BasePersistentStorageQtTest::writeConfiguration()
{
    // Assign
    QFETCH(Configuration, config);
    QFETCH(QString, writePassword);
    QFETCH(bool, expectedResult);

    Configuration configWithoutPassword = config; // password does not store in JSON file
    config.password = "";

    TestBasePersistentStorage storage(c_testDir);
    storage.m_writePassword = writePassword;

    // Act
    QString result = storage.writeConfiguration(config);

    // Assert
    QCOMPARE(result.isEmpty(), expectedResult);

    Result<QJsonObject> readResult = JsonHelper::readObject(storage.configFilePath());
    QVERIFY(readResult.success());
    QCOMPARE(readResult.data(), QJsonObject(configWithoutPassword));
}

void BasePersistentStorageQtTest::writeConfigurationInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    QString result = storage.writeConfiguration(Examples::validConfig());

    // Assert
    QVERIFY(!result.isEmpty());
}

void BasePersistentStorageQtTest::readLastMessageUID()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);

    LastMessageUIDs uids = Examples::validLastMessageUIDs();
    QJsonObject uidsJsonObj = lastMessageUIDsJsonObj(uids);
    Q_ASSERT(JsonHelper::writeObject(uidsJsonObj, storage.lastMessageUIDsFilePath()).isEmpty());

    // Act
    Result<LastMessageUIDs> result = storage.readLastMessageUIDs();

    // Assert
    QVERIFY(result.success());
    QCOMPARE(result.data(), uids);
}

void BasePersistentStorageQtTest::readLastMessageUIDInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    Result<LastMessageUIDs> result = storage.readLastMessageUIDs();

    // Assert
    QVERIFY(!result.success());
}

void BasePersistentStorageQtTest::writeLastMessageUIDs()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);

    LastMessageUIDs uids = Examples::validLastMessageUIDs();
    QJsonObject uidsJsonObj = lastMessageUIDsJsonObj(uids);

    // Act
    QString result = storage.writeLastMessageUIDs(uids);

    // Assert
    QVERIFY(result.isEmpty());

    Result<QJsonObject> readResult = JsonHelper::readObject(storage.lastMessageUIDsFilePath());
    QVERIFY(readResult.success());
    QCOMPARE(readResult.data(), uidsJsonObj);
}

void BasePersistentStorageQtTest::writeLastMessageUIDsInvalidFile()
{
    // Assign
    TestBasePersistentStorage storage(c_testDir);
    deleteTestDir();

    // Act
    QString result = storage.writeLastMessageUIDs(Examples::validLastMessageUIDs());

    // Assert
    QVERIFY(!result.isEmpty());
}

QJsonObject BasePersistentStorageQtTest::lastMessageUIDsJsonObj(const LastMessageUIDs &uids) const
{
    QJsonObject uidsJsonObj;
    for (auto it = uids.cbegin(); it != uids.cend(); ++it)
    {
        uidsJsonObj.insert(it.key(), QString::number(it.value()));
    }
    return uidsJsonObj;
}

void BasePersistentStorageQtTest::createTestDir()
{
    deleteTestDir();

    QDir dir(c_testDir);
    Q_ASSERT(dir.mkpath("."));
}

void BasePersistentStorageQtTest::deleteTestDir()
{
    QDir dir(c_testDir);
    if (dir.exists())
    {
        Q_ASSERT(dir.removeRecursively());
    }
}

QTEST_GUILESS_MAIN(BasePersistentStorageQtTest)

#include "tst_basepersistentstorage.moc"

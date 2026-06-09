#include "daemon.h"
#include "examples.h"
#include "log.h"
#include "mockmailclient.h"
#include "mocknotificationmanager.h"
#include "mockpersistentstorage.h"
#include "raiideleter.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>

struct StartMonitoringResult
{
    bool success;
    bool writeErrorLogMessageCalled;
    bool isMonitoringActivated;
};
Q_DECLARE_METATYPE(StartMonitoringResult);

struct OnMailRequestTimerResult
{
    bool writeErrorLogMessageCalled;
    bool isMonitoringActivated;
    int mailRequestFinishedSignalCount;
    QList<bool> notificationSentSignalsResults;
};
Q_DECLARE_METATYPE(OnMailRequestTimerResult);

struct SetupResult
{
    bool success;
    bool writeErrorLogMessageCalled;
    bool isMonitoringActivated;
};
Q_DECLARE_METATYPE(SetupResult);

class DaemonQtTest : public QObject
{
    Q_OBJECT

private slots:
    void status_data();
    void status();
    void statusAfterStoppingMonitoring();

    void setup_data();
    void setup();

    void startMonitoring_data();
    void startMonitoring();

    void stopMonitoring_data();
    void stopMonitoring();

    void fetchMailboxes_data();
    void fetchMailboxes();

    void nullptrModules_data();
    void nullptrModules();

    void onMailRequestTimerTimeout_data();
    void onMailRequestTimerTimeout();

    void setupInConstructor_data();
    void setupInConstructor();

private:
    MockPersistentStorage::TestData happyPathStorageTestData() const;
    MockMailClient::TestData happyPathMailClientTestData() const;
    MockNotificationManager::TestData happyPathNotificationManagerTestData() const;

    Daemon *createDaemon(
        const MockPersistentStorage::TestData &storageTestData,
        const MockMailClient::TestData &mailClientTestData,
        const MockNotificationManager::TestData &managerTestData
    ) const;

    bool checkMailRequestFinishedSignals(const OnMailRequestTimerResult &result, QSignalSpy &spy) const;
    bool checkSendNotificationSignals(const OnMailRequestTimerResult &result, QSignalSpy &spy) const;
};

namespace
{
const quint32 c_mailRequestIntervalMs = Examples::validDaemonConfig().mailRequestIntervalMs;
const int c_lastMessageUIDFetchCount = 5; // just a random number - just to provide at least N ticks of timer = N fetchings of last message UID
}

void DaemonQtTest::status_data()
{
    QTest::addColumn<MockPersistentStorage::TestData>("storageTestData");
    QTest::addColumn<IDaemon::Status>("expectedStatus");
    QTest::addColumn<bool>("expectedResult");

    {
        QTest::newRow("Happy path") << happyPathStorageTestData() << Examples::validDaemonStatus() << true;
    }

    {
        MockPersistentStorage::TestData testData = happyPathStorageTestData();
        testData.isValid = false;

        QTest::newRow("PersistentStorage is invalid") << testData << IDaemon::Status{} << false;
    }

    {
        MockPersistentStorage::TestData testData = happyPathStorageTestData();
        testData.readDaemonConfiguration = Result<IDaemon::Configuration>::error("error");

        IDaemon::Status status = Examples::validDaemonStatus();
        status.isMonitoringActivated = false;
        status.configuration = {};

        QTest::newRow("PersistentStorage::readDaemonConfiguration failed") << testData << status << false;
    }

    {
        MockPersistentStorage::TestData testData = happyPathStorageTestData();
        testData.readErrorLogMessage = Result<Message>::error("error");

        IDaemon::Status status = Examples::validDaemonStatus();
        status.lastError = {};

        QTest::newRow("PersistentStorage::readErrorLogMessage failed") << testData << status << false;
    }
}

void DaemonQtTest::status()
{
    // Assign
    QFETCH(MockPersistentStorage::TestData, storageTestData);
    QFETCH(IDaemon::Status, expectedStatus);
    QFETCH(bool, expectedResult);

    Daemon *daemon = createDaemon(
        storageTestData,
        happyPathMailClientTestData(),
        happyPathNotificationManagerTestData()
    );
    RAIIDeleter deleter({daemon});

    daemon->persistentStorage()->writeErrorLogMessage(expectedStatus.lastError);
    daemon->setup(expectedStatus.configuration);
    daemon->startMonitoring();

    // Act
    Result<IDaemon::Status> result = daemon->status();

    // Assert
    QCOMPARE(result.success(), expectedResult);
    QCOMPARE(result.data(), expectedStatus);
}

void DaemonQtTest::statusAfterStoppingMonitoring()
{
    // Assign
    Daemon *daemon = createDaemon(
        happyPathStorageTestData(),
        happyPathMailClientTestData(),
        happyPathNotificationManagerTestData()
    );
    RAIIDeleter deleter({daemon});

    Q_ASSERT(daemon->startMonitoring().isEmpty());
    QCoreApplication::processEvents();

    daemon->stopMonitoring();
    QCoreApplication::processEvents();

    // Act
    Result<IDaemon::Status> result = daemon->status();

    // Assert
    QVERIFY(result.success());
    QVERIFY(!result.data().isMonitoringActivated);
}

void DaemonQtTest::setup_data()
{
    QTest::addColumn<MockPersistentStorage::TestData>("storageTestData");
    QTest::addColumn<IDaemon::Configuration>("config");
    QTest::addColumn<SetupResult>("expectedResult");

    {
        SetupResult result;
        result.success = true;
        result.isMonitoringActivated = false;
        result.writeErrorLogMessageCalled = false;

        QTest::newRow("Happy path")
            << happyPathStorageTestData()
            << Examples::validDaemonConfig()
            << result;
    }

    {
        SetupResult result;
        result.success = false;
        result.isMonitoringActivated = true;
        result.writeErrorLogMessageCalled = true;

        QTest::newRow("Monitoring is activated")
            << happyPathStorageTestData()
            << Examples::validDaemonConfig()
            << result;
    }

    {
        MockPersistentStorage::TestData data;
        data.isValid = false;

        SetupResult result;
        result.success = false;
        result.isMonitoringActivated = false;
        result.writeErrorLogMessageCalled = false; // because storage is not valid and we can't write error log message

        QTest::newRow("Invalid PersistentStorage")
            << data
            << Examples::validDaemonConfig()
            << result;
    }

    {
        IDaemon::Configuration config = Examples::validDaemonConfig();
        config.mailClient.host = "";

        SetupResult result;
        result.success = false;
        result.isMonitoringActivated = false;
        result.writeErrorLogMessageCalled = true;

        QTest::newRow("Invalid config (host)")
            << happyPathStorageTestData()
            << config
            << result;
    }

    {
        IDaemon::Configuration config = Examples::validDaemonConfig();
        config.mailClient.port = 0;

        SetupResult result;
        result.success = false;
        result.isMonitoringActivated = false;
        result.writeErrorLogMessageCalled = true;

        QTest::newRow("Invalid config (port)")
            << happyPathStorageTestData()
            << config
            << result;
    }

    {
        IDaemon::Configuration config = Examples::validDaemonConfig();
        config.mailClient.login = "";

        SetupResult result;
        result.success = false;
        result.isMonitoringActivated = false;
        result.writeErrorLogMessageCalled = true;

        QTest::newRow("Invalid config (login)")
            << happyPathStorageTestData()
            << config
            << result;
    }

    {
        IDaemon::Configuration config = Examples::validDaemonConfig();
        config.mailRequestIntervalMs = 0;

        SetupResult result;
        result.success = false;
        result.isMonitoringActivated = false;
        result.writeErrorLogMessageCalled = true;

        QTest::newRow("Invalid config (mailRequestIntervalMs)")
            << happyPathStorageTestData()
            << config
            << result;
    }

    {
        MockPersistentStorage::TestData testData = happyPathStorageTestData();
        testData.readPassword = Result<QString>::error("error");

        SetupResult result;
        result.success = false;
        result.isMonitoringActivated = false;
        result.writeErrorLogMessageCalled = true;

        QTest::newRow("PersistentStorage::readPassword failed")
            << testData
            << Examples::validDaemonConfig()
            << result;
    }

    {
        MockPersistentStorage::TestData testData = happyPathStorageTestData();
        testData.writeDaemonConfiguration = "error";

        SetupResult result;
        result.success = false;
        result.isMonitoringActivated = false;
        result.writeErrorLogMessageCalled = true;

        QTest::newRow("PersistentStorage::writeDaemonConfiguration failed")
            << testData
            << Examples::validDaemonConfig()
            << result;
    }
}

void DaemonQtTest::setup()
{
    // Assign
    QFETCH(MockPersistentStorage::TestData, storageTestData);
    QFETCH(IDaemon::Configuration, config);
    QFETCH(SetupResult, expectedResult);

    Daemon *daemon = createDaemon(
        storageTestData,
        happyPathMailClientTestData(),
        happyPathNotificationManagerTestData()
    );
    RAIIDeleter deleter({daemon});

    MockPersistentStorage *mockStorage = static_cast<MockPersistentStorage*>(daemon->persistentStorage());

    if (expectedResult.isMonitoringActivated)
    {
        Q_ASSERT(daemon->startMonitoring().isEmpty());
    }

    // Act
    QString result = daemon->setup(config);

    // Assert
    QCOMPARE(result.isEmpty(), expectedResult.success);
    QCOMPARE(mockStorage->m_testData.writeErrorLogMessageCalled, expectedResult.writeErrorLogMessageCalled);
}

void DaemonQtTest::startMonitoring_data()
{
    QTest::addColumn<MockPersistentStorage::TestData>("storageTestData");
    QTest::addColumn<MockMailClient::TestData>("mailClientTestData");
    QTest::addColumn<MockNotificationManager::TestData>("managerTestData");
    QTest::addColumn<StartMonitoringResult>("expectedResult");

    {
        StartMonitoringResult result;
        result.success = true;
        result.writeErrorLogMessageCalled = false;
        result.isMonitoringActivated = true;

        QTest::newRow("Happy path")
            << happyPathStorageTestData()
            << happyPathMailClientTestData()
            << happyPathNotificationManagerTestData()
            << result;
    }

    {
        MockPersistentStorage::TestData storageTestData = happyPathStorageTestData();
        storageTestData.isValid = false;

        StartMonitoringResult result;
        result.success = false;
        result.writeErrorLogMessageCalled = false;
        result.isMonitoringActivated = false;

        QTest::newRow("PersistentStorage::isValid failed")
            << storageTestData
            << happyPathMailClientTestData()
            << happyPathNotificationManagerTestData()
            << result;
    }

    {
        MockMailClient::TestData mailClientTestData = happyPathMailClientTestData();
        mailClientTestData.isValid = false;

        StartMonitoringResult result;
        result.success = false;
        result.writeErrorLogMessageCalled = true;
        result.isMonitoringActivated = false;

        QTest::newRow("MailClient::isValid failed")
            << happyPathStorageTestData()
            << mailClientTestData
            << happyPathNotificationManagerTestData()
            << result;
    }

    {
        MockNotificationManager::TestData managerTestData = happyPathNotificationManagerTestData();
        managerTestData.isValid = false;

        StartMonitoringResult result;
        result.success = false;
        result.writeErrorLogMessageCalled = true;
        result.isMonitoringActivated = false;

        QTest::newRow("NotificationManager::isValid failed")
            << happyPathStorageTestData()
            << happyPathMailClientTestData()
            << managerTestData
            << result;
    }

    {
        MockPersistentStorage::TestData storageTestData = happyPathStorageTestData();
        storageTestData.readDaemonConfiguration = Result<IDaemon::Configuration>::success({});

        StartMonitoringResult result;
        result.success = false;
        result.writeErrorLogMessageCalled = true;
        result.isMonitoringActivated = false;

        QTest::newRow("isSetUp() failed")
            << storageTestData
            << happyPathMailClientTestData()
            << happyPathNotificationManagerTestData()
            << result;
    }

    {
        IDaemon::Configuration config = Examples::validDaemonConfig();
        config.mailRequestIntervalMs = 0;

        MockPersistentStorage::TestData storageTestData = happyPathStorageTestData();
        storageTestData.readDaemonConfiguration = Result<IDaemon::Configuration>::success(config);

        StartMonitoringResult result;
        result.success = false;
        result.writeErrorLogMessageCalled = true;
        result.isMonitoringActivated = false;

        QTest::newRow("Invalid mail request interval")
            << storageTestData
            << happyPathMailClientTestData()
            << happyPathNotificationManagerTestData()
            << result;
    }

    {
        MockMailClient::TestData mailClientTestData = happyPathMailClientTestData();
        mailClientTestData.fetchLastMessageUIDsQueue.clear();
        mailClientTestData.fetchLastMessageUIDsQueue.enqueue(Result<LastMessageUIDs>::error("error"));

        StartMonitoringResult result;
        result.success = false;
        result.writeErrorLogMessageCalled = true;
        result.isMonitoringActivated = false;

        QTest::newRow("First onMailRequestTimerTimeout() failed")
            << happyPathStorageTestData()
            << mailClientTestData
            << happyPathNotificationManagerTestData()
            << result;
    }
}

void DaemonQtTest::startMonitoring()
{
    // Assign
    QFETCH(MockPersistentStorage::TestData, storageTestData);
    QFETCH(MockMailClient::TestData, mailClientTestData);
    QFETCH(MockNotificationManager::TestData, managerTestData);
    QFETCH(StartMonitoringResult, expectedResult);

    Daemon *daemon = createDaemon(
        storageTestData,
        mailClientTestData,
        managerTestData
    );
    RAIIDeleter deleter({daemon});

    MockPersistentStorage *mockStorage = static_cast<MockPersistentStorage*>(daemon->persistentStorage());

    // Act
    QString result = daemon->startMonitoring();

    // Assert
    QCOMPARE(result.isEmpty(), expectedResult.success);
    QCOMPARE(mockStorage->m_testData.writeErrorLogMessageCalled, expectedResult.writeErrorLogMessageCalled);
    QCOMPARE(daemon->status().data().isMonitoringActivated, expectedResult.isMonitoringActivated);
}

void DaemonQtTest::stopMonitoring_data()
{
    QTest::addColumn<bool>("activated");
    QTest::addColumn<bool>("expectedResult");

    QTest::newRow("Monitoring activated") << true << true;
    QTest::newRow("Monitoring not activated") << false << false;
}

void DaemonQtTest::stopMonitoring()
{
    // Assign
    QFETCH(bool, activated);

    Daemon *daemon = createDaemon(
        happyPathStorageTestData(),
        happyPathMailClientTestData(),
        happyPathNotificationManagerTestData()
    );
    RAIIDeleter deleter({daemon});

    MockPersistentStorage *mockStorage = static_cast<MockPersistentStorage*>(daemon->persistentStorage());

    if (activated)
    {
        Q_ASSERT(daemon->startMonitoring().isEmpty());
        QCoreApplication::processEvents();
    }

    // Act
    daemon->stopMonitoring();
    QCoreApplication::processEvents();

    // Assert
    QVERIFY(!daemon->status().data().isMonitoringActivated);
}

void DaemonQtTest::fetchMailboxes_data()
{
    QTest::addColumn<MockMailClient::TestData>("mailClientTestData");
    QTest::addColumn<MockPersistentStorage::TestData>("storageData");
    QTest::addColumn<IMailClient::Configuration>("config");
    QTest::addColumn<bool>("expectedResultSuccess");
    QTest::addColumn<QStringList>("expectedMailboxes");

    {
        QTest::newRow("Happy path")
            << happyPathMailClientTestData()
            << happyPathStorageTestData()
            << Examples::validMailClientConfig()
            << true
            << Examples::validMailboxes();
    }

    {
        QTest::newRow("Invalid config")
            << happyPathMailClientTestData()
            << happyPathStorageTestData()
            << IMailClient::Configuration{}
            << false
            << QStringList{};
    }

    {
        MockPersistentStorage::TestData data = happyPathStorageTestData();
        data.readPassword = Result<QString>::error("error");

        QTest::newRow("PersistentStorage::readPassword failed")
            << happyPathMailClientTestData()
            << data
            << Examples::validMailClientConfig()
            << false
            << QStringList{};
    }

    {
        MockMailClient::TestData data = happyPathMailClientTestData();
        data.fetchMailboxes = Result<QStringList>::error("error");

        QTest::newRow("MailClient::fetchMailboxes failed")
            << data
            << happyPathStorageTestData()
            << Examples::validMailClientConfig()
            << false
            << QStringList{};
    }

    {
        MockMailClient::TestData data = happyPathMailClientTestData();
        data.fetchMailboxes = Result<QStringList>::success({});

        QTest::newRow("No mailboxes")
            << data
            << happyPathStorageTestData()
            << Examples::validMailClientConfig()
            << true
            << QStringList{};
    }
}

void DaemonQtTest::fetchMailboxes()
{
    // Assign
    QFETCH(MockMailClient::TestData, mailClientTestData);
    QFETCH(MockPersistentStorage::TestData, storageData);
    QFETCH(IMailClient::Configuration, config);
    QFETCH(bool, expectedResultSuccess);
    QFETCH(QStringList, expectedMailboxes);

    Daemon *daemon = createDaemon(
        storageData,
        mailClientTestData,
        happyPathNotificationManagerTestData()
    );
    RAIIDeleter deleter({daemon});

    MockPersistentStorage *mockStorage = static_cast<MockPersistentStorage*>(daemon->persistentStorage());

    // Act
    Result<QStringList> result = daemon->fetchMailboxes(config);

    // Assert
    QCOMPARE(result.success(), expectedResultSuccess);
    QVERIFY(result.success() ? true : mockStorage->m_testData.writeErrorLogMessageCalled);
    QCOMPARE(result.data(), expectedMailboxes);
}

void DaemonQtTest::nullptrModules_data()
{
    QTest::addColumn<MockMailClient*>("mailClient");
    QTest::addColumn<MockPersistentStorage*>("storage");
    QTest::addColumn<MockNotificationManager*>("manager");

    {
        MockMailClient *mailClient = nullptr;
        MockPersistentStorage *storage = new MockPersistentStorage();
        MockNotificationManager *manager = new MockNotificationManager();

        QTest::newRow("MailClient nullptr") << mailClient << storage << manager;
    }

    {
        MockMailClient *mailClient = new MockMailClient();
        MockPersistentStorage *storage = nullptr;
        MockNotificationManager *manager = new MockNotificationManager();

        QTest::newRow("PersistentStorage nullptr") << mailClient << storage << manager;
    }

    {
        MockMailClient *mailClient = new MockMailClient();
        MockPersistentStorage *storage = new MockPersistentStorage();
        MockNotificationManager *manager = nullptr;

        QTest::newRow("NotificationManager nullptr") << mailClient << storage << manager;
    }
}

void DaemonQtTest::nullptrModules()
{
    // Assign
    QFETCH(MockMailClient*, mailClient);
    QFETCH(MockPersistentStorage*, storage);
    QFETCH(MockNotificationManager*, manager);

    RAIIDeleter deleter({storage, mailClient, manager});

    Daemon daemon(mailClient, storage, manager);

    // Act
    bool isValid = daemon.isValid();
    QString setupResult = daemon.setup(Examples::validDaemonConfig());
    QString startMonitoringResult = daemon.startMonitoring();
    Result<QStringList> fetchMailboxesResult = daemon.fetchMailboxes(Examples::validMailClientConfig());

    // Assert
    QVERIFY(!isValid);
    QVERIFY(!setupResult.isEmpty());
    QVERIFY(!startMonitoringResult.isEmpty());
    QVERIFY(!fetchMailboxesResult.success());
}

void DaemonQtTest::onMailRequestTimerTimeout_data()
{
    // TODO: checkModulesNullptr() call is not covered with tests

    QTest::addColumn<MockPersistentStorage::TestData>("storageTestData");
    QTest::addColumn<MockMailClient::TestData>("mailClientTestData");
    QTest::addColumn<MockNotificationManager::TestData>("managerTestData");
    QTest::addColumn<OnMailRequestTimerResult>("expectedResult");

    {
        OnMailRequestTimerResult expectedResult;
        expectedResult.writeErrorLogMessageCalled = false;
        expectedResult.isMonitoringActivated = true;
        expectedResult.mailRequestFinishedSignalCount = c_lastMessageUIDFetchCount;
        // -1 because first time didn't count since last message UID would have not changed yet
        expectedResult.notificationSentSignalsResults = QList<bool>(c_lastMessageUIDFetchCount - 1, true);

        QTest::newRow("Happy path")
            << happyPathStorageTestData()
            << happyPathMailClientTestData()
            << happyPathNotificationManagerTestData()
            << expectedResult;
    }

    {
        MockMailClient::TestData testData;
        testData.fetchLastMessageUIDsQueue.clear();
        for (int i = 0; i < c_lastMessageUIDFetchCount; ++i)
        {
            LastMessageUIDs lastMessageUIDs;
            for (const QString &mailbox : Examples::validMailboxes())
            {
                lastMessageUIDs[mailbox] = 1;
            }
            testData.fetchLastMessageUIDsQueue.enqueue(Result<LastMessageUIDs>::success(lastMessageUIDs));
        }

        OnMailRequestTimerResult expectedResult;
        expectedResult.writeErrorLogMessageCalled = false;
        expectedResult.isMonitoringActivated = true;
        expectedResult.mailRequestFinishedSignalCount = c_lastMessageUIDFetchCount;
        expectedResult.notificationSentSignalsResults = {};

        QTest::newRow("Happy path - no new mesasges")
            << happyPathStorageTestData()
            << happyPathMailClientTestData()
            << happyPathNotificationManagerTestData()
            << expectedResult;
    }

    {
        MockMailClient::TestData testData = happyPathMailClientTestData();
        testData.fetchLastMessageUIDsQueue.clear();
        testData.fetchLastMessageUIDsQueue.enqueue(Result<LastMessageUIDs>::error("error"));

        OnMailRequestTimerResult expectedResult;
        expectedResult.writeErrorLogMessageCalled = true;
        expectedResult.isMonitoringActivated = false;
        expectedResult.mailRequestFinishedSignalCount = 1;
        expectedResult.notificationSentSignalsResults = { false };

        QTest::newRow("MailClient::fetchLastMessageUIDs failed")
            << happyPathStorageTestData()
            << testData
            << happyPathNotificationManagerTestData()
            << expectedResult;
    }

    {
        MockPersistentStorage::TestData testData = happyPathStorageTestData();
        testData.readLastMessageUIDs = Result<LastMessageUIDs>::error("error");

        OnMailRequestTimerResult expectedResult;
        expectedResult.writeErrorLogMessageCalled = true;
        expectedResult.isMonitoringActivated = false;
        expectedResult.mailRequestFinishedSignalCount = 1;
        expectedResult.notificationSentSignalsResults = { false };

        QTest::newRow("PersistentStorage::readLastMessageUIDs failed")
            << testData
            << happyPathMailClientTestData()
            << happyPathNotificationManagerTestData()
            << expectedResult;
    }

    {
        MockPersistentStorage::TestData testData = happyPathStorageTestData();
        testData.writeLastMessageUIDs = "error";

        OnMailRequestTimerResult expectedResult;
        expectedResult.writeErrorLogMessageCalled = true;
        expectedResult.isMonitoringActivated = false;
        expectedResult.mailRequestFinishedSignalCount = 1;
        expectedResult.notificationSentSignalsResults = { false };

        QTest::newRow("PersistentStorage::writeLastMessageUIDs failed")
            << testData
            << happyPathMailClientTestData()
            << happyPathNotificationManagerTestData()
            << expectedResult;
    }
}

void DaemonQtTest::onMailRequestTimerTimeout()
{
    // Assign
    QFETCH(MockPersistentStorage::TestData, storageTestData);
    QFETCH(MockMailClient::TestData, mailClientTestData);
    QFETCH(MockNotificationManager::TestData, managerTestData);
    QFETCH(OnMailRequestTimerResult, expectedResult);

    Daemon *daemon = createDaemon(
        storageTestData,
        mailClientTestData,
        managerTestData
    );
    RAIIDeleter deleter({daemon});

    MockPersistentStorage *mockStorage = static_cast<MockPersistentStorage*>(daemon->persistentStorage());

    QSignalSpy mailRequestFinishedSpy(daemon, &Daemon::mailRequestFinished);
    QSignalSpy sendNotificationSpy(daemon, &Daemon::sendNotification);

    daemon->startMonitoring();

    // Act
    // Timer is working by itself after startMonitoring() calling...
    QCoreApplication::processEvents();

    // Assert
    QCOMPARE(daemon->status().data().isMonitoringActivated, expectedResult.isMonitoringActivated);
    QCOMPARE(expectedResult.writeErrorLogMessageCalled, mockStorage->m_testData.writeErrorLogMessageCalled);
    QVERIFY(checkMailRequestFinishedSignals(expectedResult, mailRequestFinishedSpy));
    QVERIFY(checkSendNotificationSignals(expectedResult, sendNotificationSpy));
}

void DaemonQtTest::setupInConstructor_data()
{
    QTest::addColumn<MockPersistentStorage::TestData>("storageTestData");
    QTest::addColumn<IDaemon::Configuration>("config");

    {
        QTest::newRow("Happy path") << happyPathStorageTestData() << Examples::validDaemonConfig();
    }

    {
        MockPersistentStorage::TestData testData;
        testData.readDaemonConfiguration = Result<IDaemon::Configuration>::error("error");

        QTest::newRow("PersistentStorage::readDaemonConfiguration failed") << testData << IDaemon::Configuration{};
    }
}

void DaemonQtTest::setupInConstructor()
{
    // Assign
    QFETCH(MockPersistentStorage::TestData, storageTestData);
    QFETCH(IDaemon::Configuration, config);

    Daemon *daemon = nullptr;
    RAIIDeleter deleter({daemon});

    // Act
    daemon = createDaemon(
        storageTestData,
        happyPathMailClientTestData(),
        happyPathNotificationManagerTestData()
    );

    // Assert
    QCOMPARE(daemon->status().data().configuration, config);

    MockPersistentStorage *mockStorage = static_cast<MockPersistentStorage*>(daemon->persistentStorage());
    QVERIFY(!mockStorage->m_writeDaemonConfigurationCalled);
    QVERIFY(!mockStorage->m_writeLastMessageUIDsCalled);
}

MockPersistentStorage::TestData DaemonQtTest::happyPathStorageTestData() const
{
    MockPersistentStorage::TestData testData;
    testData.isValid = true;
    testData.readDaemonConfiguration = Result<IDaemon::Configuration>::success(Examples::validDaemonConfig());
    testData.writeDaemonConfiguration = "";
    testData.readLastMessageUIDs = Result<LastMessageUIDs>::success(Examples::validLastMessageUIDs());
    testData.writeLastMessageUIDs = "";
    testData.readErrorLogMessage = Result<Message>::success(Examples::validErrorLogMessage());
    testData.writeErrorLogMessage = "";
    testData.writeErrorLogMessageCalled = false;
    testData.readPassword = Result<QString>::success(Examples::validMailClientConfig().password);
    testData.writePassword = "";
    return testData;
}

MockMailClient::TestData DaemonQtTest::happyPathMailClientTestData() const
{
    MockMailClient::TestData testData;
    testData.isValid = true;
    testData.configuration = Examples::validMailClientConfig();
    testData.fetchMailboxes = Result<QStringList>::success(Examples::validMailboxes());
    for (int i = 0; i < c_lastMessageUIDFetchCount; ++i)
    {
        LastMessageUIDs lastMessageUIDs;
        for (const QString &mailbox : Examples::validMailboxes())
        {
            lastMessageUIDs[mailbox] = i + 1;
        }
        testData.fetchLastMessageUIDsQueue.enqueue(Result<LastMessageUIDs>::success(lastMessageUIDs));
    }
    return testData;
}

MockNotificationManager::TestData DaemonQtTest::happyPathNotificationManagerTestData() const
{
    MockNotificationManager::TestData testData;
    testData.isValid = true;
    testData.sendNotification = true;
    return testData;
}

Daemon *DaemonQtTest::createDaemon(const MockPersistentStorage::TestData &storageTestData, const MockMailClient::TestData &mailClientTestData, const MockNotificationManager::TestData &managerTestData) const
{
    MockPersistentStorage *storage = new MockPersistentStorage();
    MockMailClient *mailClient = new MockMailClient();
    MockNotificationManager *manager = new MockNotificationManager();

    storage->m_testData = storageTestData;
    mailClient->m_testData = mailClientTestData;
    manager->m_testData = managerTestData;

    Daemon *daemon = new Daemon(mailClient, storage, manager);

    storage->setParent(daemon);
    mailClient->setParent(daemon);
    manager->setParent(daemon);

    return daemon;
}

bool DaemonQtTest::checkMailRequestFinishedSignals(const OnMailRequestTimerResult &result, QSignalSpy &spy) const
{
    if (result.mailRequestFinishedSignalCount == 0)
    {
        return true;
    }

    while (spy.count() < result.mailRequestFinishedSignalCount)
    {
        if (!spy.wait(c_mailRequestIntervalMs))
        {
            logCritical() << "checkMailRequestFinishedSignals: !spy.wait(c_mailRequestIntervalMs)";
            return false;
        }
    }
    return true;
}

bool DaemonQtTest::checkSendNotificationSignals(const OnMailRequestTimerResult &result, QSignalSpy &spy) const
{
    QList<bool> signalResults = result.notificationSentSignalsResults;

    if (signalResults.isEmpty())
    {
        return true;
    }

    while (spy.count() < signalResults.size())
    {
        if (!spy.wait(c_mailRequestIntervalMs))
        {
            logCritical() << "checkSendNotificationSignals: !spy.wait(c_mailRequestIntervalMs)";
            return false;
        }
    }

    if (signalResults.size() > spy.size())
    {
        logCritical() << QString("checkSendNotificationSignals: signalResults(=%1) > spy.size()(=%2)")
                           .arg(signalResults.size())
                           .arg(spy.size());
        return false;
    }

    QList<bool> actualResults;
    for (int i = 0; i < signalResults.size(); ++i)
    {
        bool signalSuccessArg = spy.at(i).at(0).toBool();
        actualResults.append(signalSuccessArg);
    }

    bool equal = signalResults == actualResults;
    if (!equal)
    {
        logCritical() << "checkSendNotificationSignals: signalResults != actualResults";
    }
    return equal;
}

QTEST_GUILESS_MAIN(DaemonQtTest)

#include "tst_daemon.moc"

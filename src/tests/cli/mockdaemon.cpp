#include "mockdaemon.h"

bool MockDaemon::isValid() const
{
    return m_testData.isValid;
}

Result<IDaemon::Status> MockDaemon::status() const
{
    return m_testData.status;
}

QString MockDaemon::setup(const Configuration &config)
{
    return m_testData.setup;
}

QString MockDaemon::startMonitoring()
{
    return m_testData.startMonitoring;
}

QString MockDaemon::stopMonitoring()
{
    return m_testData.stopMonitoring;
}

Result<QStringList> MockDaemon::fetchMailboxes()
{
    return m_testData.fetchMailboxes;
}

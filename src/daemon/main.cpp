#include "daemon.h"
#ifdef USE_MOCK_MAIL_CLIENT
#include "mockmailclient.h"
#else
#include "imapclient.h"
#endif
#include "notificationmanager.h"
#include "persistentstorage.h"
#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QtDBus/QtDBus>

int main(int argc, char *argv[])
{
    qSetMessagePattern(
        "%{if-category}[%{category}] %{endif}"
        "%{if-debug}[Debug][%{function}:%{line}] %{endif}"
        "%{if-warning}[Warning] %{endif}"
        "%{if-critical}[Critical] %{endif}"
        "%{message}"
    );

    QCoreApplication app(argc, argv);
    app.setApplicationName(DAEMON_NAME_STRING_LITERAL);
    app.setOrganizationName(AUTHOR_EMAIL_STRING_LITERAL);
    app.setApplicationVersion(PROGRAM_VERSION_STRING_LITERAL);

#ifdef USE_MOCK_MAIL_CLIENT
    MockMailClient mailClient;
#else
    IMAPClient mailClient;
#endif

    PersistentStorage storage;
    NotificationManager notificationMngr;

    Daemon daemon(&mailClient, &storage, &notificationMngr);
    daemon.registerDBusService();

    notificationMngr.registerDBusInterface();

    if (daemon.isSetUp())
    {
        daemon.startMonitoring();
    }

    return app.exec();
}

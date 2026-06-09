#pragma once

#include <QMap>

// key - mailbox, value - mailbox last message UID
typedef QMap<QString, quint64> LastMessageUIDs;

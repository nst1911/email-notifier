#pragma once

#include <QtGlobal>

#ifdef BUILD_COMMON_LIB
#define COMMON_VISIBILITY Q_DECL_EXPORT
#else
#define COMMON_VISIBILITY Q_DECL_IMPORT
#endif

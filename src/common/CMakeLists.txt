cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME common)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC
    basemailclient.cpp
    basemailclient.h
    basepersistentstorage.cpp
    basepersistentstorage.h
    commandlineinterface.cpp
    commandlineinterface.h
    daemon.cpp
    daemon.h
    dbus.cpp
    dbus.h
    export.h
    interfaces/icommandlineinterface.h
    interfaces/idaemon.cpp
    interfaces/idaemon.h
    interfaces/imailclient.cpp
    interfaces/imailclient.h
    interfaces/imodule.h
    interfaces/inotificationmanager.h
    interfaces/ipasswordinputhandler.h
    interfaces/ipersistentstorage.h
    jsonhelper.cpp
    jsonhelper.h
    log.h
    message.cpp
    message.h
    notification.cpp
    notification.h
    persistentstorage.cpp
    persistentstorage.h
    result.h
)

target_compile_definitions(${PROJECT_NAME} PRIVATE BUILD_COMMON_LIB)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt6 REQUIRED COMPONENTS Core DBus)
find_package(Qt6Keychain REQUIRED)
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::DBus Qt6Keychain::Qt6Keychain)

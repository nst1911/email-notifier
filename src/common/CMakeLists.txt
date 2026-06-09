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
    configuration.cpp
    configuration.h
    daemon.cpp
    daemon.h
    dbus.cpp
    dbus.h
    export.h
    interfaces/idaemon.h
    interfaces/imailclient.h
    interfaces/imodule.h
    interfaces/inotificationmanager.h
    interfaces/ipasswordinputhandler.h
    interfaces/ipersistentstorage.h
    lastmessageuids.h
    message.cpp
    message.h
    result.h
    jsonhelper.h
    jsonhelper.cpp
    log.h
    notification.h notification.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE BUILD_COMMON_LIB)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Qt6 REQUIRED COMPONENTS Core DBus)
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::DBus)

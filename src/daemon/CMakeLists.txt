cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME ${DAEMON_NAME})

project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0" CACHE STRING "gdb flags" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MAKESELF_OUTPUT_DIR})

# Uncomment it to build email-notifier with mock mail client. It can be used for debugging
add_compile_definitions(USE_MOCK_MAIL_CLIENT)

add_executable(${PROJECT_NAME}
  main.cpp
  imapclient.h
  imapclient.cpp
  notificationmanager.h
  notificationmanager.cpp
  persistentstorage.h
  persistentstorage.cpp
  mockmailclient.h 
  mockmailclient.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Core DBus)
find_package(CURL REQUIRED)
find_package(Qt6Keychain REQUIRED)

target_link_libraries(${PROJECT_NAME}
  common
  Qt6::Core
  Qt6::DBus
  CURL::libcurl
  Qt6Keychain::Qt6Keychain
)


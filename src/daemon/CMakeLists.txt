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
# add_compile_definitions(USE_MOCK_MAIL_CLIENT)

add_executable(${PROJECT_NAME}
  main.cpp
  imapclient.h
  imapclient.cpp
  notificationmanager.h
  notificationmanager.cpp
  mockmailclient.h 
  mockmailclient.cpp
)

find_package(CURL REQUIRED)
target_link_libraries(${PROJECT_NAME}
  common
  CURL::libcurl
)


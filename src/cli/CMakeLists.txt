cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME ${CLI_NAME})

project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MAKESELF_OUTPUT_DIR})

add_executable(${PROJECT_NAME}
  main.cpp
  daemondbusinterface.h
  daemondbusinterface.cpp
  passwordinputhandler.h
  passwordinputhandler.cpp
)

find_package(Qt6 REQUIRED COMPONENTS Core DBus)
target_link_libraries(${PROJECT_NAME} common Qt6::Core Qt6::DBus)

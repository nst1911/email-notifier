cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME base-persistent-storage-test)

project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
  tst_basepersistentstorage.cpp
  testbasepersistentstorage.h
  testbasepersistentstorage.cpp
  ${TESTS_COMMON_SOURCE_DIR}/examples.h
)

target_include_directories(${PROJECT_NAME} PRIVATE ${TESTS_COMMON_SOURCE_DIR})

find_package(Qt6 REQUIRED COMPONENTS
  Core
  DBus
  Test
)
target_link_libraries(${PROJECT_NAME}
  common
  Qt6::Core
  Qt6::DBus
  Qt6::Test
)

cmake_minimum_required(VERSION 3.16)

project(tests)

set(TESTS_COMMON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)

add_subdirectory(base-mail-client)
add_subdirectory(base-persistent-storage)
add_subdirectory(daemon)
add_subdirectory(cli)

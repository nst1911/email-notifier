cmake_minimum_required(VERSION 3.16)

set(QT_CREATOR_SKIP_MAINTENANCE_TOOL_PROVIDER ON)

set(PROGRAM_NAME email-notifier)
project(${PROGRAM_NAME})

set(PROGRAM_VERSION 0.2)
set(AUTHOR_EMAIL nst1911@yandex.ru)
set(DAEMON_NAME email-notifier-daemon)
set(CLI_NAME ${PROGRAM_NAME})
set(UNINSTALLER_NAME ${PROGRAM_NAME}-uninstaller)
set(SERVICE_NAME com.github.nst1911.emailnotifier)
set(DEPS "libqt6core6 libqt6sql6 libcurl4 libqt6keychain1")

set(MAKESELF_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/makeself)
set(ROOT_SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(COMMON_SOURCE_DIR   ${ROOT_SOURCE_DIR}/common)
set(DAEMON_SOURCE_DIR   ${ROOT_SOURCE_DIR}/daemon)
set(CLI_SOURCE_DIR      ${ROOT_SOURCE_DIR}/cli)
set(TESTS_SOURCE_DIR    ${ROOT_SOURCE_DIR}/tests)
set(MAKESELF_OUTPUT_DIR ${CMAKE_BINARY_DIR}/makeself)

add_compile_definitions(PROGRAM_NAME_STRING_LITERAL=\"${PROGRAM_NAME}\")
add_compile_definitions(DAEMON_NAME_STRING_LITERAL=\"${DAEMON_NAME}\")
add_compile_definitions(CLI_NAME_LITERAL=\"${CLI_NAME}\")
add_compile_definitions(PROGRAM_VERSION_STRING_LITERAL=\"${PROGRAM_VERSION}\")
add_compile_definitions(AUTHOR_EMAIL_STRING_LITERAL=\"${AUTHOR_EMAIL}\")
add_compile_definitions(SERVICE_NAME_STRING_LITERAL=\"${SERVICE_NAME}\")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

add_subdirectory(${COMMON_SOURCE_DIR})
add_subdirectory(${DAEMON_SOURCE_DIR})
add_subdirectory(${CLI_SOURCE_DIR})
add_subdirectory(${TESTS_SOURCE_DIR})

set(ARCHIVE_DIR_ARG \"${MAKESELF_OUTPUT_DIR}\")
set(FILE_NAME_ARG \"${PROGRAM_NAME}_${PROGRAM_VERSION}_install.run\")
set(LABEL_ARG \"\") # empty string
set(STARTUP_SCRIPT_ARG \"./install.sh\")

add_custom_target(makeself_create
  ALL
  DEPENDS ${DAEMON_NAME} ${CLI_NAME}
  # Copy files (scripts, service, bash completion) to ${MAKESELF_OUTPUT_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy ${MAKESELF_DIR}/install.sh ${MAKESELF_OUTPUT_DIR}/
  COMMAND ${CMAKE_COMMAND} -E copy ${MAKESELF_DIR}/uninstall.sh ${MAKESELF_OUTPUT_DIR}/${UNINSTALLER_NAME}
  COMMAND ${CMAKE_COMMAND} -E copy ${MAKESELF_DIR}/${SERVICE_NAME}.service ${MAKESELF_OUTPUT_DIR}/
  COMMAND ${CMAKE_COMMAND} -E copy ${MAKESELF_DIR}/bash_completion ${MAKESELF_OUTPUT_DIR}/
  # Substitute variables in scripts and .service file
  COMMAND ${CMAKE_COMMAND} -DPROGRAM_NAME="${PROGRAM_NAME}"
                           -DPROGRAM_VERSION="${PROGRAM_VERSION}"
                           -DCLI_NAME="${CLI_NAME}"  
                           -DDAEMON_NAME="${DAEMON_NAME}"
                           -DUNINSTALLER_NAME="${UNINSTALLER_NAME}"  
                           -DSERVICE_NAME="${SERVICE_NAME}" 
                           "-DDEPS=${DEPS}" # quotes to prevent cmake escape spaces with slashes
                           -DMAKESELF_OUTPUT_DIR="${MAKESELF_OUTPUT_DIR}"
                           -P ${MAKESELF_DIR}/substitute_variables.cmake
  # Creating self-extractable archive archive with makeself (for build time)
  COMMAND chmod +x "${MAKESELF_OUTPUT_DIR}/install.sh"
  COMMAND makeself --tar-format posix ${ARCHIVE_DIR_ARG} ${FILE_NAME_ARG} ${LABEL_ARG} ${STARTUP_SCRIPT_ARG}
)

# Prepend variables to script files
message("Substitute variables to scripts and .service files...")
set(PROGRAM_VERSION 0.1)
set(AUTHOR_EMAIL nst1911@yandex.ru)
set(DAEMON_NAME email-notifier-daemon)
set(CLI_NAME ${PROGRAM_NAME})
set(UNINSTALLER_NAME ${PROGRAM_NAME}-uninstaller)
set(SERVICE_NAME com.github.nst1911.emailnotifier)
set(DEPS "libqt6core6 libqt6sql6 libcurl4 libqt6keychain1")

set(SCRIPT_HEADER "#!/bin/bash
PROGRAM_NAME=\"${PROGRAM_NAME}\"
PROGRAM_VERSION=\"${PROGRAM_VERSION}\"
DAEMON_NAME=\"${DAEMON_NAME}\"
CLI_NAME=\"${CLI_NAME}\"
UNINSTALLER_NAME=\"${UNINSTALLER_NAME}\"
SERVICE_NAME=\"${SERVICE_NAME}\"
DEPS=\"${DEPS}\"
")

function(prepend_script_header FILE_PATH)
  file(READ "${FILE_PATH}" SCRIPT_CONTENT)
  file(WRITE "${FILE_PATH}" "${SCRIPT_HEADER}\n${SCRIPT_CONTENT}")
endfunction()
prepend_script_header("${MAKESELF_OUTPUT_DIR}/install.sh")
prepend_script_header("${MAKESELF_OUTPUT_DIR}/${UNINSTALLER_NAME}")

# Substitute variables in .service file
file(READ "${MAKESELF_OUTPUT_DIR}/${SERVICE_NAME}.service" SERVICE_FILE_CONTENT)
string(REPLACE "<PROGRAM_NAME>" "${PROGRAM_NAME}" SERVICE_FILE_CONTENT "${SERVICE_FILE_CONTENT}")
string(REPLACE "<SERVICE_NAME>" "${SERVICE_NAME}" SERVICE_FILE_CONTENT "${SERVICE_FILE_CONTENT}")
string(REPLACE "<DAEMON_NAME>" "${DAEMON_NAME}" SERVICE_FILE_CONTENT "${SERVICE_FILE_CONTENT}")
file(WRITE "${MAKESELF_OUTPUT_DIR}/${SERVICE_NAME}.service" "${SERVICE_FILE_CONTENT}")

set(SHARED_TARGET_NAME "Shared")

add_library("${SHARED_TARGET_NAME}" INTERFACE)

add_library("${PROJECT_NAME}::${SHARED_TARGET_NAME}" ALIAS "${SHARED_TARGET_NAME}")

target_include_directories("${SHARED_TARGET_NAME}"
INTERFACE
    "include"
)

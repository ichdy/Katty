set(SERVER_TARGET_NAME "Server")

add_executable("${SERVER_TARGET_NAME}"
    "src/main.cpp"
    "src/mainwindow.cpp"
    "src/engine.cpp"
    "src/server.cpp"
    "src/mainwindow.h"
    "src/engine.h"
    "src/server.h"
)

add_executable("${PROJECT_NAME}::${SERVER_TARGET_NAME}" ALIAS "${SERVER_TARGET_NAME}")

target_link_libraries("${SERVER_TARGET_NAME}"
PUBLIC
    Katty::Shared
    Qt5::Core
    Qt5::Gui
    Qt5::Sql
    Qt5::WebSockets
    Qt5::Widgets
)

target_compile_definitions("${SERVER_TARGET_NAME}"
PRIVATE
    "QT_DEPRECATED_WARNINGS"
    "QT_DISABLE_DEPRECATED_BEFORE=0x060000"
)

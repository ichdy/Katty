set(CLIENT_TARGET_NAME "Client")

add_executable("${CLIENT_TARGET_NAME}"
    "src/main.cpp"
    "src/client.cpp"
    "src/mainwindow.cpp"
    "src/engine.cpp"
    "src/loginpage.cpp"
    "src/mainpage.cpp"
    "src/useritemdelegate.cpp"
    "src/chatedit.cpp"
    "src/chatwidget.cpp"
    "src/chatitemdelegate.cpp"
    "src/chatview.cpp"
    "src/loadingoverlay.cpp"
    "src/mainwindow.h"
    "src/client.h"
    "src/engine.h"
    "src/loginpage.h"
    "src/mainpage.h"
    "src/shareddata.h"
    "src/useritemdelegate.h"
    "src/chatedit.h"
    "src/chatwidget.h"
    "src/chatitemdelegate.h"
    "src/chatview.h"
    "src/loadingoverlay.h"
    "res/Client.rc"
    "res/Client.qrc"
)

add_executable("${PROJECT_NAME}::${CLIENT_TARGET_NAME}" ALIAS "${CLIENT_TARGET_NAME}")

target_link_libraries("${CLIENT_TARGET_NAME}"
PUBLIC
    Katty::Shared
    Qt5::Core
    Qt5::Gui
    Qt5::WebSockets
    Qt5::Widgets
)

target_compile_definitions("${CLIENT_TARGET_NAME}"
PRIVATE
    "QT_DEPRECATED_WARNINGS"
    "QT_DISABLE_DEPRECATED_BEFORE=0x060000"
)

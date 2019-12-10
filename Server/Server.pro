#-------------------------------------------------
#
# Project created by QtCreator 2018-05-02T14:46:51
#
#-------------------------------------------------

QT       += core gui websockets sql widgets

TARGET = Katty-Server
TEMPLATE = app
DESTDIR = $$OUT_PWD/../build

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/../include

HEADERS += \
    ../include/define.h \
    src/mainwindow.h \
    src/engine.h \
    src/server.h

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/engine.cpp \
    src/server.cpp

#-------------------------------------------------
#
# Project created by QtCreator 2016-06-14T11:16:34
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ServerEsp8266
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    mainwindow.cpp \
    errorlist.cpp \
    log.cpp \
    logfileread.cpp \
    arduinopanel.cpp

HEADERS  += widget.h \
    mainwindow.h \
    errorlist.h \
    log.h \
    logfileread.h \
    arduinopanel.h

FORMS    += widget.ui \
    mainwindow.ui \
    errorlist.ui \
    logfileread.ui \
    arduinopanel.ui

RESOURCES += \
    img/img.qrc

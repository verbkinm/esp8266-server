#-------------------------------------------------
#
# Project created by QtCreator 2016-06-14T11:16:34
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ServerEsp8266
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    mainwindow.cpp

HEADERS  += widget.h \
    mainwindow.h

FORMS    += widget.ui \
    mainwindow.ui

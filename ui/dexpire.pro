#-------------------------------------------------
#
# Project created by QtCreator 2014-09-04T15:12:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dexpire
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += \
    icons.qrc

win32 {
    LIBS += -LH:\Projects\dexpire\build\Debug -ldexpire
}

INCLUDEPATH += ..\src

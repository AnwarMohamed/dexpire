#-------------------------------------------------
#
# Project created by QtCreator 2014-09-04T15:12:12
#
#-------------------------------------------------

QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dexpire
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    treeitem.cpp \
    treemodel.cpp \
    codeeditorlinenumber.cpp \
    codeeditor.cpp

HEADERS  += mainwindow.h \
    treeitem.h \
    treemodel.h \
    codeeditorlinenumber.h \
    codeeditor.h \
    htmldelegate.h

FORMS    += mainwindow.ui

RESOURCES += \
    icons.qrc

win32 {
    LIBS += H:/Projects/dexpire/build/Debug/dexpire.lib
}

INCLUDEPATH += ..\src

OTHER_FILES += \
    default.txt

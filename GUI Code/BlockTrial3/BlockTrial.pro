#-------------------------------------------------
#
# Project created by QtCreator 2016-06-30T10:18:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BlockTrial
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
#        dragwidget.cpp
    ledlabel.cpp \
    behaviorwindow.cpp\
    LightParameter.cpp

HEADERS  += mainwindow.h\
#         dragwidget.h
    ledlabel.h \
    behaviorwindow.h\
    LightParameter.h\

RESOURCES   = draggableicons.qrc

FORMS    += mainwindow.ui \
    behaviorwindow.ui


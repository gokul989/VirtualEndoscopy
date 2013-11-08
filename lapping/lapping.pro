#-------------------------------------------------
#
# Project created by QtCreator 2013-08-20T13:30:39
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = lapping
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp \
    trackball.cpp \
    glextensions.cpp \
    3deng.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    trackball.h \
    glextensions.h \
    3deng.h \
    curves.h

FORMS    += mainwindow.ui

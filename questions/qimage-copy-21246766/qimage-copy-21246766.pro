#-------------------------------------------------
#
# Project created by QtCreator 2014-01-21T02:38:46
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets
  CONFIG += c++11
}

INCLUDEPATH += /opt/local/include
LIBS += -L /opt/local/lib -lopencv_core

TARGET = qimage-copy-21246766

TEMPLATE = app


SOURCES += main.cpp

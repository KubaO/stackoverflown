QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = gview-scroll
TEMPLATE = app
macx {
    LIBS += -framework Foundation
    OBJECTIVE_SOURCES += helper.m
}
SOURCES += main.cpp

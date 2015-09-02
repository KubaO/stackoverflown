QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
macx: QMAKE_CXXFLAGS_X86_64 += -mmacosx-version-min=10.7

TARGET = stylesize-21316514

TEMPLATE = app

SOURCES += main.cpp

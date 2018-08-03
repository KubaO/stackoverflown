QT += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app
SOURCES += main.cpp
exists(../../tooling/tooling.pri): include(../../tooling/tooling.pri)

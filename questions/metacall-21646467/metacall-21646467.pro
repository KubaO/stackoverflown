QT       += core core_private
QT       -= gui
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11

TARGET = metacall-21646467
CONFIG   += console c++14
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

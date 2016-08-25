QT = widgets
CONFIG += c++11
TEMPLATE = app
SOURCES += main.cpp
LIBS += -L../lib1 -llib1 -L../lib2 -llib2
INCLUDEPATH += ..
DEPENDPATH += ..

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

SDL = -F/Library/Frameworks
QMAKE_CXXFLAGS += $$SDL
LIBS += $$SDL -framework SDL

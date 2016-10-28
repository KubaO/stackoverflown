CONFIG += console c++11
CONFIG -= qt app_bundle
TARGET = mmap-boost-40308164
TEMPLATE = app
SOURCES += main.cpp
macx { # boost from macports
   LIBS += -L /opt/local/lib -lboost_filesystem-mt -lboost_system-mt
   INCLUDEPATH += /opt/local/include
}

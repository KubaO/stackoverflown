###main/main.pro
if (true) {
   # Use demand-loaded lib1
   DEFINES += DEMAND_LOAD_LIB1
   LIBS += -L../lib1_demand -llib1_demand
} else {
   # Use direct-loaded lib1
   LIBS += -L../lib1 -llib1
}
QT = core
CONFIG += console c++11
CONFIG -= app_bundle
TARGET = demand-load-39291032
TEMPLATE = app
INCLUDEPATH += ..
DEPENDPATH += ..
SOURCES = main.cpp

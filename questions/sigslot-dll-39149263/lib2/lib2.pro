QT = widgets
CONFIG += c++11
TEMPLATE = lib
HEADERS += lib2.h
win32:DEFINES += LIB2_EXPORT=__declspec(dllexport)
LIBS += -L../lib1 -llib1
INCLUDEPATH += ..
DEPENDPATH += ..

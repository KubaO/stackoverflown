QT = widgets
CONFIG += c++11
TEMPLATE = lib
HEADERS += lib1.h
win32:DEFINES += LIB1_EXPORT=__declspec(dllexport)

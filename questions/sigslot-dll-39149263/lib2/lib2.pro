QT = widgets
CONFIG += c++11
TEMPLATE = lib
HEADERS += lib2.h
win32:DEFINES += LIB2_EXPORT=__declspec(dllexport)
win32:CONFIG(debug,release|debug) LIBSUBPATH=/debug
win32:CONFIG(release,release|debug) LIBSUBPATH=/release
LIBS += -L../lib1$$LIBSUBPATH -llib1
INCLUDEPATH += ..
DEPENDPATH += ..

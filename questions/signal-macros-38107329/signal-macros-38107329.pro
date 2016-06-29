QT = core 
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += c++11
} else {
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    }
}
CONFIG -= app_bundle
TARGET = signal-macros-38107329
TEMPLATE = app
SOURCES += main.cpp

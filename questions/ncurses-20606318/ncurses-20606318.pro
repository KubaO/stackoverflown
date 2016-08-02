QT = core
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
    CONFIG += c++11
} else {
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    }
}
LIBS += -lncurses
TARGET = ncurses-20606318
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += main.cpp

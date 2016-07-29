QT = gui
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
TARGET = button-grid-22641306
TEMPLATE = app
SOURCES += main.cpp

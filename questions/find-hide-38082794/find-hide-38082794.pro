greaterThan(QT_MAJOR_VERSION, 4) {
    QT = widgets
} else {
    QT = gui
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    }
}
CONFIG += c++11
TARGET = find-hide-38082794
TEMPLATE = app
SOURCES += main.cpp

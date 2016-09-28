greaterThan(QT_MAJOR_VERSION, 4) {
    QT = widgets
    CONFIG += c++11
} else {
    QT = gui
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    }
}
QT += opengl
TARGET = opengl-viewport-val-39750134
TEMPLATE = app
SOURCES += main.cpp

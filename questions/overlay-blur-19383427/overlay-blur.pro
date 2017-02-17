greaterThan(QT_MAJOR_VERSION, 4) {
    QT = widgets
    CONFIG += c++14
} else {
    QT = gui
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    }
}
TARGET = overlay-widget-19362455
TEMPLATE = app
SOURCES += main.cpp

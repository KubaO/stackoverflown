# scene-movable-circles-11188261.pro
greaterThan(QT_MAJOR_VERSION, 4) {
    QT = widgets 
    CONFIG += c++11
} else {
    QT = gui 
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
        QMAKE_CXXFLAGS_WARN_ON += -Wno-inconsistent-missing-override
    }
}
TARGET = scene-movable-circles-11188261
TEMPLATE = app
SOURCES += main.cpp
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050800

greaterThan(QT_MAJOR_VERSION, 4) {
    QT = widgets
    CONFIG += c++17
} else {
    QT = gui
    unix:QMAKE_CXXFLAGS += -std=c++17
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    }
}
SOURCES += main.cpp

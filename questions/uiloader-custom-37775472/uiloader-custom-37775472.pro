greaterThan(QT_MAJOR_VERSION, 4) {
    QT = widgets uitools
    CONFIG += c++11
} else {
    QT = gui
    CONFIG += uitools
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    }
}
TARGET = uiloader-custom-37775472
TEMPLATE = app
SOURCES += main.cpp

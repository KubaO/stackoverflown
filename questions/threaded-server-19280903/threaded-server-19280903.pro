greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += c++11
} else {
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
        QMAKE_CXXFLAGS_WARN_ON += -Wno-inconsistent-missing-override
    }
}
QT = network
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050800
TARGET = threaded-server-19280903
TEMPLATE = app
SOURCES += main.cpp

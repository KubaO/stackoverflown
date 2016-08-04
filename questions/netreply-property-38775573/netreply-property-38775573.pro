QT = network
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += c++11
} else {
    QT += gui
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
    }
}
CONFIG -= app_bundle
TARGET = netreply-property-38775573
TEMPLATE = app
SOURCES += main.cpp

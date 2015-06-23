QT       += gui svg xml network

greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += c++11
    QT += widgets concurrent
}

TARGET = svg-thread-process-30983142

TEMPLATE = app

SOURCES += main.cpp

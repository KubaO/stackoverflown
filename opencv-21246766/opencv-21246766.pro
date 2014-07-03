QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++11

TARGET = opencv-21246766

TEMPLATE = app

SOURCES += main.cpp

LIBS += -L /opt/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui

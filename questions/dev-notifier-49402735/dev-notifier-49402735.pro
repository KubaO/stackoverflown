CONFIG += console c++14
CONFIG -= app_bundle
QT = concurrent core_private
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050800
macx: INCLUDEPATH += /opt/local/include
TARGET = dev-notifier-49402735
TEMPLATE = app
SOURCES += main.cpp

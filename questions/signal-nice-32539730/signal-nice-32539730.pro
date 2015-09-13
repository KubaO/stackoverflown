QT = core
CONFIG += c++11 console
CONFIG -= app-bundle
TARGET = signal-nice-32539730
TEMPLATE = app
SOURCES += main.cpp \
    posixsignalproxy.cpp
HEADERS += \
    posixsignalproxy.h

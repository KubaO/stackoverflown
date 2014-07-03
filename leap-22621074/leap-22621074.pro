#-------------------------------------------------
#
# Project created by QtCreator 2014-03-25T12:48:22
#
#-------------------------------------------------

QT       += core widgets

TARGET = leap-22621074

TEMPLATE = app

SOURCES += main.cpp

macx0 {
    LEAPSDK = $$(HOME)/src/LeapDeveloperKit/LeapSDK
    leaplib.name = libLeap.dylib
    leaplib.srcpath = $$LEAPSDK/lib
    leaplib.src = $${leaplib.srcpath}/$${leaplib.name}
    leaplib.dst = $$OUT_PWD/$${TARGET}.app/Contents/MacOS
    leaplib.target = $${leaplib.dst}/$${leaplib.name}
    leaplib.commands = "$$QMAKE_MKDIR $${leaplib.dst} && $${QMAKE_COPY_FILE} $${leaplib.src} $${leaplib.target}"

    INCLUDEPATH += $$LEAPSDK/include
    LIBS += -L$$LEAPSDK/lib -lleap

    PRE_TARGETDEPS += $${leaplib.target}
    QMAKE_EXTRA_TARGETS += leaplib
}

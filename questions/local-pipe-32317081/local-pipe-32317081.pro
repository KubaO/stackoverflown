# local-pipe-32317081.pro
QT = core
greaterThan(QT_MAJOR_VERSION, 4): QT = core-private testlib
else: CONFIG += qtestlib
DEFINES += \
  QT_DEPRECATED_WARNINGS \
  QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
  QT_RESTRICTED_CAST_FROM_ASCII
CONFIG += console c++14
CONFIG -= app_bundle
TEMPLATE = app
SOURCES = main.cpp

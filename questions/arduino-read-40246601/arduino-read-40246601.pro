QT = serialport
#QT += widgets
CONFIG += c++14
!contains(QT, widgets) {
    CONFIG += console
    CONFIG -= app_bundle
}
DEFINES += \
  QT_DEPRECATED_WARNINGS \
  QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
  QT_RESTRICTED_CAST_FROM_ASCII
SOURCES += main.cpp

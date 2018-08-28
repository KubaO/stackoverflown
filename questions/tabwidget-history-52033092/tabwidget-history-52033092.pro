QT = widgets
CONFIG += c++14
SOURCES += main.cpp
DEFINES += \
  QT_DEPRECATED_WARNINGS \
  QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
  QT_RESTRICTED_CAST_FROM_ASCII
exists(../../tooling/tooling.pri): include(../../tooling/tooling.pri)

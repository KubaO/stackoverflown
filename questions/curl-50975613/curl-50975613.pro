QT = widgets
DEFINES += \
  QT_DEPRECATED_WARNINGS \
  QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
  QT_RESTRICTED_CAST_FROM_ASCII
CONFIG += c++14
TEMPLATE = app
SOURCES = main.cpp
macx {
  INCLUDEPATH += /opt/local/include
  LIBS += /opt/local/lib/libcurl.dylib
} else {
  LIBS += -lcurl
}

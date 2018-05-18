CONFIG += c++14
QT = widgets
DEFINES += \
  QT_DEPRECATED_WARNINGS \
  QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
  QT_RESTRICTED_CAST_FROM_ASCII
SOURCES += main.cpp
macx {
  ports = /opt/local
  LIBS += $$ports/lib/libopencv_core.dylib $$ports/lib/libopencv_imgproc.dylib
  INCLUDEPATH += $$ports/include
} else {
  LIBS += -lopencv_core -lopencv_imgproc
}

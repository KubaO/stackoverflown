QT = core
greaterThan(QT_MAJOR_VERSION, 4) {
  QT = widgets
  CONFIG += c++14
} else {
  QT += gui
  QMAKE_CXXFLAGS += -std=c++14
  macx: DEFINES += MACPORTS_QT4_BINARY_COMPAT_FIX
}
DEFINES += \
  QT_DEPRECATED_WARNINGS \
  QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
  QT_RESTRICTED_CAST_FROM_ASCII
TEMPLATE = app
SOURCES = main.cpp
LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui
macx {
  INCLUDEPATH += /opt/local/include
  LIBS += -L /opt/local/lib
}

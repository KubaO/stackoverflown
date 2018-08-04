TOOLING_HAS_WIDGETS = 0
TOOLING_PWD=$$PWD

greaterThan(QT_MAJOR_VERSION, 4) {
  qtHaveModule(widgets): TOOLING_HAS_WIDGETS = 1
} else {
  contains(QT, gui): TOOLING_HAS_WIDGETS = 1
  QMAKE_CXXFLAGS -= -std=c++11
  QMAKE_CXXFLAGS *= -std=c++14
  DEFINES += QT_WIDGETS_LIB
}

!defined(NO_TOOLING) {
  greaterThan(QT_MAJOR_VERSION, 4): QT *= core
  else: QT *= core

  HEADERS += \
    $$TOOLING_PWD/backport.h \
    $$TOOLING_PWD/tooling.h

  SOURCES += \
    $$TOOLING_PWD/backport.cpp \
    $$TOOLING_PWD/screenshot.cpp \
    $$TOOLING_PWD/showinshell.cpp \
    $$TOOLING_PWD/standardpaths.cpp \
    $$TOOLING_PWD/tooling.cpp
}



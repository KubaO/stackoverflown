TOOLING_HAS_WIDGETS = 0
TOOLING_PWD=$$PWD

greaterThan(QT_MAJOR_VERSION, 4) {
  qtHaveModule(widgets): TOOLING_HAS_WIDGETS = 1
} else {
  contains(QT, gui): TOOLING_HAS_WIDGETS = 1
  QMAKE_CXXFLAGS -= -std=c++11
  QMAKE_CXXFLAGS *= -std=c++14
}

!defined(NO_TOOLING) {
  greaterThan(QT_MAJOR_VERSION, 4): QT *= core_private
  else: QT *= core

  HEADERS += \
    $$TOOLING_PWD/backport.h \
    $$TOOLING_PWD/tooling.h

  SOURCES += \
    $$TOOLING_PWD/backport.cpp \
    $$TOOLING_PWD/showinshell.cpp

  equals(TOOLING_HAS_WIDGETS, 1) {
    SOURCES += \
      $$TOOLING_PWD/screenshot.cpp
  }
}

HEADERS += \
    $$PWD/tooling.h

SOURCES += \
    $$PWD/standardpaths.cpp


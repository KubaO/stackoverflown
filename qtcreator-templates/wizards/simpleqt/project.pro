@if "%{QT}" == "true"
@if "%{CONSOLE}" == "false"
@if "%{QT4SUPPORT}" == "false"
QT = widgets %{MODULES}
CONFIG += c++11
@else
greaterThan(QT_MAJOR_VERSION, 4) {
    QT = widgets %{MODULES}
    CONFIG += c++11
} else {
    QT = gui %{MODULES}
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
        QMAKE_CXXFLAGS_WARN_ON += -Wno-inconsistent-missing-override
    }
}
@endif
@else
QT = core %{MODULES}
@if "%{QT4SUPPORT}" == "false"
CONFIG += console c++11
@else
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += c++11
} else {
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
        QMAKE_CXXFLAGS_WARN_ON += -Wno-inconsistent-missing-override
    }
}
@endif
CONFIG -= app_bundle
@endif
@else
@if "%{QT4SUPPORT}" == "false"
CONFIG += console c++11
@else
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += c++11
} else {
    unix:QMAKE_CXXFLAGS += -std=c++11
    macx {
        QMAKE_CXXFLAGS += -stdlib=libc++
        QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.7
        QMAKE_CXXFLAGS_WARN_ON += -Wno-inconsistent-missing-override
    }
}
@endif
CONFIG -= qt app_bundle
@endif
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x050800
TARGET = %{ProjectName}
TEMPLATE = app
SOURCES += main.cpp

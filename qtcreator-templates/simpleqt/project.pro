@if "%QT%" == "true"
@if "%CONSOLE%" == "false"
@if "%QT4SUPPORT%" == "false"
QT = widgets %MODULES%
CONFIG += c++11
@else
greaterThan(QT_MAJOR_VERSION, 4) {
    QT = widgets %MODULES%
    CONFIG += c++11
} else {
    QT = gui %MODULES%
    unix:QMAKE_CXXFLAGS += -std=c++11
}
@endif
@else
QT = core %MODULES%
@if "%QT4SUPPORT%" == "false"
CONFIG += console c++11
@else
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += c++11
} else {
    unix:QMAKE_CXXFLAGS += -std=c++11
}
@endif
CONFIG -= app_bundle
@endif
@else
@if "%QT4SUPPORT%" == "false"
CONFIG += console c++11
@else
CONFIG += console
greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += c++11
} else {
    unix:QMAKE_CXXFLAGS += -std=c++11
}
@endif
CONFIG -= qt app_bundle
@endif
TARGET = %ProjectName%
TEMPLATE = app
SOURCES += main.cpp

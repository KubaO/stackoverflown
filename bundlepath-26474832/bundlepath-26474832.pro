QT       += core
QT       -= gui
TARGET = bundlepath-26474832
CONFIG   += console
TEMPLATE = app
SOURCES += main.cpp
mac {
    BUNDLE = $$OUT_PWD/$$TARGET$$quote(.app)/Contents
    QMAKE_POST_LINK += ditto \"$$PWD/data.txt\" \"$$BUNDLE/Resources/\";
}

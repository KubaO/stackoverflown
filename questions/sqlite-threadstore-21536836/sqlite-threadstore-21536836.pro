QT = core
TARGET = sqlite-threadstore-21536836
CONFIG   += console
CONFIG   -= app_bundle
TEMPLATE = app
SOURCES += main.cpp
macx:LIBS += -L/opt/local/lib -lsqlite3

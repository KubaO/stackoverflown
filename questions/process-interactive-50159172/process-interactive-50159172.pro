QT = core
CONFIG += c++14 console
CONFIG -= app_bundle
TARGET = process-interactive-50159172
TEMPLATE = app
SOURCES += main.cpp
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000 QT_RESTRICTED_CAST_FROM_ASCII
OTHER_FILES += test.py

copydata.commands = $(COPY_FILE) \"$$shell_path($$PWD\\test.py)\" \"$$shell_path($$OUT_PWD)\"
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

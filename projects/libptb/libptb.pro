include(../base.pro)

TEMPLATE = lib
CONFIG += staticlib

VERSION = 0.1.0

TARGET = ptb
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
}

RESOURCES += ../../src/ptb.qrc

target.path = /usr/local/lib/
headers.path = /usr/local/include/
headers.files = ../../src/include/*

INSTALLS += headers target

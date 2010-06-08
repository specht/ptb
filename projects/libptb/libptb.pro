include(../base.pro)

TEMPLATE = lib
CONFIG += staticlib

VERSION = 0.1.0

TARGET = ptb
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
}

isEmpty(PREFIX) {
    PREFIX = /usr/local/
}

message(libptb will be installed in $$PREFIX)

RESOURCES += ../../src/ptb.qrc

target.path = $$PREFIX/lib/
headers.path = $$PREFIX/include/
headers.files = ../../src/include/*

INSTALLS += headers target

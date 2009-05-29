include(../base.pro)

TEMPLATE = lib
CONFIG += staticlib

TARGET = ptb
CONFIG(debug, debug|release) {
	TARGET = $$join(TARGET,,,_debug)
}

RESOURCES += ../../src/ptb.qrc

include(../base.pro)

TARGET = groupproteins
CONFIG(debug, debug|release) {
	TARGET = $$join(TARGET,,,_debug)
}

RESOURCES += ../../src/ptb.qrc

SOURCES += \
	../../src/groupproteins.cpp \

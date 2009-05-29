include(../base.pro)

TARGET = stripscans
CONFIG(debug, debug|release) {
	TARGET = $$join(TARGET,,,_debug)
}

RESOURCES += ../../src/ptb.qrc

HEADERS += \

SOURCES += \
	../../src/stripscans.cpp \

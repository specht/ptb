include(../base.pro)

TARGET = decoyfasta
CONFIG(debug, debug|release) {
	TARGET = $$join(TARGET,,,_debug)
}

SOURCES += \
	../../src/decoyfasta.cpp \

include(../base.pro)

TARGET = cleanfasta
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
}

SOURCES += \
    ../../src/cleanfasta.cpp \

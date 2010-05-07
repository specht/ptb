include(../base.pro)

TARGET = translatedna
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
}

RESOURCES += ../../src/ptb.qrc

SOURCES += \
    ../../src/translatedna.cpp \

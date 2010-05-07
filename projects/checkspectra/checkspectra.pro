include(../base.pro)

TARGET = checkspectra
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
}

HEADERS += \
    ../../src/SpectrumChecker.h \

SOURCES += \
    ../../src/SpectrumChecker.cpp \
    ../../src/checkspectra.cpp \

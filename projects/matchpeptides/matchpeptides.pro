include(../base.pro)

TARGET = matchpeptides
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
}

RESOURCES += ../../src/ptb.qrc

SOURCES += \
    ../../src/matchpeptides.cpp \

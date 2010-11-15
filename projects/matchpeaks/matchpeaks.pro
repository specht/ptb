include(../base.pro)

TARGET = matchpeaks
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
}

HEADERS += \

SOURCES += \
    ../../src/matchpeaks.cpp \

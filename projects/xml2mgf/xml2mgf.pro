include(../base.pro)

TARGET = xml2mgf
CONFIG(debug, debug|release) {
	TARGET = $$join(TARGET,,,_debug)
}

HEADERS += \

SOURCES += \
	../../src/xml2mgf.cpp \

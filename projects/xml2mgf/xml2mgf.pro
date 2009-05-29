include(../base.pro)

TARGET = xml2mgf
CONFIG(debug, debug|release) {
	TARGET = $$join(TARGET,,,_debug)
}

HEADERS += \
	../../src/MgfWriter.h \

SOURCES += \
	../../src/MgfWriter.cpp \
	../../src/xml2mgf.cpp \

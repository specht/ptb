TEMPLATE = app

CONFIG += debug_and_release console

DEPENDPATH += .
INCLUDEPATH += . ../../src/include/

macx {
	LIBPATH += /Users/michael/programming/ext/lib
	INCLUDEPATH += /Users/michael/programming/ext/include
}

macx {
	CONFIG -= app_bundle
	CONFIG += x86 ppc
	QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.4
}

linux {
    CONFIG += static
}

LIBS += -lz -lbz2 -lquazip -lyaml-cpp

CONFIG(debug, debug|release) {
	OBJECTS_DIR = ../../obj/debug/
	MOC_DIR = ../../obj/debug/
	RCC_DIR = ../../obj/debug/
}
else {
	OBJECTS_DIR = ../../obj/release/
	MOC_DIR = ../../obj/release/
	RCC_DIR = ../../obj/release/
}

DESTDIR = ../../

QT = core xml

HEADERS += \
	../../src/include/ptb/FastaReader.h \
	../../src/include/ptb/FastaWriter.h \
	../../src/include/ptb/IsotopeEnvelope.h \
	../../src/include/ptb/MgfWriter.h \
	../../src/include/ptb/MzDataHandler.h \
	../../src/include/ptb/MzMlHandler.h \
	../../src/include/ptb/MzXmlHandler.h \
	../../src/include/ptb/RefPtr.h \
	../../src/include/ptb/ScanIterator.h \
	../../src/include/ptb/XmlHandler.h \
    ../../src/Yaml.h \
    ../../src/YamlEmitter.h \
    ../../src/YamlParser.h \
	../../src/include/ptb/ZipFileOrNot.h \

SOURCES += \
	../../src/FastaReader.cpp \
	../../src/FastaWriter.cpp \
	../../src/IsotopeEnvelope.cpp \
	../../src/MgfWriter.cpp \
	../../src/MzDataHandler.cpp \
	../../src/MzMlHandler.cpp \
	../../src/MzXmlHandler.cpp \
	../../src/ScanIterator.cpp \
	../../src/XmlHandler.cpp \
    ../../src/Yaml.cpp \
    ../../src/YamlEmitter.cpp \
    ../../src/YamlParser.cpp \
    ../../src/ZipFileOrNot.cpp \

# KFilterBase

# Input

HEADERS += \
	../../src/kfilterbase/kbzip2filter.h \
	../../src/kfilterbase/kde_config.h \
	../../src/kfilterbase/kfilterbase.h \
	../../src/kfilterbase/kfilterdev.h \
	../../src/kfilterbase/kgzipfilter.h \

SOURCES += \
	../../src/kfilterbase/kbzip2filter.cpp \
	../../src/kfilterbase/kfilterbase.cpp \
	../../src/kfilterbase/kfilterdev.cpp \
	../../src/kfilterbase/kgzipfilter.cpp	 \

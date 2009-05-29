TEMPLATE = app

CONFIG += debug_and_release console

DEPENDPATH += .
INCLUDEPATH += .

macx {
	CONFIG -= app_bundle
	CONFIG += x86
}

LIBS += -lz -lbz2 -lquazip

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

QT = core gui xml svg

HEADERS += \
	../../src/FastaReader.h \
	../../src/FastaWriter.h \
	../../src/MzDataHandler.h \
	../../src/MzMlHandler.h \
	../../src/MzXmlHandler.h \
	../../src/RefPtr.h \
	../../src/ScanIterator.h \
	../../src/XmlHandler.h \
	../../src/ZipFileOrNot.h \

SOURCES += \
	../../src/FastaReader.cpp \
	../../src/FastaWriter.cpp \
	../../src/MzDataHandler.cpp \
	../../src/MzMlHandler.cpp \
	../../src/MzXmlHandler.cpp \
	../../src/ScanIterator.cpp \
	../../src/XmlHandler.cpp \
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

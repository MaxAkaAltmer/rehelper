#-------------------------------------------------
#
# Project created by QtCreator 2014-05-23T15:12:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ReHelper
TEMPLATE = app

INCLUDEPATH += glob

SOURCES += main.cpp\
        mainwindow.cpp \
    apreprocessor.cpp \
    glob/adata.cpp \
    glob/astring.cpp \
    glob/avariant.cpp \
    glob/math_vec.cpp \
    glob/afile.cpp \
    glob/astring_path_utils.cpp \
    glob/rxml/tinystr.cpp \
    glob/rxml/tinyxml.cpp \
    glob/rxml/tinyxmlerror.cpp \
    glob/rxml/tinyxmlparser.cpp \
    glob/axobject.cpp \
    netlist_reader.cpp \
    alex_base.cpp \
    netlist_reader_def.cpp \
    netlist_reader_prim.cpp \
    glob/atime.cpp

HEADERS  += mainwindow.h \
    apreprocessor.h \
    glob/adata.h \
    glob/afile.h \
    glob/astring.h \
    glob/at_array.h \
    glob/at_hash.h \
    glob/at_ring.h \
    glob/avariant.h \
    glob/crc32.h \
    glob/math_int.h \
    glob/math_vec.h \
    glob/types.h \
    glob/astring_path_utils.h \
    glob/rxml/tinystr.h \
    glob/rxml/tinyxml.h \
    glob/axobject.h \
    netlist_reader.h \
    alex_base.h \
    netlist_reader_def.h \
    glob/astring_latin.h \
    glob/astring_utf.h \
    glob/at_graph.h \
    glob/at_string_utils.h \
    glob/atime.h

win32{
    LIBS += -lws2_32
}

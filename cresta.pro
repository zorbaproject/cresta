#-------------------------------------------------
#
# Project created by QtCreator 2018-03-14T18:58:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cresta
TEMPLATE = app

include(3rdparty/qtxslx/src/xlsx/qtxlsx.pri)

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

LANGUAGES = en it de fr es pt

defineReplace(prependAll) {
 for(a,$$1):result += $$2$${a}$$3
 return($$result)
}

TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/languages/Translation_, .ts)

unix {
qtPrepareTool(LRELEASE, lrelease)
 for(tsfile, TRANSLATIONS) {
     command = $$LRELEASE $$tsfile
     system($$command)|error("Failed to run: $$command")
 }
}

RESOURCES += \
    languages.qrc

SUBDIRS += \
    3rdparty/qtxslx/src/xlsx/doc/snippets/doc_src_qtxlsx.pro \
    3rdparty/qtxslx/src/xlsx/xlsx.pro \
    3rdparty/qtxslx/src/src.pro

DISTFILES += \
    3rdparty/qtxslx/src/xlsx/qtxlsx.pri \
    3rdparty/qtxslx/src/xlsx/doc/qtxlsx.qdocconf \
    3rdparty/qtxslx/src/xlsx/doc/src/examples.qdoc \
    3rdparty/qtxslx/src/xlsx/doc/src/qtxlsx-index.qdoc \
    3rdparty/qtxslx/src/xlsx/doc/src/qtxlsx.qdoc \
    3rdparty/qtxslx/src/xlsx/doc/src/usage.qdoc

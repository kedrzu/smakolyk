#-------------------------------------------------
#
# Project created by QtCreator 2012-10-20T18:38:38
#
#-------------------------------------------------

QT       += core network xml

QT       -= gui

TARGET = KC-Presta
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    PSWebService.cpp \
    Presta.cpp \
    Logger.cpp \
    KCFirma.cpp \
    Produkt.cpp \
    Kategoria.cpp \
    Config.cpp \
    Zamowienie.cpp \
    SpecificPrice.cpp

HEADERS += \
    PSWebService.h \
    Presta.h \
    Kategoria.h \
    Produkt.h \
    Logger.h \
    KCFirma.h \
    Config.h \
    Zamowienie.h \
    SpecificPrice.h

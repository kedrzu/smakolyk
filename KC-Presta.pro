#-------------------------------------------------
#
# Project created by QtCreator 2012-10-20T18:38:38
#
#-------------------------------------------------

QT       += core network xml gui sql

TARGET = KC-Presta
CONFIG   += console
#CONFIG   -= app_bundle

win32:RC_FILE = icon.rc

TEMPLATE = app

SOURCES += main.cpp \
    PSWebService.cpp \
    Presta.cpp \
    Logger.cpp \
    KCFirma.cpp \
    Produkt.cpp \
    KCPresta.cpp \
    PrestaStructs.cpp \
    MainWindow.cpp \
    ConsoleDialog.cpp \
    Exception.cpp

HEADERS += \
    PSWebService.h \
    Presta.h \
    Kategoria.h \
    Produkt.h \
    Logger.h \
    KCFirma.h \
    Zamowienie.h \
    KCPresta.h \
    PrestaStructs.h \
    MainWindow.h \
    ConsoleDialog.h \
    Exception.h

FORMS += \
    MainWindow.ui \
    ConsoleDialog.ui

RESOURCES += \
    icons.qrc

OTHER_FILES += \
    icon.rc

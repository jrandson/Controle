#-------------------------------------------------
#
# Project created by QtCreator 2015-02-27T14:38:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Controle2
TEMPLATE = app


SOURCES += main.cpp\
        painel.cpp \
    qcustomplot.cpp \
    controller.cpp \
    dlgpid.cpp \
    controladorpid.cpp \
    tanque.cpp

HEADERS  += painel.h \
    qcustomplot.h \
    controller.h \
    signal.h \
    quanser.h \
    dlgpid.h \
    controladorpid.h \
    tanque.h \
    estados.h

FORMS    += painel.ui \
    dlgpid.ui

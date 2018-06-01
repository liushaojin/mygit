#-------------------------------------------------
#
# Project created by QtCreator 2018-05-30T18:37:06
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UTest
TEMPLATE = app


SOURCES += main.cpp\
        utestdlg.cpp \
    udpthread.cpp \
    udpReceived.cpp

HEADERS  += utestdlg.h \
    udpthread.h \
    udpReceived.h

FORMS    += utestdlg.ui

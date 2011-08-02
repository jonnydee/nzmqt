#-------------------------------------------------
#
# Project created by QtCreator 2010-10-15T17:00:35
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = TestApp
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    main.cpp

HEADERS += \
    ../include/nzmqt/nzmqt.hpp \
    TestApp.h \
    pubsub/PubSubServer.h \
    pubsub/PubSubClient.h \
    reqrep/ReqRepServer.h \
    reqrep/ReqRepClient.h

LIBS += -lzmq

INCLUDEPATH += \
    ../include \
    /opt/local/include

QMAKE_LIBDIR += \
    /opt/local/lib


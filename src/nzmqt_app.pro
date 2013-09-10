# Copyright 2011-2013 Johann Duscher (a.k.a. Jonny Dee). All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
#
#    1. Redistributions of source code must retain the above copyright notice, this list of
#       conditions and the following disclaimer.
#
#    2. Redistributions in binary form must reproduce the above copyright notice, this list
#       of conditions and the following disclaimer in the documentation and/or other materials
#       provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY JOHANN DUSCHER ''AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# The views and conclusions contained in the software and documentation are those of the
# authors and should not be interpreted as representing official policies, either expressed
# or implied, of Johann Duscher.


QT       += core

QT       -= gui

TARGET = nzmqt_app
VERSION = 2.2.1
DESTDIR = $$_PRO_FILE_PWD_/../bin
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += \
# Uncomment this line for nzmqt "Include and Source File" compilation option.
#    NZMQT_LIB

SOURCES += \
# Uncomment this line for nzmqt "Include and Source File" compilation option.
#    nzmqt/nzmqt.cpp \
    app/main.cpp

HEADERS += \
    ../include/nzmqt/nzmqt.hpp \
    common/SampleBase.hpp \
    pubsub/Subscriber.hpp \
    pubsub/Publisher.hpp \
    pushpull/Sink.hpp \
    pushpull/Worker.hpp \
    pushpull/Ventilator.hpp \
    reqrep/Requester.hpp \
    reqrep/Replier.hpp \
    app/NzmqtApp.hpp

LIBS += -lzmq

INCLUDEPATH += \
    ../include \
    ../externals/include \
    $(QTDIR)/include \
    /opt/local/include

QMAKE_LIBDIR += \
    /opt/local/lib

OTHER_FILES += \
    ../README.md \
    ../LICENSE.header \
    ../CHANGELOG.md \
    ../LICENSE.md

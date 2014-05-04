# Copyright 2011-2014 Johann Duscher (a.k.a. Jonny Dee). All rights reserved.
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


QT       -= gui

TARGET = nzmqt
VERSION = 3.2.1
DESTDIR = $$_PRO_FILE_PWD_/../bin
TEMPLATE = lib

CONFIG   += debug_and_release
CONFIG(debug, debug|release) {
     TARGET = $$join(TARGET,,,d)
}

DEFINES += \
    NZMQT_LIB \
    NZMQT_SHAREDLIB

SOURCES += \
    nzmqt/nzmqt.cpp

HEADERS += \
    ../include/nzmqt/global.hpp \
    ../include/nzmqt/nzmqt.hpp \
    ../include/nzmqt/impl.hpp

LIBS += -lzmq

INCLUDEPATH += \
    ../include \
    ../3rdparty/cppzmq \
    $(QTDIR)/include \
    /opt/local/include

QMAKE_LIBDIR += \
    /opt/local/lib

OTHER_FILES += \
    ../README.md \
    ../LICENSE.header \
    ../CHANGELOG.md \
    ../LICENSE.md



#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../Libs/ZEROMQ/lib/ -llibzmq
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../Libs/ZEROMQ/lib/ -llibzmq_d

#INCLUDEPATH += $$PWD/../../../../../Libs/ZEROMQ/include
#DEPENDPATH += $$PWD/../../../../../Libs/ZEROMQ/include


#INCLUDEPATH += $$PWD/../include
#DEPENDPATH += $$PWD/../include

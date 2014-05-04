# Include this file into your project to build and link
# the nzmqt library into you application

# This define will "move" nzmqt class method
# implementations to nzmqt.cpp file.
DEFINES += NZMQT_LIB

SOURCES += \
    $$PWD/src/nzmqt/nzmqt.cpp

HEADERS += \
    $$PWD/include/nzmqt/global.hpp \
    $$PWD/include/nzmqt/nzmqt.hpp \
    $$PWD/include/nzmqt/impl.hpp

INCLUDEPATH += \
    $$PWD/include \
    $$PWD/3rdparty/cppzmq
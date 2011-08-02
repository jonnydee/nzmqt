nzmqt - A lightweight C++ [Qt][] binding for [ZeroMQ][]
======================================================

nzmqt is a re-implementation of the approach taken by the zeromqt library. The idea 
is to integrate ZeroMQ into the Qt event loop, mapping ZeroMQ message events onto 
Qt signals. The original implementation also provides a Qt-like API
which allows to represent messages as QByteArray instances. While I took this idea
and implementation as a source of information, nzmqt is a completely new implementation.
I did this not only in order to get rid of short comings, but also because I wanted
to release my changes using the simplified BSD license and until now the author of
zeromqt hasn't officially released his work under a certain license.

While zeromqt works fine for non-multi-part messages, his implementation doesn't
support multi-part messages yet. Also, a lot of code duplicates code within the
ZeroMQ's standard C++ binding. So in contrast to his implementation, nzmqt reuses
as much code of ZeroMQ's original C++ binding as possible. Additionally, several
things have been changed from a user's perspective. In summary, this nzmqt contains
the following changes:

* The implementation is a complete re-write in the sense that it doesn't duplicate code
of ZeroMQ's official C++ binding anymore. Instead, it builds upon existing code
through inheritance and, hence, it will likely benefit from future bugfixes and
enhancements for the official C++ binding.
* All classes are placed into a separate namespace 'nzmqt'.
* This version now also supports ZeroMQ's multi-part messages.
* The initial support for using Qt's way of handling errors using error codes 
has been dropped. Instead, this code only throws exception originally thrown
by ZeroMQ's official C++ API. Note that although it looks like 'ZMQException'
is a new custom exception class there is no custom exception class, but only
a simple typedef which places the original exception class into the new
namespace giving it a new name.
* As with ZeroMQ's C++ binding all classes are contained within a singe header
file which makes integrating this Qt binding very easy.
* There is no 'ZmqContext' singleton anymore. Instead you can create your
own 'ZMQContext' instance yourself.
* The socket class 'ZMQSocket' now also inherits from QObject, so you can
add it as a child to any QObject parent as you know it from Qt.
* The code is officially licensed under the simplified BSD license.

Status
------

There are no known bugs.

Usage
-----

As ZeroMQ's C++ binding this Qt binding only consists of a single C++ header file
which you need to include in your project.

Consequently, using 'nzmqt' in a Qt project is as simple as adding that single header
file to your project's .pro file as follows (assumed you use QT Creator).

    HEADERS += nzmqt/nzmqt.hpp

If not already done, you also need to link against ZeroMQ library:

    LIBS += -lzmq

Of course, you need to make sure the header file as well as the ZeroMQ library
can be found by your compiler/linker.

As nzmqt uses C++ exceptions for error handling so you will need to catch them
by overriding QCoreApplication::notify() method. The included sample will
show you how this can be done. 

Included Samples
----------------

Currently, there are samples showing PUB-SUB and REQ-REP protocol with multi-part messages in action.
They also show how to deal with exceptions in Qt.

More Information
----------------

[nzmqt]:   https://github.com/jonnydee/nzmqt "https://github.com/jonnydee/nzmqt"

[Qt]:     http://qt.nokia.com/ "Qt"
[ZeroMQ]: http://zeromq.com/   "ZeroMQ"

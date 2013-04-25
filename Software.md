Overview
========

Versioning and Compatibility
----------------------------

It has been decided to reflect compatibility with 0MQ releases using a corresponding versioning scheme. The last release not using the new scheme is version-0.7, which is compatible with 0MQ release 2.2. From now on all nzmqt releases compatible with 0MQ release 2.x will have version numbers starting with 2.x, too. Correspondingly, all nzmqt releases compatible with 0MQ 3.x will have version numbers starting with 3.x.

Dependencies
------------

* [Qt 4.8.x][]
* [0MQ 3.2.x][zeromq 3.2.x]
* [C++ binding for 0MQ][cppzmq]

Usage
-----

As 0MQ's C++ binding this Qt binding only consists of a single C++ header file which you need to include in your project.

Consequently, using 'nzmqt' in a Qt project is as simple as adding that single header file to your project's .pro file as follows (assumed you use QT Creator).

    HEADERS += nzmqt/nzmqt.hpp

If not already done, you also need to link against 0MQ library:

    LIBS += -lzmq

Of course, you need to make sure the header file as well as the 0MQ library can be found by your compiler/linker.

As nzmqt uses C++ exceptions for error handling so you will need to catch them by overriding QCoreApplication::notify() method. The included samples will show you how this can be done. 


 [cppzmq]:              https://github.com/zeromq/cppzmq                            "C++ binding for 0MQ on GitHub"
 [Qt 4.8.x]:            http://download.qt-project.org/official_releases/qt/4.8/    "Qt 4.8.x download page"
 [zeromq 3.2.x]:        http://www.zeromq.org/intro:get-the-software                "0MQ download page" 

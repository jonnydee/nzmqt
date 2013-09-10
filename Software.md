Overview
========

Getting Started
---------------

### Download nzmqt

First of all, decide which version of nzmqt you will need. Your choice will depend on the version of 0MQ you are going to use. In order to make this choice easier a new versioning scheme for nzmqt has been introduced. nzmqt version numbers indicate compatibility with 0MQ version <0MQ-major>.<0MQ-minor>.<0MQ-bugfix> by adopting the major and minor part. For instance, nzmqt releases compatible with all 0MQ versions 2.2.x will have a corresponding version number 2.2.y, and an nzmqt version number 3.2.y indicates compatibility with all 0MQ versions 3.2.x. Note that there is no relation between nzmqt's and 0MQ's bugfix versions as bugfixes do not introduce incompatible API changes. Also note that release 0.7 is the last release NOT conforming to this scheme.

Once you know which version you need you can either clone the complete nzmqt code repository or you can directly download the sources of a release as a ZIP file from GitHub.

### Resolve dependencies

As nzmqt is a Qt binding for 0MQ there is obviously a dependency to these two libraries. Since 0MQ version 3 the C++ binding for 0MQ is not part of the core anymore. It has been externalized to a [separate Git repository][cppzmq].

So here is the complete list of dependencies:

* [Qt 4.8.x][] (or newer): You will need to download and install it yourself.
* [0MQ 3.2.x][zeromq 3.2.x]: You will need to download and install it yourself.
* [C++ binding for 0MQ][cppzmq]: A script delivered with nzmqt will download the source code and appropriate version itself.

### Setup nzmqt project

As already mentioned in the previous section nzmqt comes with a short script that will make sure a compatible version of 0MQ's C++ binding is available by downloading the correct version. In fact, the corresponding GitHub project repository is referenced as a Git submodule. So the script actually initializes the submodule. Furthermore, it will copy the C++ binding's include file to ``<path-to-nzmqt>/externals/include`` directory in the project root.

***Execute setup script***

In a console type:

    cd <path-to-nzmqt>
    ./setup-project.sh

***Test your installation***

In order to see if everything works well you can compile and run unit tests provided with nzmqt. Just compile ``nzmqt_test.pro`` project located under ``<path-to-nzmqt>/src`` directory. If the build is successful you will find an ``nzmqt_test`` executable under ``<path-to-nzmqt>/bin`` directory. Run this binary in a console and look if all tests passed. If some tests did not pass please [file a bug][nzmqt issue tracker] using the nzmqt issue tracker on GitHub.

### Setup your own project to use nzmqt

There are different options for integrating nzmqt in your project. The following descriptions assume you use Qt's [QMake][] tool.

***Include File Only***

Like 0MQ's C++ binding nzmqt can be added to your project by including a single C++ header file which you need to include in your project. Consequently, using nzmqt in a Qt project is as simple as adding that single header file to your project's ``.pro`` file as follows.

    HEADERS += nzmqt/nzmqt.hpp

Adjust your include path:

    INCLUDEPATH += \
        <path-to-nzmqt>/include \
        <path-to-nzmqt>/externals/include

And if not already done, you also need to link against 0MQ library:

    LIBS += -lzmq

***Include and Source File***

If you don't like the everything is inlined as it is the case with *Include File Only* option you can use this approach. It will "move" the class method implementations to a separate ``nzmqt.cpp`` file. For this to work modify your ``.pro`` file as follows.

    # This define will "move" nzmqt class method
    # implementations to nzmqt.cpp file.
    DEFINES += NZMQT_LIB
    
    HEADERS += nzmqt/nzmqt.hpp
    SOURCES += nzmqt/nzmqt.cpp
    
    INCLUDEPATH += \
        <path-to-nzmqt>/include \
        <path-to-nzmqt>/externals/include
    
    LIBS += -lzmq
    
***Static Library***

You can also create an nzmqt static library. There is a project file ``nzmqt_staticlib.pro`` included in ``<path-to-nzmqt>/src`` directory which is preconfigured to do this.

***Shared Library***

Finally, there is the option to create an nzmqt shared library. A corresponding preconfigured project file ``nzmqt_sharedlib.pro`` can also be found in ``<path-to-nzmqt>/src``directory.

Documentation
-------------

* [API reference][]
* [changelog][]
* [software license][]
* [samples][]


 [cppzmq]:              https://github.com/zeromq/cppzmq                                        "C++ binding for 0MQ on GitHub"
 [Qt 4.8.x]:            http://download.qt-project.org/official_releases/qt/4.8/                "Qt 4.8.x download page"
 [zeromq 3.2.x]:        http://www.zeromq.org/intro:get-the-software                            "0MQ download page"
 [QMake]:               http://doc-snapshot.qt-project.org/qt5-stable/qmake/qmake-manual.html   "Latest QMake manual"
 [nzmqt issue tracker]: https://github.com/jonnydee/nzmqt/issues                                "nzmqt issue tracker on GitHub"

 [API reference]:       Software-API-Reference                                                  "nzmqt API reference"
 [changelog]:           Software-Changelog                                                      "nzmqt software changelog"
 [software license]:    Software-License                                                        "nzmqt software license"
 [samples]:             Samples                                                                 "nzmqt samples overview"

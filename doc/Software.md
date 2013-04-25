Overview
========

Getting Started
---------------

### Download nzmqt

First of all, you need to decide which version of nzmqt you will need. Your choice will depend on the version of 0MQ you are going to use. In order to make this choice easier a new versioning scheme for nzmqt has been introduced. nzmqt version numbers indicate compatibility with 0MQ version <0MQ-major>.<0MQ-minor>.<0MQ-bugfix> by adopting the major and minor part. For instance, nzmqt releases compatible with all 0MQ versions 2.2.x will have a correponding version number 2.2.y, and an nzmqt version number 3.2.y indicates compatibility with all 0MQ versions 3.2.x. Note that there is no relation between nzmqt's and 0MQ's bugfix versions as bugfixes do not introduce incompatible API changes. Also not that release 0.7 is the last release NOT coforming to this scheme.

Once you know which version you need you can either clone the complete nzmqt code repository or you can directly download the sources of a release as a ZIP file from GitHub.

### Resolve dependencies

As nzmqt is a Qt binding for 0MQ there are obviously at least two dependencies to these two libraries. Since 0MQ version 3 the C++ binding for 0MQ is not part of the core anymore. It has been externalized to a [separate Git repository][cppzmq].

So here is the complete list of dependencies:
* [Qt 4.8.x][]: You will need to download and install it yourself.
* [0MQ 3.2.x][zeromq 3.2.x]: You will need to download and install it yourself.
* [C++ binding for 0MQ][cppzmq]: A script delivered with nzmqt will downlaod the source code and appropriate version itself.

### Setup nzmqt project

As already mentioned in the previous section nzmqt comes with a short script that will make sure the correct version of 0MQ's C++ binding is available by downloading the correct version. In fact, the corresponding GitHub project repository is referenced as a Git submodule. So the script actually initializes the submodule. Furthermore, it will copy the C++ binding's include file to directory 'externals/include' in the project root.

Execute the script like this:
    cd <path-to-nzmqt>
    ./setup-project.sh

### Setup your own project to use nzmqt

Like 0MQ's C++ binding nzmqt only consists of a single C++ header file which you need to include in your project. Consequently, using 'nzmqt' in a Qt project is as simple as adding that single header file to your project's .pro file as follows (assumed you use QT's QMake tool).

    HEADERS += nzmqt/nzmqt.hpp

Adjust your include path:

    INCLUDEPATH += \
        <path-to-nzmqt>/include \
        <path-to-nzmqt>/externals/include

And if not already done, you also need to link against 0MQ library:

    LIBS += -lzmq

Documentation
-------------

* [API reference][]
* [changelog][]
* [software license][]
* [samples][]


 [cppzmq]:              https://github.com/zeromq/cppzmq                            "C++ binding for 0MQ on GitHub"
 [Qt 4.8.x]:            http://download.qt-project.org/official_releases/qt/4.8/    "Qt 4.8.x download page"
 [zeromq 3.2.x]:        http://www.zeromq.org/intro:get-the-software                "0MQ download page"

 [API reference]:       Software-API-Reference.md                                   "nzmqt API reference"
 [changelog]:           ../CHANGELOG.md                                             "nzmqt software changelog"
 [software license]:    ../LICENSE.md                                               "nzmqt software license"
 [samples]:             Samples.md                                                  "nzmqt samples overview"

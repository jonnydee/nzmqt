Overview
========

There are some samples showing PUB-SUB, REQ-REP and PUSH-PULL protocol with multi-part messages in action. As nzmqt uses C++ exceptions for error handling you will need to catch them by overriding QCoreApplication::notify() method. The included samples will show you how this can be done.

* [pubsub][]: Demonstrates how to implement PUB-SUB protocol.
* [pushpull][]: Demonstrates how to implement PUSH-PULL protocol.
* [reqrep][]: Demonstrates how to implement REQ-REP protocol.


 [pubsub]:      Samples-pubsub      "PUB-SUB protocol example"
 [pushpull]:    Samples-pushpull    "PUSH-PULL protocol example"
 [reqrep]:      Samples-reqrep      "REQ-REP protocol example"

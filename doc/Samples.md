Overview
========

There are some samples showing *PUB-SUB*, *REQ-REP* and *PUSH-PULL* protocol with multi-part messages in action. As nzmqt uses C++ exceptions for error handling you will need to catch them by overriding ``QCoreApplication::notify()`` method. The included samples will show you how this can be done.

Running samples
---------------

All samples are compiled into one executable called ``nzmqt_app``. You can try them out by providing appropriate command line parameters. If you don't provide any, or if you provide ``-h`` or ``--help`` option, a list of all examples together with required parameters will be shown:

```
USAGE: ./nzmqt_app [-h|--help]                                                                 -- Show this help message.

USAGE: ./nzmqt_app pubsub-publisher <address> <topic>                                          -- Start PUB server.
       ./nzmqt_app pubsub-subscriber <address> <topic>                                         -- Start SUB client.

USAGE: ./nzmqt_app reqrep-replier <address> <reply-msg>                                        -- Start REQ server.
       ./nzmqt_app reqrep-requester <address> <request-msg>                                    -- Start REP client.

USAGE: ./nzmqt_app pushpull-ventilator <ventilator-address> <sink-address> <numberOfWorkItems> -- Start ventilator.
       ./nzmqt_app pushpull-worker <ventilator-address> <sink-address>                         -- Start a worker.
       ./nzmqt_app pushpull-sink <sink-address>                                                -- Start sink.

Publish-Subscribe Sample:
* Publisher:   ./nzmqt_app pubsub-publisher tcp://127.0.0.1:1234 ping
* Subscriber:  ./nzmqt_app pubsub-subscriber tcp://127.0.0.1:1234 ping

Request-Reply Sample:
* Replier:     ./nzmqt_app reqrep-replier tcp://127.0.0.1:1234 World
* Requester:   ./nzmqt_app reqrep-requester tcp://127.0.0.1:1234 Hello

Push-Pull Sample:
* Ventilator:  ./nzmqt_app pushpull-ventilator tcp://127.0.0.1:5557 tcp://127.0.0.1:5558 100
* Worker 1..n: ./nzmqt_app pushpull-worker tcp://127.0.0.1:5557 tcp://127.0.0.1:5558
* Sink:        ./nzmqt_app pushpull-sink tcp://127.0.0.1:5558
```

As can be seen, this output also provides examples showing how to run them with concrete command line parameters. You can directly copy and paste them to a shell prompt and run them without changing anything (assumed the given ports are free).

Documentation
-------------

Sample-specific information can be found here:
* [pubsub][]: Demonstrates how to implement PUB-SUB protocol.
* [pushpull][]: Demonstrates how to implement PUSH-PULL protocol.
* [reqrep][]: Demonstrates how to implement REQ-REP protocol.


 [pubsub]:      Samples-pubsub.md       "PUB-SUB protocol example"
 [pushpull]:    Samples-pushpull.md     "PUSH-PULL protocol example"
 [reqrep]:      Samples-reqrep.md       "REQ-REP protocol example"

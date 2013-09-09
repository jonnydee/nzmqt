Release 3.2.0-dev
=================
* Improved documentation.
* Emit poll errors as signals (instead of ignoring them).
* Fixed gh-6; Exception thrown during poll.
* Fixed gh-7: Fix occasional incorrect result from hasMoreMessageParts.
* Fixed gh-8: Unsafe disconnect in PollingZMQContext::unregisterSocket(QObject\* socket_).
* Fixed gh-12: warnings with -Wall (and compilation broken if -Werror).
* Feature gh-13: Support type-safe Qt 5 singals & slots connections.
* Feature gh-14: Additionally allow to link nzmqt 3.2.0 as static or shared lib.
* Task gh-16: Implement unit tests.

API Changes
-----------
* Method 'ZMQContext::createSocket()' does not implicitly set the 'ZMQContext' instance as parent of the created instance anymore. So if you don't pass a pointer to a parent QObject (NULL pointer) the socket instance won't have a parent set. Code doing this will **leak memory** now! Please make sure you delete the socket instance yourself in such cases.
* Method 'ZMQSocket::close()' now is a slot.


Release 2.2.0
=============
* fixed gh-5: Change PollingZMQContext::poll to empty the socket queue rather than poll once per interval.


Release 0.7
===========
* Introduced enumeration types for several ZMQ constants for type-safety.
* Added a new polling based implementation that works for all ZMQ communication protocols.
* Dropped support for REQ-REP protocol for old 'QSocketNotifier' based implementation.
* Added some more convenience methods to 'ZMQSocket' class.
* Old and new socket implementations now emit a signal with a received message as parameter.


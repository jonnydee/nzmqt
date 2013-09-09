// Copyright 2011-2013 Johann Duscher (a.k.a. Jonny Dee). All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice, this list of
//       conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright notice, this list
//       of conditions and the following disclaimer in the documentation and/or other materials
//       provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY JOHANN DUSCHER ''AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are those of the
// authors and should not be interpreted as representing official policies, either expressed
// or implied, of Johann Duscher.

#ifndef NZMQT_H
#define NZMQT_H

#include "nzmqt/global.hpp"

#include <zmq.hpp>

#include <QByteArray>
#include <QFlag>
#include <QList>
#include <QMetaType>
#include <QMutex>
#include <QObject>
#include <QRunnable>
#include <QVector>

// Define default context implementation to be used.
#ifndef NZMQT_DEFAULT_ZMQCONTEXT_IMPLEMENTATION
    #define NZMQT_DEFAULT_ZMQCONTEXT_IMPLEMENTATION PollingZMQContext
    //#define NZMQT_DEFAULT_ZMQCONTEXT_IMPLEMENTATION SocketNotifierZMQContext
#endif

// Define default number of IO threads to be used by ZMQ.
#ifndef NZMQT_DEFAULT_IOTHREADS
    #define NZMQT_DEFAULT_IOTHREADS 4
#endif

// Define default poll interval for polling-based implementation.
#ifndef NZMQT_POLLINGZMQCONTEXT_DEFAULT_POLLINTERVAL
    #define NZMQT_POLLINGZMQCONTEXT_DEFAULT_POLLINTERVAL 10 /* msec */
#endif

// Declare metatypes for using them in Qt signals.
Q_DECLARE_METATYPE(QList< QList<QByteArray> >)
Q_DECLARE_METATYPE(QList<QByteArray>)

class QSocketNotifier;

namespace nzmqt
{
    typedef zmq::free_fn free_fn;
    typedef zmq::pollitem_t pollitem_t;

    typedef zmq::error_t ZMQException;

    using zmq::poll;
    using zmq::version;

    // This class wraps ZMQ's message structure.
    class NZMQT_API ZMQMessage : private zmq::message_t
    {
        friend class ZMQSocket;

        typedef zmq::message_t super;

    public:
        ZMQMessage();

        ZMQMessage(size_t size_);

        ZMQMessage(void* data_, size_t size_, free_fn *ffn_, void* hint_ = 0);

        ZMQMessage(const QByteArray& b);

        using super::rebuild;

        void move(ZMQMessage* msg_);

        void copy(ZMQMessage* msg_);

        void clone(ZMQMessage* msg_);

        using super::data;

        using super::size;

        QByteArray toByteArray();
    };

    class ZMQContext;

    // This class cannot be instantiated. Its purpose is to serve as an
    // intermediate base class that provides Qt-based convenience methods
    // to subclasses.
    class NZMQT_API ZMQSocket : public QObject, private zmq::socket_t
    {
        Q_OBJECT
        Q_ENUMS(Type Event SendFlag ReceiveFlag Option)
        Q_FLAGS(Event Events)
        Q_FLAGS(SendFlag SendFlags)
        Q_FLAGS(ReceiveFlag ReceiveFlags)

        typedef QObject qsuper;
        typedef zmq::socket_t zmqsuper;

    public:
        enum Type
        {
            TYP_PUB = ZMQ_PUB,
            TYP_SUB = ZMQ_SUB,
            TYP_PUSH = ZMQ_PUSH,
            TYP_PULL = ZMQ_PULL,
            TYP_REQ = ZMQ_REQ,
            TYP_REP = ZMQ_REP,
            TYP_DEALER = ZMQ_DEALER,
            TYP_ROUTER = ZMQ_ROUTER,
            TYP_PAIR = ZMQ_PAIR,
            TYP_XPUB = ZMQ_XPUB,
            TYP_XSUB = ZMQ_XSUB
        };

        enum Event
        {
            EVT_POLLIN = ZMQ_POLLIN,
            EVT_POLLOUT = ZMQ_POLLOUT,
            EVT_POLLERR = ZMQ_POLLERR
        };
        Q_DECLARE_FLAGS(Events, Event)

        enum SendFlag
        {
            SND_MORE = ZMQ_SNDMORE,
            SND_NOBLOCK = ZMQ_NOBLOCK
        };
        Q_DECLARE_FLAGS(SendFlags, SendFlag)

        enum ReceiveFlag
        {
            RCV_NOBLOCK = ZMQ_NOBLOCK
        };
        Q_DECLARE_FLAGS(ReceiveFlags, ReceiveFlag)

        enum Option
        {
            // Get only.
            OPT_TYPE = ZMQ_TYPE,
            OPT_RCVMORE = ZMQ_RCVMORE,
            OPT_FD = ZMQ_FD,
            OPT_EVENTS = ZMQ_EVENTS,

            // Set only.
            OPT_SUBSCRIBE = ZMQ_SUBSCRIBE,
            OPT_UNSUBSCRIBE = ZMQ_UNSUBSCRIBE,

            // Get and set.
            OPT_HWM = ZMQ_HWM,
            OPT_SWAP = ZMQ_SWAP,
            OPT_AFFINITY = ZMQ_AFFINITY,
            OPT_IDENTITY = ZMQ_IDENTITY,
            OPT_RATE = ZMQ_RATE,
            OPT_RECOVERY_IVL = ZMQ_RECOVERY_IVL,
            OPT_RECOVERY_IVL_MSEC = ZMQ_RECOVERY_IVL_MSEC,
            OPT_MCAST_LOOP = ZMQ_MCAST_LOOP,
            OPT_SNDBUF = ZMQ_SNDBUF,
            OPT_RCVBUF = ZMQ_RCVBUF,
            OPT_LINGER = ZMQ_LINGER,
            OPT_RECONNECT_IVL = ZMQ_RECONNECT_IVL,
            OPT_RECONNECT_IVL_MAX = ZMQ_RECONNECT_IVL_MAX,
            OPT_BACKLOG = ZMQ_BACKLOG
        };

        ~ZMQSocket();

        using zmqsuper::operator void *;

        void setOption(Option optName_, const void *optionVal_, size_t optionValLen_);

        void setOption(Option optName_, const char* str_);

        void setOption(Option optName_, const QByteArray& bytes_);

        void setOption(Option optName_, qint32 value_);

        void setOption(Option optName_, quint32 value_);

        void setOption(Option optName_, qint64 value_);

        void setOption(Option optName_, quint64 value_);

        void getOption(Option option_, void *optval_, size_t *optvallen_) const;

        void bindTo(const QString& addr_);

        void bindTo(const char *addr_);

        void connectTo(const QString& addr_);

        void connectTo(const char* addr_);

        bool sendMessage(ZMQMessage& msg_, SendFlags flags_ = SND_NOBLOCK);

        bool sendMessage(const QByteArray& bytes_, SendFlags flags_ = SND_NOBLOCK);

        // Interprets the provided list of byte arrays as a multi-part message
        // and sends them accordingly.
        // If an empty list is provided this method doesn't do anything and returns trua.
        bool sendMessage(const QList<QByteArray>& msg_, SendFlags flags_ = SND_NOBLOCK);

        // Receives a message or a message part.
        bool receiveMessage(ZMQMessage* msg_, ReceiveFlags flags_ = RCV_NOBLOCK);

        // Receives a message.
        // The message is represented as a list of byte arrays representing
        // a message's parts. If the message is not a multi-part message the
        // list will only contain one array.
        QList<QByteArray> receiveMessage();

        // Receives all messages currently available.
        // Each message is represented as a list of byte arrays representing the messages
        // and their parts in case of multi-part messages. If a message isn't a multi-part
        // message the corresponding byte array list will only contain one element.
        // Note that this method won't work with REQ-REP protocol.
        QList< QList<QByteArray> > receiveMessages();

        qint32 fileDescriptor() const;

        Events events() const;

        // Returns true if there are more parts of a multi-part message
        // to be received.
        bool hasMoreMessageParts() const;

        void setIdentity(const char* nameStr_);

        void setIdentity(const QString& name_);

        void setIdentity(const QByteArray& name_);

        QByteArray identity() const;

        void setLinger(int msec_);

        qint32 linger() const;

        void subscribeTo(const char* filterStr_);

        void subscribeTo(const QString& filter_);

        void subscribeTo(const QByteArray& filter_);

        void unsubscribeFrom(const char* filterStr_);

        void unsubscribeFrom(const QString& filter_);

        void unsubscribeFrom(const QByteArray& filter_);

    signals:
        void messageReceived(const QList<QByteArray>&);

    public slots:
        void close();

    protected:
        ZMQSocket(ZMQContext* context_, Type type_);

    private:
        friend class ZMQContext;

        ZMQContext* m_context;
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS(ZMQSocket::Events)
    Q_DECLARE_OPERATORS_FOR_FLAGS(ZMQSocket::SendFlags)
    Q_DECLARE_OPERATORS_FOR_FLAGS(ZMQSocket::ReceiveFlags)


    // This class is an abstract base class for concrete implementations.
    class NZMQT_API ZMQContext : public QObject, private zmq::context_t
    {
        Q_OBJECT

        typedef QObject qsuper;
        typedef zmq::context_t zmqsuper;

        friend class ZMQSocket;

    public:
        ZMQContext(QObject* parent_ = 0, int io_threads_ = NZMQT_DEFAULT_IOTHREADS);

        // Deleting children is necessary, because otherwise the children are deleted after the context
        // which results in a blocking state. So we delete the children before the zmq::context_t
        // destructor implementation is called.
        ~ZMQContext();

        using zmqsuper::operator void*;

        // Creates a socket instance of the specified type and parent.
        // The created instance will have the specified parent
        // (as usual you can also call 'ZMQSocket::setParent()' method to change
        // ownership later on). Make sure, however, that the socket's parent
        // belongs to the same thread as the socket instance itself (as it is required
        // by Qt). Otherwise, you will encounter strange errors.
        ZMQSocket* createSocket(ZMQSocket::Type type_, QObject* parent_ = 0);

        // Start watching for incoming messages.
        virtual void start() = 0;

        // Stop watching for incoming messages.
        virtual void stop() = 0;

        // Indicates if watching for incoming messages is enabled.
        virtual bool isStopped() const = 0;

    protected:
        typedef QVector<ZMQSocket*> Sockets;

        // Creates a socket instance of the specified type.
        virtual ZMQSocket* createSocketInternal(ZMQSocket::Type type_) = 0;

        virtual void registerSocket(ZMQSocket* socket_);

        // Remove the given socket object from the list of registered sockets.
        virtual void unregisterSocket(ZMQSocket* socket_);

        virtual const Sockets& registeredSockets() const;

    private:
        Sockets m_sockets;
    };


    class NZMQT_API ZMQDevice : public QObject, public QRunnable
    {
        Q_OBJECT
        Q_ENUMS(Type)

    public:
        enum Type
        {
            TYP_QUEUE = ZMQ_QUEUE,
            TYP_FORWARDED = ZMQ_FORWARDER,
            TYP_STREAMER = ZMQ_STREAMER
        };

        ZMQDevice(Type type, ZMQSocket* frontend, ZMQSocket* backend);

        void run();

    private:
        Type type_;
        ZMQSocket* frontend_;
        ZMQSocket* backend_;
    };


    class PollingZMQContext;

    // An instance of this class cannot directly be created. Use one
    // of the 'PollingZMQContext::createSocket()' factory methods instead.
    class NZMQT_API PollingZMQSocket : public ZMQSocket
    {
        Q_OBJECT

        typedef ZMQSocket super;

        friend class PollingZMQContext;

    protected:
        PollingZMQSocket(PollingZMQContext* context_, Type type_);

        // This method is called by the socket's context object in order
        // to signal a new received message.
        void onMessageReceived(const QList<QByteArray>& message);
    };

    class NZMQT_API PollingZMQContext : public ZMQContext, public QRunnable
    {
        Q_OBJECT

        typedef ZMQContext super;

    public:
        PollingZMQContext(QObject* parent_ = 0, int io_threads_ = NZMQT_DEFAULT_IOTHREADS);

        // Sets the polling interval.
        // Note that the interval does not denote the time the zmq::poll() function will
        // block in order to wait for incoming messages. Instead, it denotes the time in-between
        // consecutive zmq::poll() calls.
        void setInterval(int interval_);

        int getInterval() const;

        // Starts the polling process by scheduling a call to the 'run()' method into Qt's event loop.
        void start();

        // Stops the polling process in the sense that no further 'run()' calls will be scheduled into
        // Qt's event loop.
        void stop();

        bool isStopped() const;

    public slots:
        // If the polling process is not stopped (by a previous call to the 'stop()' method) this
        // method will call the 'poll()' method once and re-schedule a subsequent call to this method
        // using the current polling interval.
        void run();

        // This method will poll on all currently available poll-items (known ZMQ sockets)
        // using the given timeout to wait for incoming messages. Note that this timeout has
        // nothing to do with the polling interval. Instead, the poll method will block the current
        // thread by waiting at most the specified amount of time for incoming messages.
        // This method is public because it can be called directly if you need to.
        void poll(long timeout_ = 0);

    signals:
        // This signal will be emitted by run() method if a call to poll(...) method
        // results in an exception.
        void pollError(int errorNum, const QString& errorMsg);

    protected:
        PollingZMQSocket* createSocketInternal(ZMQSocket::Type type_);

        // Add the given socket to list list of poll-items.
        void registerSocket(ZMQSocket* socket_);

        // Remove the given socket object from the list of poll-items.
        void unregisterSocket(ZMQSocket* socket_);

    private:
        typedef QVector<pollitem_t> PollItems;

        PollItems m_pollItems;
        QMutex m_pollItemsMutex;
        int m_interval;
        volatile bool m_stopped;
    };


    // An instance of this class cannot directly be created. Use one
    // of the 'SocketNotifierZMQContext::createSocket()' factory methods instead.
    class NZMQT_API SocketNotifierZMQSocket : public ZMQSocket
    {
        Q_OBJECT

        friend class SocketNotifierZMQContext;

        typedef ZMQSocket super;

//    public:
//        using super::sendMessage;

//        bool sendMessage(const QByteArray& bytes_, SendFlags flags_ = SND_NOBLOCK);

    protected:
        SocketNotifierZMQSocket(ZMQContext* context_, Type type_);

    protected slots:
        void socketReadActivity();

//        void socketWriteActivity();

    private:
        QSocketNotifier *socketNotifyRead_;
//        QSocketNotifier *socketNotifyWrite_;
    };

    class NZMQT_API SocketNotifierZMQContext : public ZMQContext
    {
        Q_OBJECT

        typedef ZMQContext super;

    public:
        SocketNotifierZMQContext(QObject* parent_ = 0, int io_threads_ = NZMQT_DEFAULT_IOTHREADS);

        void start();

        void stop();

        bool isStopped() const;

    protected:
        SocketNotifierZMQSocket* createSocketInternal(ZMQSocket::Type type_);
    };

    NZMQT_API inline ZMQContext* createDefaultContext(QObject* parent_ = 0, int io_threads_ = NZMQT_DEFAULT_IOTHREADS)
    {
        return new NZMQT_DEFAULT_ZMQCONTEXT_IMPLEMENTATION(parent_, io_threads_);
    }
}

#if !defined(NZMQT_LIB)
 #include "nzmqt/impl.hpp"
#endif

#endif // NZMQT_H

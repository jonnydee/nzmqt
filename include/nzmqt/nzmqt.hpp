// Copyright 2011-2012 Johann Duscher. All rights reserved.
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

#include <zmq.hpp>

#include <QDebug>
#include <QObject>
#include <QList>
#include <QPair>
#include <QByteArray>
#include <QSocketNotifier>
#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QRunnable>

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

namespace nzmqt
{
    typedef zmq::free_fn free_fn;
    typedef zmq::pollitem_t pollitem_t;

    typedef zmq::error_t ZMQException;

    using zmq::poll;
    using zmq::device;
    using zmq::version;

    // This class wraps ZMQ's message structure.
    class ZMQMessage : private zmq::message_t
    {
        friend class ZMQSocket;

        typedef zmq::message_t super;

    public:
        inline ZMQMessage() : super() {}

        inline ZMQMessage(size_t size_) : super(size_) {}

        inline ZMQMessage(void* data_, size_t size_, free_fn *ffn_, void* hint_ = 0)
            : super(data_, size_, ffn_, hint_) {}

        inline ZMQMessage(const QByteArray& b) : super(b.size())
        {
            memcpy(data(), b.constData(), b.size());
        }

        using super::rebuild;

        inline void move(ZMQMessage* msg_)
        {
            super::move(static_cast<zmq::message_t*>(msg_));
        }

        inline void copy(ZMQMessage* msg_)
        {
            super::copy(msg_);
        }

        inline void clone(ZMQMessage* msg_)
        {
            rebuild(msg_->size());
            memcpy(data(), msg_->data(), size());
        }

        using super::data;

        using super::size;

        inline QByteArray toByteArray()
        {
            return QByteArray((const char *)data(), size());
        }
    };

    // This class cannot be instantiated. Its purpose is to serve as an
    // intermediate base class that provides Qt-based convenience methods
    // to subclasses.
    class ZMQSocket : public QObject, private zmq::socket_t
    {
        Q_OBJECT

        typedef QObject qsuper;
        typedef zmq::socket_t zmqsuper;

    public:
        using zmqsuper::operator void *;

        using zmqsuper::close;

        inline void setOption(int option_, const void *optionVal_, size_t optionValLen_)
        {
            setsockopt(option_, optionVal_, optionValLen_);
        }

        inline void setOption(int optName_, const char* str_)
        {
            setOption(optName_, str_, strlen(str_));
        }

        inline void setOption(int optName_, const QByteArray& bytes_)
        {
            setOption(optName_, bytes_.constData(), bytes_.size());
        }

        inline void setOption(int optName_, int value_)
        {
            setOption(optName_, &value_, sizeof(value_));
        }

        inline void getOption(int option_, void *optval_, size_t *optvallen_) const
        {
            const_cast<ZMQSocket*>(this)->getsockopt(option_, optval_, optvallen_);
        }

        inline void bindTo(const QString& addr_)
        {
            bind(addr_.toLocal8Bit());
        }

        inline void bindTo(const char *addr_)
        {
            bind(addr_);
        }

        inline void connectTo(const QString& addr_)
        {
            zmqsuper::connect(addr_.toLocal8Bit());
        }

        inline void connectTo(const char* addr_)
        {
            zmqsuper::connect(addr_);
        }

        inline bool sendMessage(ZMQMessage& msg_, int flags_ = ZMQ_NOBLOCK)
        {
            return send(msg_, flags_);
        }

        inline bool sendMessage(const QByteArray& bytes_, int flags_ = ZMQ_NOBLOCK)
        {
            ZMQMessage msg(bytes_);
            return send(msg, flags_);
        }

        // Interprets the provided list of byte arrays as a multi-part message
        // and sends them accordingly.
        // If an empty list is provided this method doesn't do anything and returns trua.
        inline bool sendMessage(const QList<QByteArray>& msg_, int flags_ = ZMQ_NOBLOCK)
        {
            int i;
            for (i = 0; i < msg_.size() - 1; i++)
            {
                if (!sendMessage(msg_[i], flags_ | ZMQ_SNDMORE))
                    return false;
            }
            if (i < msg_.size())
                return sendMessage(msg_[i], flags_);

            return true;
        }

        // Receives a message or a message part.
        inline bool receiveMessage(ZMQMessage* msg_, int flags_ = ZMQ_NOBLOCK)
        {
            return recv(msg_, flags_);
        }

        // Receives a message.
        // The message is represented as a list of byte arrays representing
        // a message's parts. If the message is not a multi-part message the
        // list will only contain one array.
        inline QList<QByteArray> receiveMessage()
        {
            QList<QByteArray> parts;

            ZMQMessage msg;
            while (receiveMessage(&msg))
            {
                parts += msg.toByteArray();
                msg.rebuild();

                if (!hasMoreMessageParts())
                    break;
            }

            return parts;
        }

        // Receives all messages currently available.
        // Each message is represented as a list of byte arrays representing the messages
        // and their parts in case of multi-part messages. If a message isn't a multi-part
        // message the corresponding byte array list will only contain one element.
        // Note that this method won't work with REQ-REP protocol.
        inline QList< QList<QByteArray> > receiveMessages()
        {
            QList< QList<QByteArray> > ret;

            QList<QByteArray> parts = receiveMessage();
            while (!parts.isEmpty())
            {
                ret += parts;

                parts = receiveMessage();
            }

            return ret;
        }

        inline int fileDescriptor() const
        {
            int value;
            size_t size = sizeof(value);
            getOption(ZMQ_FD, &value, &size);
            return value;
        }

        inline quint32 flags() const
        {
            quint32 value;
            size_t size = sizeof(value);
            getOption(ZMQ_EVENTS, &value, &size);
            return value;
        }

        // Returns true if there are more parts of a multi-part message
        // to be received.
        inline bool hasMoreMessageParts() const
        {
            quint64 value;
            size_t size = sizeof(value);
            getOption(ZMQ_RCVMORE, &value, &size);
            return value;
        }

        inline void setIdentity(const char* nameStr_)
        {
            setOption(ZMQ_IDENTITY, nameStr_);
        }

        inline void setIdentity(const QString& name_)
        {
            setOption(ZMQ_IDENTITY, name_.toLocal8Bit());
        }

        inline void setIdentity(const QByteArray& name_)
        {
            setOption(ZMQ_IDENTITY, const_cast<char*>(name_.constData()), name_.size());
        }

        inline QByteArray identity() const
        {
            char idbuf[256];
            size_t size = sizeof(idbuf);
            getOption(ZMQ_IDENTITY, idbuf, &size);
            return QByteArray(idbuf, size);
        }

        inline void setLinger(int msec_)
        {
            setOption(ZMQ_LINGER, msec_);
        }

        inline int linger() const
        {
            int msec=-1;
            size_t size = sizeof(msec);
            getOption(ZMQ_LINGER, &msec, &size);
            return msec;
        }

        inline void subscribeTo(const char* filterStr_)
        {
            setOption(ZMQ_SUBSCRIBE, filterStr_);
        }

        inline void subscribeTo(const QString& filter_)
        {
            setOption(ZMQ_SUBSCRIBE, filter_.toLocal8Bit());
        }

        inline void subscribeTo(const QByteArray& filter_)
        {
            setOption(ZMQ_SUBSCRIBE, filter_);
        }

        inline void unsubscribeFrom(const char* filterStr_)
        {
            setOption(ZMQ_UNSUBSCRIBE, filterStr_);
        }

        inline void unsubscribeFrom(const QString& filter_)
        {
            setOption(ZMQ_UNSUBSCRIBE, filter_.toLocal8Bit());
        }

        inline void unsubscribeFrom(const QByteArray& filter_)
        {
            setOption(ZMQ_UNSUBSCRIBE, filter_);
        }

    protected:
        inline ZMQSocket(zmq::context_t* context_, int type_)
            : qsuper(0), zmqsuper(*context_, type_) {}
    };

    // This class is an abstract base class for concrete implementations.
    class ZMQContext : public QObject, protected zmq::context_t
    {
        Q_OBJECT

        typedef QObject qsuper;
        typedef zmq::context_t zmqsuper;

    public:
        inline ZMQContext(int io_threads_, QObject* parent_ = 0)
            : qsuper(parent_), zmqsuper(io_threads_) {}

        // Deleting children is necessary, because otherwise the children are deleted after the context
        // which results in a blocking state. So we delete the children before the zmq::context_t
        // destructor implementation is called.
        inline ~ZMQContext()
        {
            QObjectList children_ = children();
            foreach (QObject* child, children_)
                delete child;
        }

        using zmqsuper::operator void*;

        // Creates a socket instance of the specified type.
        // The created instance will have this context set as its parent,
        // so deleting this context will first delete the socket.
        // You can call 'ZMQSocket::setParent()' method to change ownership,
        // but then you need to make sure the socket instance is deleted
        // before its context. Otherwise, you might encounter blocking
        // behavior.
        inline ZMQSocket* createSocket(int type_)
        {
            return createSocket(type_, this);
        }

        // Creates a socket instance of the specified type and parent.
        // The created instance will have the specified parent.
        // You can also call 'ZMQSocket::setParent()' method to change
        // ownership later on, but then you need to make sure the socket
        // instance is deleted before its context. Otherwise, you might
        // encounter blocking behavior.
        inline ZMQSocket* createSocket(int type_, QObject* parent_)
        {
            ZMQSocket* socket = createSocketInternal(type_);
            socket->setParent(parent_);
            return socket;
        }

    protected:
        // Creates a socket instance of the specified type.
        virtual ZMQSocket* createSocketInternal(int type_) = 0;
    };


    // An instance of this class cannot directly be created. Use one
    // of the 'PollingZMQContext::createSocket()' factory methods instead.
    class PollingZMQSocket : public ZMQSocket
    {
        Q_OBJECT

        typedef ZMQSocket super;

        friend class PollingZMQContext;

    protected:
        inline PollingZMQSocket(zmq::context_t* context_, int type_)
            : super(context_, type_) {}

        // This method is called by the socket's context object in order
        // to signal a new received message.
        inline void onMessageReceived(const QList<QByteArray>& message)
        {
            emit messageReceived(message);
        }

    signals:
        void messageReceived(const QList<QByteArray>&);
    };

    class PollingZMQContext : public ZMQContext, public QRunnable
    {
        Q_OBJECT

        typedef ZMQContext super;

    public:
        inline PollingZMQContext(int io_threads_ = NZMQT_DEFAULT_IOTHREADS, QObject* parent_ = 0)
            : super(io_threads_, parent_),
              m_interval(NZMQT_POLLINGZMQCONTEXT_DEFAULT_POLLINTERVAL),
              m_stopped(false)
        {
            setAutoDelete(false);
        }

        // Sets the polling interval.
        // Note that the interval does not denote the time the zmq::poll() function will
        // block in order to wait for incoming messages. Instead, it denotes the time in-between
        // consecutive zmq::poll() calls.
        inline void setInterval(int interval_)
        {
            m_interval = interval_;
        }

        inline int getInterval() const
        {
            return m_interval;
        }

        // Starts the polling process by scheduling a call to the 'run()' method into Qt's event loop.
        inline void start()
        {
            m_stopped = false;
            QTimer::singleShot(0, this, SLOT(run()));
        }

        // Stops the polling process in the sense that no further 'run()' calls will be scheduled into
        // Qt's event loop.
        inline void stop()
        {
            m_stopped = true;
        }

        inline bool isStopped() const
        {
            return m_stopped;
        }

    public slots:
        // If the polling process is not stopped (by a previous call to the 'stop()' method) this
        // method will call the 'poll()' method once and re-schedule a subsequent call to this method
        // using the current polling interval.
        inline void run()
        {
            if (m_stopped)
                return;

            poll();

            if (!m_stopped)
                QTimer::singleShot(m_interval, this, SLOT(run()));
        }

        // This method will poll on all currently available poll-items (known ZMQ sockets)
        // using the given timeout to wait for incoming messages. Note that this timeout has
        // nothing to do with the polling interval. Instead, the poll method will block the current
        // thread by waiting at most the specified amount of time for incoming messages.
        // This method is public because it can be called directly if you need to.
        inline void poll(long timeout_ = 0)
        {
            QMutexLocker lock(&m_pollItemsMutex);

            if (m_pollItems.empty())
                return;

            zmq::poll(&m_pollItems[0], m_pollItems.size(), timeout_);

            PollItems::iterator poIt = m_pollItems.begin();
            Sockets::iterator soIt = m_sockets.begin();
            while (poIt != m_pollItems.end())
            {
                if (poIt->revents & ZMQ_POLLIN)
                {
                    QList<QByteArray> message = (*soIt)->receiveMessage();
                    (*soIt)->onMessageReceived(message);
                }
                ++soIt;
                ++poIt;
            }
        }

    protected:
        inline PollingZMQSocket* createSocketInternal(int type_)
        {
            PollingZMQSocket* socket = new PollingZMQSocket(this, type_);
            // Make sure the socket is removed from the poll-item list as soon
            // as it is destroyed.
            connect(socket, SIGNAL(destroyed(QObject*)), SLOT(unregisterSocket(QObject*)));
            // Add the socket to the poll-item list.
            registerSocket(socket);
            return socket;
        }

        // Add the given socket to list list of poll-items.
        inline void registerSocket(PollingZMQSocket* socket_)
        {
            pollitem_t pollItem = { *socket_, 0, ZMQ_POLLIN, 0 };

            QMutexLocker lock(&m_pollItemsMutex);
            m_sockets.push_back(socket_);
            m_pollItems.push_back(pollItem);
        }

    protected slots:
        // Remove the given socket object from the list of poll-items.
        inline void unregisterSocket(QObject* socket_)
        {
            QMutexLocker lock(&m_pollItemsMutex);

            PollItems::iterator poIt = m_pollItems.begin();
            Sockets::iterator soIt = m_sockets.begin();
            while (soIt != m_sockets.end())
            {
                if (*soIt == socket_)
                {
                    m_sockets.erase(soIt);
                    m_pollItems.erase(poIt);
                    break;
                }
                ++soIt;
                ++poIt;
            }
        }

    private:
        typedef QVector<pollitem_t> PollItems;
        typedef QVector<PollingZMQSocket*> Sockets;

        Sockets m_sockets;
        PollItems m_pollItems;
        QMutex m_pollItemsMutex;
        int m_interval;
        volatile bool m_stopped;
    };


    // An instance of this class cannot directly be created. Use one
    // of the 'SocketNotifierZMQContext::createSocket()' factory methods instead.
    class SocketNotifierZMQSocket : public ZMQSocket
    {
        Q_OBJECT

        friend class SocketNotifierZMQContext;

        typedef ZMQSocket super;

    public:
        using super::sendMessage;

        inline bool sendMessage(const QByteArray& bytes_, int flags_ = ZMQ_NOBLOCK)
        {
            bool result = super::sendMessage(bytes_, flags_);

            if (!result)
                socketNotifyWrite_->setEnabled(true);

            return result;
        }

    protected:
        inline SocketNotifierZMQSocket(zmq::context_t* context_, int type_)
            : super(context_, type_)
        {
            int fd = fileDescriptor();

            socketNotifyRead_ = new QSocketNotifier(fd, QSocketNotifier::Read, this);
            QObject::connect(socketNotifyRead_, SIGNAL(activated(int)), this, SLOT(socketReadActivity()));

            socketNotifyWrite_ = new QSocketNotifier(fd, QSocketNotifier::Write, this);
            QObject::connect(socketNotifyWrite_, SIGNAL(activated(int)), this, SLOT(socketWriteActivity()));
        }

        inline void onSocketActivity(quint32 flags_)
        {
            if(flags_ & ZMQ_POLLIN) {
                emit readyRead();
            }
            if(flags_ & ZMQ_POLLOUT) {
                emit readyWrite();
            }
            if(flags_ & ZMQ_POLLERR) {
                emit pollError();
            }
        }

    protected slots:
        inline void socketReadActivity()
        {
            quint32 flags_ = flags();
            onSocketActivity(flags_);
        }

        inline void socketWriteActivity()
        {
            quint32 flags_ = flags();
            if (flags_ == 0)
            {
                socketNotifyWrite_->setEnabled(false);
            }
            onSocketActivity(flags_);
        }

    signals:
        void readyRead();
        void readyWrite();
        void pollError();

    private:
        QSocketNotifier *socketNotifyRead_;
        QSocketNotifier *socketNotifyWrite_;
    };

    class SocketNotifierZMQContext : public ZMQContext
    {
        Q_OBJECT

        typedef ZMQContext super;

    public:
        inline SocketNotifierZMQContext(int io_threads_ = NZMQT_DEFAULT_IOTHREADS, QObject* parent_ = 0)
            : super(io_threads_, parent_)
        {
        }

    protected:
        inline SocketNotifierZMQSocket* createSocketInternal(int type_)
        {
            return new SocketNotifierZMQSocket(this, type_);
        }
    };
}


#endif // NZMQT_H

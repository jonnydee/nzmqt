// Copyright 2011 Johann Duscher. All rights reserved.
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

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QSocketNotifier>


namespace nzmqt
{
    typedef zmq::free_fn free_fn;
    typedef zmq::pollitem_t pollitem_t;

    typedef zmq::error_t ZMQException;

    using zmq::poll;
    using zmq::device;
    using zmq::version;

    class ZMQMessage : private zmq::message_t
    {
        friend class ZMQSocket;

        typedef zmq::message_t super;

    public:
        inline ZMQMessage() : super() {}

        inline ZMQMessage(size_t size_) : super(size_) {}

        inline ZMQMessage(void* data_, size_t size_, free_fn *ffn_, void* hint_ = 0)
            : super(data_, size_, ffn_, hint_) {}

        inline ZMQMessage(const QByteArray& b) : super(b.size()) {
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

    // An instance of this class cannot directly be created. Use one
    // of the 'ZMQContext::createSocket()' methods instead.
    class ZMQSocket : public QObject, private zmq::socket_t
    {
        Q_OBJECT

        friend class ZMQContext;

        typedef QObject qsuper;
        typedef zmq::socket_t zmqsuper;

    public:
        using zmqsuper::operator void *;

        using zmqsuper::close;

        inline void setOption(int option_, const void *optionVal_, size_t optionValLen_)
        {
            zmqsuper::setsockopt(option_, optionVal_, optionValLen_);
        }

        inline void setOption(int optName_, const char* str_) {
            setOption(optName_, str_, strlen(str_));
        }

        inline void setOption(int optName_, const QByteArray& bytes_) {
            setOption(optName_, bytes_.constData(), bytes_.size());
        }

        inline void setOption(int optName_, int value_) {
            setOption(optName_, &value_, sizeof(value_));
        }

        inline void getOption(int option_, void *optval_, size_t *optvallen_)
        {
            zmqsuper::getsockopt(option_, optval_, optvallen_);
        }

        inline void bindTo(const QString& addr_)
        {
            zmqsuper::bind(addr_.toLocal8Bit());
        }

        inline void bindTo(const char *addr_)
        {
            zmqsuper::bind(addr_);
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
            return zmqsuper::send(msg_, flags_);
        }

        inline bool sendMessage(const QByteArray& bytes_, int flags_ = ZMQ_NOBLOCK) {
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
            return zmqsuper::recv(msg_, flags_);
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

        inline int fileDescriptor()
        {
            int value;
            size_t size = sizeof(value);
            getOption(ZMQ_FD, &value, &size);
            return value;
        }

        inline quint32 flags()
        {
            quint32 value;
            size_t size = sizeof(value);
            getOption(ZMQ_EVENTS, &value, &size);
            return value;
        }

        // Returns true if there are more parts of a multi-part message
        // to be received.
        inline bool hasMoreMessageParts()
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

        inline QByteArray identity()
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

        inline int linger()
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
            : qsuper(0), zmqsuper(*context_, type_)
        {
            int fd = fileDescriptor();

            socketNotifyRead_ = new QSocketNotifier(fd, QSocketNotifier::Read, this);
            qsuper::connect(socketNotifyRead_, SIGNAL(activated(int)), this, SLOT(socketActivity()));
        }

    signals:
        void readyRead();
        void readyWrite();
        void pollError();

    protected slots:
        inline void socketActivity()
        {
            quint32 flags_ = flags();

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

    private:
        QSocketNotifier *socketNotifyRead_;
    };

    class ZMQContext : public QObject, private zmq::context_t
    {
        Q_OBJECT

        typedef QObject qsuper;
        typedef zmq::context_t zmqsuper;

    public:
        inline ZMQContext(int io_threads_, QObject* parent_ = 0) : qsuper(parent_), zmqsuper(io_threads_) {}

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

        // Creates a socket instance with the specified type.
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

        // Creates a socket instance with the specified type and parent.
        // The created instance will have this context set as its parent.
        // You can also call 'ZMQSocket::setParent()' method to change
        // ownership later on, but then you need to make sure the socket
        // instance is deleted before its context. Otherwise, you might
        // encounter blocking behavior.
        inline virtual ZMQSocket* createSocket(int type_, QObject* parent_)
        {
            ZMQSocket* socket = new ZMQSocket(this, type_);
            socket->setParent(parent_);
            return socket;
        }
    };
}


#endif // NZMQT_H

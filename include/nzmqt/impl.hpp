// Copyright 2011-2014 Johann Duscher (a.k.a. Jonny Dee). All rights reserved.
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

#ifndef NZMQT_IMPL_HPP
#define NZMQT_IMPL_HPP

#include "nzmqt/nzmqt.hpp"

#include <QDebug>
#include <QMutexLocker>
#include <QSocketNotifier>
#include <QTimer>
#include <climits>

#if defined(NZMQT_LIB)
// #pragma message("nzmqt is built as library")
 #define NZMQT_INLINE
#else
// #pragma message("nzmqt is built inline")
 #define NZMQT_INLINE inline
#endif

namespace nzmqt
{

/*
 * ZMQMessage
 */

NZMQT_INLINE ZMQMessage::ZMQMessage()
    : super()
{
}

NZMQT_INLINE ZMQMessage::ZMQMessage(size_t size_)
    : super(size_)
{
}

NZMQT_INLINE ZMQMessage::ZMQMessage(void* data_, size_t size_, free_fn *ffn_, void* hint_)
    : super(data_, size_, ffn_, hint_)
{
}

NZMQT_INLINE ZMQMessage::ZMQMessage(const QByteArray& b)
    : super(b.size())
{
    memcpy(data(), b.constData(), b.size());
}

NZMQT_INLINE void ZMQMessage::move(ZMQMessage* msg_)
{
    super::move(static_cast<zmq::message_t*>(msg_));
}

NZMQT_INLINE void ZMQMessage::clone(ZMQMessage* msg_)
{
    rebuild(msg_->size());
    memcpy(data(), msg_->data(), size());
}

NZMQT_INLINE QByteArray ZMQMessage::toByteArray()
{
    return size() <= INT_MAX ? QByteArray(data<char>(), int(size())) : QByteArray();
}



/*
 * ZMQSocket
 */

NZMQT_INLINE ZMQSocket::ZMQSocket(ZMQContext* context_, Type type_)
    : qsuper(nullptr)
    , zmqsuper(*context_, type_)
    , m_context(context_)
{
}

NZMQT_INLINE ZMQSocket::~ZMQSocket()
{
//    qDebug() << Q_FUNC_INFO << "Context:" << m_context;
    close();
}

NZMQT_INLINE void ZMQSocket::close()
{
//    qDebug() << Q_FUNC_INFO << "Context:" << m_context;
    if (m_context)
    {
        m_context->unregisterSocket(this);
        m_context = nullptr;
    }
    zmqsuper::close();
}

NZMQT_INLINE void ZMQSocket::setOption(Option optName_, const void *optionVal_, size_t optionValLen_)
{
    setsockopt(optName_, optionVal_, optionValLen_);
}

NZMQT_INLINE void ZMQSocket::setOption(Option optName_, const char* str_)
{
    setOption(optName_, str_, strlen(str_));
}

NZMQT_INLINE void ZMQSocket::setOption(Option optName_, const QByteArray& bytes_)
{
    setOption(optName_, bytes_.constData(), bytes_.size());
}

NZMQT_INLINE void ZMQSocket::getOption(Option option_, void *optval_, size_t *optvallen_) const
{
    const_cast<ZMQSocket*>(this)->getsockopt(option_, optval_, optvallen_);
}

NZMQT_INLINE void ZMQSocket::bindTo(const QString& addr_)
{
    bind(addr_.toLocal8Bit());
}

NZMQT_INLINE void ZMQSocket::bindTo(const char *addr_)
{
    bind(addr_);
}

NZMQT_INLINE void ZMQSocket::unbindFrom(const QString& addr_)
{
    unbind(addr_.toLocal8Bit());
}

NZMQT_INLINE void ZMQSocket::unbindFrom(const char *addr_)
{
    unbind(addr_);
}

NZMQT_INLINE void ZMQSocket::connectTo(const QString& addr_)
{
    zmqsuper::connect(addr_.toLocal8Bit());
}

NZMQT_INLINE void ZMQSocket::connectTo(const char* addr_)
{
    zmqsuper::connect(addr_);
}

NZMQT_INLINE void ZMQSocket::disconnectFrom(const QString& addr_)
{
    zmqsuper::disconnect(addr_.toLocal8Bit());
}

NZMQT_INLINE void ZMQSocket::disconnectFrom(const char* addr_)
{
    zmqsuper::disconnect(addr_);
}

NZMQT_INLINE bool ZMQSocket::sendMessage(ZMQMessage& msg_, SendFlags flags_)
{
    return send(msg_, flags_);
}

NZMQT_INLINE bool ZMQSocket::sendMessage(const QByteArray& bytes_, SendFlags flags_)
{
    ZMQMessage msg(bytes_);
    return send(msg, flags_);
}

NZMQT_INLINE bool ZMQSocket::sendMessage(const QList<QByteArray>& msg_, SendFlags flags_)
{
    int i;
    for (i = 0; i < msg_.size() - 1; i++)
    {
        if (!sendMessage(msg_[i], flags_ | SND_MORE))
            return false;
    }
    if (i < msg_.size())
        return sendMessage(msg_[i], flags_);

    return true;
}

NZMQT_INLINE bool ZMQSocket::receiveMessage(ZMQMessage* msg_, ReceiveFlags flags_)
{
    return recv(msg_, flags_);
}

NZMQT_INLINE QList<QByteArray> ZMQSocket::receiveMessage(ReceiveFlags flags_)
{
    QList<QByteArray> parts;

    ZMQMessage msg;
    while (receiveMessage(&msg, flags_))
    {
        parts += msg.toByteArray();
        msg.rebuild();

        if (!hasMoreMessageParts())
            break;
    }

    return parts;
}

NZMQT_INLINE QList< QList<QByteArray> > ZMQSocket::receiveMessages(ReceiveFlags flags_)
{
    QList< QList<QByteArray> > ret;

    QList<QByteArray> parts = receiveMessage(flags_);
    while (!parts.isEmpty())
    {
        ret += std::move(parts);

        parts = receiveMessage(flags_);
    }

    return ret;
}

NZMQT_INLINE qintptr ZMQSocket::fileDescriptor() const
{
    qintptr value;
    size_t size = sizeof(value);
    getOption(OPT_FD, &value, &size);
    return value;
}

NZMQT_INLINE ZMQSocket::Events ZMQSocket::events() const
{
    qint32 value;
    size_t size = sizeof(value);
    getOption(OPT_EVENTS, &value, &size);
    return static_cast<Events>(value);
}

// Returns true if there are more parts of a multi-part message
// to be received.
NZMQT_INLINE bool ZMQSocket::hasMoreMessageParts() const
{
    qint32 value;
    size_t size = sizeof(value);
    getOption(OPT_RCVMORE, &value, &size);
    return value;
}

NZMQT_INLINE void ZMQSocket::setIdentity(const char* nameStr_)
{
    setOption(OPT_IDENTITY, nameStr_);
}

NZMQT_INLINE void ZMQSocket::setIdentity(const QString& name_)
{
    setOption(OPT_IDENTITY, name_.toLocal8Bit());
}

NZMQT_INLINE void ZMQSocket::setIdentity(const QByteArray& name_)
{
    setOption(OPT_IDENTITY, const_cast<char*>(name_.constData()), name_.size());
}

NZMQT_INLINE QByteArray ZMQSocket::identity() const
{
    char idbuf[256];
    size_t size = sizeof(idbuf);
    getOption(OPT_IDENTITY, idbuf, &size);
    return QByteArray(idbuf, int(size));
}

NZMQT_INLINE void ZMQSocket::setLinger(int msec_)
{
    setOption(OPT_LINGER, msec_);
}

NZMQT_INLINE qint32 ZMQSocket::linger() const
{
    qint32 msec=-1;
    size_t size = sizeof(msec);
    getOption(OPT_LINGER, &msec, &size);
    return msec;
}

NZMQT_INLINE void ZMQSocket::subscribeTo(const char* filterStr_)
{
    setOption(OPT_SUBSCRIBE, filterStr_);
}

NZMQT_INLINE void ZMQSocket::subscribeTo(const QString& filter_)
{
    setOption(OPT_SUBSCRIBE, filter_.toLocal8Bit());
}

NZMQT_INLINE void ZMQSocket::subscribeTo(const QByteArray& filter_)
{
    setOption(OPT_SUBSCRIBE, filter_);
}

NZMQT_INLINE void ZMQSocket::unsubscribeFrom(const char* filterStr_)
{
    setOption(OPT_UNSUBSCRIBE, filterStr_);
}

NZMQT_INLINE void ZMQSocket::unsubscribeFrom(const QString& filter_)
{
    setOption(OPT_UNSUBSCRIBE, filter_.toLocal8Bit());
}

NZMQT_INLINE void ZMQSocket::unsubscribeFrom(const QByteArray& filter_)
{
    setOption(OPT_UNSUBSCRIBE, filter_);
}

NZMQT_INLINE void ZMQSocket::setSendHighWaterMark(int value_)
{
    setOption(OPT_SNDHWM, value_);
}

NZMQT_INLINE void ZMQSocket::setReceiveHighWaterMark(int value_)
{
    setOption(OPT_RCVHWM, value_);
}

NZMQT_INLINE bool ZMQSocket::isConnected()
{
    return const_cast<ZMQSocket*>(this)->connected();
}

/*
 * ZMQContext
 */

NZMQT_INLINE ZMQContext::ZMQContext(QObject* parent_, int io_threads_)
    : qsuper(parent_)
    , zmqsuper(io_threads_)
{
}

NZMQT_INLINE ZMQContext::~ZMQContext()
{
//    qDebug() << Q_FUNC_INFO << "Sockets:" << m_sockets;
    for(ZMQSocket* socket : registeredSockets())
    {
        socket->m_context = nullptr;
        // As stated by 0MQ, close() must ONLY be called from the thread
        // owning the socket. So we use 'invokeMethod' which (hopefully)
        // results in a 'close' call from within the socket's thread.
        QMetaObject::invokeMethod(socket, "close");
    }
}

NZMQT_INLINE ZMQSocket* ZMQContext::createSocket(ZMQSocket::Type type_, QObject* parent_)
{
    ZMQSocket* socket = createSocketInternal(type_);
    registerSocket(socket);
    socket->setParent(parent_);
    return socket;
}

NZMQT_INLINE void ZMQContext::registerSocket(ZMQSocket* socket_)
{
    m_sockets.push_back(socket_);
}

NZMQT_INLINE void ZMQContext::unregisterSocket(ZMQSocket* socket_)
{
    for(Sockets::iterator soIt = m_sockets.begin(); soIt != m_sockets.end(); ++soIt)
    {
        if (*soIt == socket_)
        {
            m_sockets.erase(soIt);
            break;
        }
    }
}

NZMQT_INLINE const ZMQContext::Sockets& ZMQContext::registeredSockets() const
{
    return m_sockets;
}



/*
 * ZMQDevice
 */
/*
NZMQT_INLINE ZMQDevice::ZMQDevice(Type type, ZMQSocket* frontend, ZMQSocket* backend)
    : type_(type)
    , frontend_(frontend)
    , backend_(backend)
{
}

NZMQT_INLINE void ZMQDevice::run()
{
    zmq::device(type_, *frontend_, *backend_);
}
*/


/*
 * PollingZMQSocket
 */

NZMQT_INLINE PollingZMQSocket::PollingZMQSocket(PollingZMQContext* context_, Type type_)
    : super(context_, type_)
{
}


/*
 * PollingZMQContext
 */

NZMQT_INLINE PollingZMQContext::PollingZMQContext(QObject* parent_, int io_threads_)
    : super(parent_, io_threads_)
    , m_pollItemsMutex(QMutex::Recursive)
    , m_interval(NZMQT_POLLINGZMQCONTEXT_DEFAULT_POLLINTERVAL)
    , m_stopped(false)
{
    setAutoDelete(false);
}

NZMQT_INLINE void PollingZMQContext::setInterval(int interval_)
{
    m_interval = interval_;
}

NZMQT_INLINE int PollingZMQContext::getInterval() const
{
    return m_interval;
}

NZMQT_INLINE void PollingZMQContext::start()
{
    m_stopped = false;
    QTimer::singleShot(0, this, &PollingZMQContext::run);
}

NZMQT_INLINE void PollingZMQContext::stop()
{
    m_stopped = true;
}

NZMQT_INLINE bool PollingZMQContext::isStopped() const
{
    return m_stopped;
}

NZMQT_INLINE void PollingZMQContext::run()
{
    if (m_stopped)
        return;

    try
    {
        poll();
    }
    catch (const ZMQException& ex)
    {
        qWarning("Exception during poll: %s", ex.what());
        emit pollError(ex.num(), ex.what());
    }

    if (!m_stopped)
        QTimer::singleShot(m_interval, this, &PollingZMQContext::run);
}

NZMQT_INLINE void PollingZMQContext::poll(long timeout_)
{
    int cnt;
    do {
        QMutexLocker lock(&m_pollItemsMutex);

        if (m_pollItems.empty())
            return;

        cnt = zmq::poll(&m_pollItems[0], m_pollItems.size(), timeout_);
        Q_ASSERT_X(cnt >= 0, Q_FUNC_INFO, "A value < 0 should be reflected by an exception.");
        if (0 == cnt)
            return;

        PollItems::iterator poIt = m_pollItems.begin();
        ZMQContext::Sockets::const_iterator soIt = registeredSockets().begin();
        int i = 0;
        while (i < cnt && poIt != m_pollItems.end())
        {
            if (poIt->revents & ZMQSocket::EVT_POLLIN)
            {
                PollingZMQSocket* socket = static_cast<PollingZMQSocket*>(*soIt);
                QList<QByteArray> && message = socket->receiveMessage();
                socket->messageReceived(std::move(message));
                i++;
            }
            ++soIt;
            ++poIt;
        }
    } while (cnt > 0);
}

NZMQT_INLINE PollingZMQSocket* PollingZMQContext::createSocketInternal(ZMQSocket::Type type_)
{
    return new PollingZMQSocket(this, type_);
}

NZMQT_INLINE void PollingZMQContext::registerSocket(ZMQSocket* socket_)
{
    pollitem_t pollItem = { *socket_, 0, ZMQSocket::EVT_POLLIN, 0 };

    QMutexLocker lock(&m_pollItemsMutex);

    m_pollItems.push_back(pollItem);

    super::registerSocket(socket_);
}

NZMQT_INLINE void PollingZMQContext::unregisterSocket(ZMQSocket* socket_)
{
    QMutexLocker lock(&m_pollItemsMutex);

    PollItems::iterator poIt = m_pollItems.begin();
    ZMQContext::Sockets::const_iterator soIt = registeredSockets().begin();
    while (soIt != registeredSockets().end())
    {
        if (*soIt == socket_)
        {
            m_pollItems.erase(poIt);
            break;
        }
        ++soIt;
        ++poIt;
    }

    super::unregisterSocket(socket_);
}



/*
 * SocketNotifierZMQSocket
 */

NZMQT_INLINE SocketNotifierZMQSocket::SocketNotifierZMQSocket(ZMQContext* context_, Type type_)
    : super(context_, type_)
    , socketNotifyRead_(0)
    , socketNotifyWrite_(0)
{
    qintptr fd = fileDescriptor();

    socketNotifyRead_ = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    QObject::connect(socketNotifyRead_, &QSocketNotifier::activated, this, &SocketNotifierZMQSocket::socketReadActivity);

    socketNotifyWrite_ = new QSocketNotifier(fd, QSocketNotifier::Write, this);
    QObject::connect(socketNotifyWrite_, &QSocketNotifier::activated, this, &SocketNotifierZMQSocket::socketWriteActivity);
}

NZMQT_INLINE SocketNotifierZMQSocket::~SocketNotifierZMQSocket()
{
    close();
}

NZMQT_INLINE void SocketNotifierZMQSocket::close()
{
    socketNotifyRead_->deleteLater();
    socketNotifyWrite_->deleteLater();
    super::close();
}

NZMQT_INLINE void SocketNotifierZMQSocket::socketReadActivity()
{
    socketNotifyRead_->setEnabled(false);

    try
    {
        while(isConnected() && (events() & EVT_POLLIN))
        {
            const QList<QByteArray> & message = receiveMessage();
            emit messageReceived(message);
        }
    }
    catch (const ZMQException& ex)
    {
        qWarning("Exception during read: %s", ex.what());
        emit notifierError(ex.num(), ex.what());
    }

    socketNotifyRead_->setEnabled(true);
}

NZMQT_INLINE void SocketNotifierZMQSocket::socketWriteActivity()
{
    socketNotifyWrite_->setEnabled(false);

    try
    {
        while (isConnected() && (events() & EVT_POLLIN))
        {
            const QList<QByteArray> & message = receiveMessage();
            emit messageReceived(message);
        }
    }
    catch (const ZMQException& ex)
    {
        qWarning("Exception during write: %s", ex.what());
        emit notifierError(ex.num(), ex.what());
    }

    socketNotifyWrite_->setEnabled(true);
}



/*
 * SocketNotifierZMQContext
 */

NZMQT_INLINE SocketNotifierZMQContext::SocketNotifierZMQContext(QObject* parent_, int io_threads_)
    : super(parent_, io_threads_)
{
}

NZMQT_INLINE void SocketNotifierZMQContext::start()
{
}

NZMQT_INLINE void SocketNotifierZMQContext::stop()
{
}

NZMQT_INLINE bool SocketNotifierZMQContext::isStopped() const
{
    return false;
}

NZMQT_INLINE SocketNotifierZMQSocket* SocketNotifierZMQContext::createSocketInternal(ZMQSocket::Type type_)
{
    SocketNotifierZMQSocket *socket = new SocketNotifierZMQSocket(this, type_);
    connect(socket, &SocketNotifierZMQSocket::notifierError,
            this, &SocketNotifierZMQContext::notifierError);
    return socket;
}

}

#endif // NZMQT_IMPL_HPP

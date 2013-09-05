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

#ifndef NZMQT_SAMPLEBASE_H
#define NZMQT_SAMPLEBASE_H

#include "nzmqt/nzmqt.hpp"

#include <QDebug>
#include <QEventLoop>
#include <QThread>


namespace nzmqt
{

namespace samples
{

class SampleBase : public QObject
{
    Q_OBJECT
    typedef QObject super;

signals:
    void finished();
    void failure(const QString& what);

public slots:
    void start();
    void stop();

protected:
    SampleBase(QObject* parent);

    virtual void startImpl() = 0;

    static void sleep(unsigned long msecs);

private:
    class ThreadTools : private QThread
    {
    public:
        using QThread::msleep;

    private:
        ThreadTools() {}
    };
};

inline SampleBase::SampleBase(QObject* parent)
    : super(parent)
{
}

inline void SampleBase::start()
{
    try
    {
        startImpl();
    }
    catch (const nzmqt::ZMQException& ex)
    {
        qWarning() << Q_FUNC_INFO << "Exception:" << ex.what();
        emit failure(ex.what());
        emit finished();
    }
}

inline void SampleBase::stop()
{
    emit finished();
}

inline void SampleBase::sleep(unsigned long msecs)
{
    ThreadTools::msleep(msecs);
}

}

}

#endif // NZMQT_SAMPLEBASE_H

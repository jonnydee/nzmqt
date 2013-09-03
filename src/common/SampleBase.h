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

#ifndef SAMPLEBASE_HPP
#define SAMPLEBASE_HPP

#include "nzmqt/nzmqt.hpp"

#include <QDebug>
#include <QEventLoop>
#include <QObject>
#include <QRunnable>
#include <QThread>


namespace nzmqt
{

namespace samples
{

class SampleBase : public QObject, public QRunnable
{
    Q_OBJECT
    typedef QObject super;

public slots:
    void run();
    void stop();

protected:
    SampleBase(QObject* parent);

    // Sample subclass needs to implement this method.
    // It will be called by run() method implemented by this class.
    virtual void runImpl() = 0;

    // Sample subclass can use this method to wait for a call to stop()
    // method.
    void waitUntilStopped();

    static void sleep(unsigned long msecs);

private:
    volatile bool stopped_;

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
    , stopped_(true)
{
}

inline void SampleBase::run()
{
    try
    {
        stopped_ = false;
        runImpl();
    }
    catch (const nzmqt::ZMQException& ex)
    {
        qDebug() << Q_FUNC_INFO << "Exception:" << ex.what();
    }
}

inline void SampleBase::stop()
{
    stopped_ = true;
}

inline void SampleBase::waitUntilStopped()
{
    QEventLoop eventLoop;
    while (!stopped_)
    {
        sleep(50);
        eventLoop.processEvents();
    }
}

inline void SampleBase::sleep(unsigned long msecs)
{
    ThreadTools::msleep(msecs);
}

}

}

#endif // SAMPLEBASE_HPP

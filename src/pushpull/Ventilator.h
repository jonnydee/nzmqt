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

#ifndef NZMQT_PUSHPULLVENTILATOR_H
#define NZMQT_PUSHPULLVENTILATOR_H

#include "common/SampleBase.h"

#include <nzmqt/nzmqt.hpp>

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QTimer>


namespace nzmqt
{

namespace samples
{

namespace pushpull
{

class Ventilator : public SampleBase
{
    Q_OBJECT
    typedef SampleBase super;

public:
    explicit Ventilator(ZMQContext& context, const QString& ventilatorAddress, const QString& sinkAddress, quint32 numberOfWorkItems, QObject* parent = 0)
        : super(parent)
        , ventilatorAddress_(ventilatorAddress), sinkAddress_(sinkAddress), numberOfWorkItems_(numberOfWorkItems)
        , ventilator_(0), sink_(0)
    {
        ventilator_ = context.createSocket(ZMQSocket::TYP_PUSH, this);
        ventilator_->setObjectName("Ventilator.Socket.ventilator(PUSH)");

        sink_ = context.createSocket(ZMQSocket::TYP_PUSH, this);
        sink_->setObjectName("Ventilator.Socket.sink(PUSH)");
    }

    int numberOfWorkItems() const
    {
        return numberOfWorkItems_;
    }

    int maxWorkLoad() const
    {
        return 100;
    }

signals:
    void batchStarted(int);
    void workItemSent(quint32 workload);

protected:
    void startImpl()
    {
        ventilator_->bindTo(ventilatorAddress_);
        sink_->connectTo(sinkAddress_);

        // Start batch after some period of time needed to setup workers.
        QTimer::singleShot(1000, this, SLOT(runBatch()));
    }

protected slots:
    void runBatch()
    {
        // The first message tells the sink how much work it needs to do
        // and at the same time signals start of batch.

        sink_->sendMessage(QString::number(numberOfWorkItems()).toLocal8Bit());
        emit batchStarted(numberOfWorkItems());

        // Initialize random number generator.

        qsrand(QTime::currentTime().msec());

        // Send work items.

        int totalExpectedCost = 0; // Total expected cost in msecs

        for (int workItem = 0; workItem < numberOfWorkItems(); workItem++) {
            // Random workload from 1 to 100msecs
            int workload = qrand() % maxWorkLoad() + 1;
            // Update toal cost.
            totalExpectedCost += workload;
            // Push workload.
            ventilator_->sendMessage(QString::number(workload).toLocal8Bit());
            emit workItemSent(workload);
        }

        qDebug() << "Total expected cost: " << totalExpectedCost << " msec";

        QTimer::singleShot(0, this, SLOT(stop()));
    }

private:
    QString ventilatorAddress_;
    QString sinkAddress_;
    quint32 numberOfWorkItems_;

    ZMQSocket* ventilator_;
    ZMQSocket* sink_;
};

}

}

}

#endif // NZMQT_PUSHPULLVENTILATOR_H

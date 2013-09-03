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
#include <QTextStream>


namespace nzmqt
{

namespace samples
{

class PushPullVentilator : public SampleBase
{
    Q_OBJECT
    typedef SampleBase super;

public:
    explicit PushPullVentilator(ZMQContext& context, const QString& ventilatorAddress, const QString& sinkAddress, quint32 numberOfWorkItems, QObject* parent)
        : super(context, parent)
        , ventilatorAddress_(ventilatorAddress), sinkAddress_(sinkAddress), numberOfWorkItems_(numberOfWorkItems)
        , ventilator_(0), sink_(0)
    {
    }

    ~PushPullVentilator()
    {
        delete sink_;
        delete ventilator_;
    }

protected:
    void runImpl()
    {
        ventilator_ = context().createSocket(ZMQSocket::TYP_PUSH);
        ventilator_->bindTo(ventilatorAddress_);
        sink_ = context().createSocket(ZMQSocket::TYP_PUSH);
        sink_->connectTo(sinkAddress_);

        // Wait for user start.

        QTextStream stream(stdin);
        qDebug() << "Available work items:" << numberOfWorkItems_;
        qDebug() << "Press ENTER if workers are ready!";
        stream.readLine();

        // The first message tells the sink how much work it needs to do
        // and at the same time signals start of batch.

        sink_->sendMessage(QString::number(numberOfWorkItems_).toLocal8Bit());

        // Initialize random number generator.

        qsrand(QTime::currentTime().msec());

        // Send work items.

        int totalCost = 0; // Total expected cost in msecs

        for (quint32 workItem = 0; workItem < numberOfWorkItems_; workItem++) {
            // Random workload from 1 to 100msecs
            quint32 workload = qrand() % 100 + 1;;
            // Update toal cost.
            totalCost += workload;
            // Push workload.
            ventilator_->sendMessage(QString::number(workload).toLocal8Bit());
        }

        qDebug() << "Total expected cost: " << totalCost << " msec";
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

#endif // NZMQT_PUSHPULLVENTILATOR_H

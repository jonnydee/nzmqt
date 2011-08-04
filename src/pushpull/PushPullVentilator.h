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

#ifndef PUSHPULLVENTILATOR_H
#define PUSHPULLVENTILATOR_H

#include <QObject>
#include <QRunnable>
#include <QDebug>
#include <QList>
#include <QByteArray>
#include <QTimer>
#include <QDateTime>
#include <QTextStream>

#include "nzmqt/nzmqt.hpp"


class PushPullVentilator : public QObject, QRunnable
{
    Q_OBJECT

    typedef QObject super;

public:
    explicit PushPullVentilator(const QString& ventilatorAddress, const QString& sinkAddress, QObject* parent)
        : super(parent), ventilatorAddress_(ventilatorAddress), sinkAddress_(sinkAddress)
    {
        nzmqt::ZMQContext* context = new nzmqt::ZMQContext(4, this);

        ventilator_ = context->createSocket(ZMQ_PUSH);

        sink_ = context->createSocket(ZMQ_PUSH);
    }

    void run()
    {
        ventilator_->bindTo(ventilatorAddress_);
        sink_->connectTo(sinkAddress_);

        // Wait for user start.

        QTextStream stream(stdin);
        qDebug() << "Press Enter if workers are ready!";
        stream.readLine();

        // The first message is "0" and signals start of batch

        sink_->sendMessage("0");

        // TODO: Initialize random number generator.
        // qsrand(xxx);

        // Send 100 tasks

        int totalCost = 0; // Total expected cost in msecs

        for (int taskNumber = 0; taskNumber < 100; taskNumber++) {
            // Random workload from 1 to 100msecs
            int workload = qrand() % 100 + 1;;
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

    nzmqt::ZMQSocket* ventilator_;
    nzmqt::ZMQSocket* sink_;
};

#endif // PUSHPULLVENTILATOR_H

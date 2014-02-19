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

#include "pubsub/Publisher.hpp"
#include "pubsub/Subscriber.hpp"
#include "reqrep/Requester.hpp"
#include "reqrep/Replier.hpp"
#include "pushpull/Ventilator.hpp"
#include "pushpull/Worker.hpp"
#include "pushpull/Sink.hpp"

#include <QCoreApplication>
#include <QString>
#include <QtTest>

namespace nzmqt
{

class NzmqtTest : public QObject
{
    Q_OBJECT

public:
    NzmqtTest();

protected:
    QThread* makeExecutionThread(samples::SampleBase& sample) const;

private slots:
    void testPubSub();
    void testReqRep();
    void testPushPull();
};

NzmqtTest::NzmqtTest()
{
    qRegisterMetaType< QList<QByteArray> >();
}

QThread* NzmqtTest::makeExecutionThread(samples::SampleBase& sample) const
{
    QThread* thread = new QThread;
    sample.moveToThread(thread);

    bool connected = false;
    connected = connect(thread, SIGNAL(started()), &sample, SLOT(start()));         Q_ASSERT(connected);
    connected = connect(&sample, SIGNAL(finished()), thread, SLOT(quit()));         Q_ASSERT(connected);
    connected = connect(&sample, SIGNAL(finished()), &sample, SLOT(deleteLater())); Q_ASSERT(connected);
    connected = connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));   Q_ASSERT(connected);

    return thread;
}

void NzmqtTest::testPubSub()
{
    try {
        QScopedPointer<ZMQContext> context(nzmqt::createDefaultContext());

        // Create publisher.
        samples::pubsub::Publisher* publisher = new samples::pubsub::Publisher(*context, "inproc://pubsub", "ping");
        QSignalSpy spyPublisherPingSent(publisher, SIGNAL(pingSent(const QList<QByteArray>&)));
        QSignalSpy spyPublisherFailure(publisher, SIGNAL(failure(const QString&)));
        QSignalSpy spyPublisherFinished(publisher, SIGNAL(finished()));
        // Create publisher execution thread.
        QThread* publisherThread = makeExecutionThread(*publisher);
        QSignalSpy spyPublisherThreadFinished(publisherThread, SIGNAL(finished()));

        // Create subscriber.
        samples::pubsub::Subscriber* subscriber = new samples::pubsub::Subscriber(*context, "inproc://pubsub", "ping");
        QSignalSpy spySubscriberPingReceived(subscriber, SIGNAL(pingReceived(const QList<QByteArray>&)));
        QSignalSpy spySubscriberFailure(subscriber, SIGNAL(failure(const QString&)));
        QSignalSpy spySubscriberFinished(subscriber, SIGNAL(finished()));
        // Create subscriber execution thread.
        QThread* subscriberThread = makeExecutionThread(*subscriber);
        QSignalSpy spySubscriberThreadFinished(subscriberThread, SIGNAL(finished()));

        //
        // START TEST
        //

        context->start();

        publisherThread->start();
        QTest::qWait(500);
        subscriberThread->start();

        QTimer::singleShot(6000, publisher, SLOT(stop()));
        QTimer::singleShot(6000, subscriber, SLOT(stop()));

        QTest::qWait(8000);

        //
        // CHECK POSTCONDITIONS
        //

        qDebug() << "Publisher pings sent:" << spyPublisherPingSent.size();
        qDebug() << "Subscriber pings received:" << spySubscriberPingReceived.size();

        QCOMPARE(spyPublisherFailure.size(), 0);
        QCOMPARE(spySubscriberFailure.size(), 0);

        QVERIFY2(spyPublisherPingSent.size() > 3, "Server didn't send any/enough pings.");
        QVERIFY2(spySubscriberPingReceived.size() > 3, "Client didn't receive any/enough pings.");

        QVERIFY2(qAbs(spyPublisherPingSent.size() - spySubscriberPingReceived.size()) < 3, "Publisher and subscriber communication flawed.");

        QCOMPARE(spyPublisherFinished.size(), 1);
        QCOMPARE(spySubscriberFinished.size(), 1);

        QCOMPARE(spyPublisherThreadFinished.size(), 1);
        QCOMPARE(spySubscriberThreadFinished.size(), 1);
    }
    catch (std::exception& ex)
    {
        QFAIL(ex.what());
    }
}

void NzmqtTest::testReqRep()
{
    try {
        QScopedPointer<ZMQContext> context(nzmqt::createDefaultContext());

        // Create replier.
        samples::reqrep::Replier* replier = new samples::reqrep::Replier(*context, "inproc://reqrep", "world");
        QSignalSpy spyReplierRequestReceived(replier, SIGNAL(requestReceived(const QList<QByteArray>&)));
        QSignalSpy spyReplierReplySent(replier, SIGNAL(replySent(const QList<QByteArray>&)));
        QSignalSpy spyReplierFailure(replier, SIGNAL(failure(const QString&)));
        QSignalSpy spyReplierFinished(replier, SIGNAL(finished()));
        // Create replier execution thread.
        QThread* replierThread = makeExecutionThread(*replier);
        QSignalSpy spyReplierThreadFinished(replierThread, SIGNAL(finished()));

        // Create requester.
        samples::reqrep::Requester* requester = new samples::reqrep::Requester(*context, "inproc://reqrep", "hello");
        QSignalSpy spyRequesterRequestSent(requester, SIGNAL(requestSent(const QList<QByteArray>&)));
        QSignalSpy spyRequesterReplyReceived(requester, SIGNAL(replyReceived(const QList<QByteArray>&)));
        QSignalSpy spyRequesterFailure(requester, SIGNAL(failure(const QString&)));
        QSignalSpy spyRequesterFinished(requester, SIGNAL(finished()));
        // Create requester execution thread.
        QThread* requesterThread = makeExecutionThread(*requester);
        QSignalSpy spyRequesterThreadFinished(requesterThread, SIGNAL(finished()));

        //
        // START TEST
        //

        context->start();

        replierThread->start();
        QTest::qWait(500);
        requesterThread->start();

        QTimer::singleShot(6000, replier, SLOT(stop()));
        QTimer::singleShot(6000, requester, SLOT(stop()));

        QTest::qWait(8000);

        //
        // CHECK POSTCONDITIONS
        //

        qDebug() << "Requester requests sent:" << spyRequesterRequestSent.size();
        qDebug() << "Replier requests received:" << spyReplierRequestReceived.size();

        QCOMPARE(spyReplierFailure.size(), 0);
        QCOMPARE(spyRequesterFailure.size(), 0);

        QVERIFY2(spyRequesterRequestSent.size() > 3, "Requester didn't send any/enough requests.");
        QCOMPARE(spyRequesterReplyReceived.size(), spyRequesterRequestSent.size());

        QCOMPARE(spyReplierRequestReceived.size(), spyRequesterRequestSent.size());
        QCOMPARE(spyReplierReplySent.size(), spyReplierRequestReceived.size());

        QCOMPARE(spyReplierFinished.size(), 1);
        QCOMPARE(spyRequesterFinished.size(), 1);

        QCOMPARE(spyReplierThreadFinished.size(), 1);
        QCOMPARE(spyRequesterThreadFinished.size(), 1);
    }
    catch (std::exception& ex)
    {
        QFAIL(ex.what());
    }
}

void NzmqtTest::testPushPull()
{
    try {
        QScopedPointer<ZMQContext> context(nzmqt::createDefaultContext());

        // Create ventilator.
        samples::pushpull::Ventilator* ventilator = new samples::pushpull::Ventilator(*context, "tcp://127.0.0.1:5557", "tcp://127.0.0.1:5558", 200);
        QSignalSpy spyVentilatorBatchStarted(ventilator, SIGNAL(batchStarted(int)));
        QSignalSpy spyVentilatorWorkItemSent(ventilator, SIGNAL(workItemSent(quint32)));
        QSignalSpy spyVentilatorFailure(ventilator, SIGNAL(failure(const QString&)));
        QSignalSpy spyVentilatorFinished(ventilator, SIGNAL(finished()));
        // Create ventilator execution thread.
        QThread* ventilatorThread = makeExecutionThread(*ventilator);
        QSignalSpy spyVentilatorThreadFinished(ventilatorThread, SIGNAL(finished()));

        // Create worker.
        samples::pushpull::Worker* worker = new samples::pushpull::Worker(*context, "tcp://127.0.0.1:5557", "tcp://127.0.0.1:5558");
        QSignalSpy spyWorkerWorkItemReceived(worker, SIGNAL(workItemReceived(quint32)));
        QSignalSpy spyWorkerWorkItemResultSent(worker, SIGNAL(workItemResultSent()));
        QSignalSpy spyWorkerFailure(worker, SIGNAL(failure(const QString&)));
        QSignalSpy spyWorkerFinished(worker, SIGNAL(finished()));
        // Create worker execution thread.
        QThread* workerThread = makeExecutionThread(*worker);
        QSignalSpy spyWorkerThreadFinished(workerThread, SIGNAL(finished()));

        // Create sink.
        samples::pushpull::Sink* sink = new samples::pushpull::Sink(*context, "tcp://127.0.0.1:5558");
        QSignalSpy spySinkBatchStarted(sink, SIGNAL(batchStarted(int)));
        QSignalSpy spySinkWorkItemResultReceived(sink, SIGNAL(workItemResultReceived()));
        QSignalSpy spySinkBatchCompleted(sink, SIGNAL(batchCompleted()));
        QSignalSpy spySinkFailure(sink, SIGNAL(failure(const QString&)));
        QSignalSpy spySinkFinished(sink, SIGNAL(finished()));
        // Create sink execution thread.
        QThread* sinkThread = makeExecutionThread(*sink);
        QSignalSpy spySinkThreadFinished(sinkThread, SIGNAL(finished()));

        //
        // START TEST
        //

        const int numberOfWorkItems = ventilator->numberOfWorkItems();
        const int maxTotalExpectedCost = numberOfWorkItems*ventilator->maxWorkLoad();
        QTimer::singleShot(maxTotalExpectedCost + 2000, ventilator, SLOT(stop()));
        QTimer::singleShot(maxTotalExpectedCost + 2000, worker, SLOT(stop()));
        QTimer::singleShot(maxTotalExpectedCost + 2000, sink, SLOT(stop()));

        context->start();

        sinkThread->start();
        QTest::qWait(500);
        ventilatorThread->start();
        QTest::qWait(500);
        workerThread->start();

        QTest::qWait(maxTotalExpectedCost + 2500);

        //
        // CHECK POSTCONDITIONS
        //

        QCOMPARE(spyVentilatorFailure.size(), 0);
        QCOMPARE(spyWorkerFailure.size(), 0);
        QCOMPARE(spySinkFailure.size(), 0);

        QCOMPARE(spyVentilatorBatchStarted.size(), 1);
        QCOMPARE(spyVentilatorWorkItemSent.size(), numberOfWorkItems);
        QCOMPARE(spyVentilatorFinished.size(), 1);
        QCOMPARE(spyVentilatorThreadFinished.size(), 1);

        QCOMPARE(spyWorkerWorkItemReceived.size(), numberOfWorkItems);
        QCOMPARE(spyWorkerWorkItemResultSent.size(), numberOfWorkItems);
        QCOMPARE(spyWorkerFinished.size(), 1);
        QCOMPARE(spyWorkerThreadFinished.size(), 1);

        QCOMPARE(spySinkBatchStarted.size(), 1);
        QCOMPARE(spySinkWorkItemResultReceived.size(), numberOfWorkItems);
        QCOMPARE(spySinkBatchCompleted.size(), 1);
        QCOMPARE(spySinkFinished.size(), 1);
        QCOMPARE(spySinkThreadFinished.size(), 1);
    }
    catch (std::exception& ex)
    {
        QFAIL(ex.what());
    }
}

}

QTEST_MAIN(nzmqt::NzmqtTest)

#include "nzmqt_test.moc"

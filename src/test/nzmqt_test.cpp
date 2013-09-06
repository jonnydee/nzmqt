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

#include "pubsub/PubSubClient.h"
#include "pubsub/PubSubServer.h"
#include "reqrep/ReqRepClient.h"
#include "reqrep/ReqRepServer.h"

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
        samples::PubSubServer* publisher = new samples::PubSubServer(*context, "inproc://pubsub", "ping");
        QSignalSpy spyPublisherPingSent(publisher, SIGNAL(pingSent(const QList<QByteArray>&)));
        QSignalSpy spyPublisherFailure(publisher, SIGNAL(failure(const QString&)));
        QSignalSpy spyPublisherFinished(publisher, SIGNAL(finished()));
        // Create publisher execution thread.
        QThread* publisherThread = makeExecutionThread(*publisher);
        QSignalSpy spyPublisherThreadFinished(publisherThread, SIGNAL(finished()));

        // Create subscriber.
        samples::PubSubClient* subscriber = new samples::PubSubClient(*context, "inproc://pubsub", "ping");
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

        // Create server.
        samples::ReqRepServer* server = new samples::ReqRepServer(*context, "inproc://reqrep", "world");
        QSignalSpy spyServerRequestReceived(server, SIGNAL(requestReceived(const QList<QByteArray>&)));
        QSignalSpy spyServerFailure(server, SIGNAL(failure(const QString&)));
        QSignalSpy spyServerFinished(server, SIGNAL(finished()));
        // Create server execution thread.
        QThread* serverThread = makeExecutionThread(*server);
        QSignalSpy spyServerThreadFinished(serverThread, SIGNAL(finished()));

        // Create client.
        samples::ReqRepClient* client = new samples::ReqRepClient(*context, "inproc://reqrep", "hello");
        QSignalSpy spyClientRequestSent(client, SIGNAL(requestSent(const QList<QByteArray>&)));
        QSignalSpy spyClientFailure(client, SIGNAL(failure(const QString&)));
        QSignalSpy spyClientFinished(client, SIGNAL(finished()));
        // Create client execution thread.
        QThread* clientThread = makeExecutionThread(*client);
        QSignalSpy spyClientThreadFinished(clientThread, SIGNAL(finished()));

        //
        // START TEST
        //

        context->start();

        serverThread->start();
        QTest::qWait(500);
        clientThread->start();

        QTimer::singleShot(6000, server, SLOT(stop()));
        QTimer::singleShot(6000, client, SLOT(stop()));

        QTest::qWait(8000);

        //
        // CHECK POSTCONDITIONS
        //

        qDebug() << "Client requests sent:" << spyClientRequestSent.size();
        qDebug() << "Server requests received:" << spyServerRequestReceived.size();

        QCOMPARE(spyServerFailure.size(), 0);
        QCOMPARE(spyClientFailure.size(), 0);

        QVERIFY2(spyServerRequestReceived.size() > 3, "Server didn't send any/enough pings.");
        QVERIFY2(spyClientRequestSent.size() > 3, "Client didn't receive any/enough pings.");

        QCOMPARE(spyServerRequestReceived.size(), spyClientRequestSent.size());

        QCOMPARE(spyServerFinished.size(), 1);
        QCOMPARE(spyClientFinished.size(), 1);

        QCOMPARE(spyServerThreadFinished.size(), 1);
        QCOMPARE(spyClientThreadFinished.size(), 1);
    }
    catch (std::exception& ex)
    {
        QFAIL(ex.what());
    }
}

}

QTEST_MAIN(nzmqt::NzmqtTest)

#include "nzmqt_test.moc"

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

private slots:
    void testContextInstance();
    void testPubSub();

protected slots:
    void serverSentPing(const QList<QByteArray>& message);
    void serverIndicatedFailure(const QString& what);

    void clientReceivedPing(const QList<QByteArray>& message);
    void clientIndicatedFailure(const QString& what);

private:
    nzmqt::ZMQContext* context_;

    QList< QList<QByteArray> > serverSentPings_;
    QStringList serverIndicatedFailures_;

    QList< QList<QByteArray> > clientReceivedPings_;
    QStringList clientIndicatedFailures_;
};

NzmqtTest::NzmqtTest()
    : context_(nzmqt::createDefaultContext(this))
{
    qRegisterMetaType< QList<QByteArray> >();
}

void NzmqtTest::serverSentPing(const QList<QByteArray>& message)
{
    serverSentPings_ << message;
    qDebug() << Q_FUNC_INFO << message;
}

void NzmqtTest::serverIndicatedFailure(const QString& what)
{
    serverIndicatedFailures_ << what;
    qDebug() << Q_FUNC_INFO << what;
}

void NzmqtTest::clientReceivedPing(const QList<QByteArray>& message)
{
    clientReceivedPings_ << message;
    qDebug() << Q_FUNC_INFO << message;
}

void NzmqtTest::clientIndicatedFailure(const QString& what)
{
    clientIndicatedFailures_ << what;
    qDebug() << Q_FUNC_INFO << what;
}

void NzmqtTest::testContextInstance()
{
    QVERIFY2(context_, "No context available!");
}

void NzmqtTest::testPubSub()
{
    try {
        QThread* serverThread = new QThread;
        samples::PubSubServer* server = new samples::PubSubServer(*context_, "inproc://pubsub", "ping");
        server->moveToThread(serverThread);
        QVERIFY(connect(serverThread, SIGNAL(started()), server, SLOT(start())));
        QVERIFY(connect(server, SIGNAL(finished()), serverThread, SLOT(quit())));
        QVERIFY(connect(server, SIGNAL(finished()), server, SLOT(deleteLater())));
        QVERIFY(connect(serverThread, SIGNAL(finished()), serverThread, SLOT(deleteLater())));

        QThread* clientThread = new QThread;
        samples::PubSubClient* client = new samples::PubSubClient(*context_, "inproc://pubsub", "ping");
        client->moveToThread(clientThread);
        QVERIFY(connect(clientThread, SIGNAL(started()), client, SLOT(start())));
        QVERIFY(connect(client, SIGNAL(finished()), clientThread, SLOT(quit())));
        QVERIFY(connect(client, SIGNAL(finished()), client, SLOT(deleteLater())));
        QVERIFY(connect(clientThread, SIGNAL(finished()), clientThread, SLOT(deleteLater())));

        // Connect to server signals for checking test condition.
        QVERIFY(connect(server, SIGNAL(pingSent(const QList<QByteArray>&)), SLOT(serverSentPing(const QList<QByteArray>&))));
        QVERIFY(connect(server, SIGNAL(failure(const QString&)), SLOT(serverIndicatedFailure(const QString&))));
        // Connect to client signals for checking test condition.
        QVERIFY(connect(client, SIGNAL(pingReceived(const QList<QByteArray>&)), SLOT(clientReceivedPing(QList<QByteArray>))));
        QVERIFY(connect(client, SIGNAL(failure(const QString&)), SLOT(clientIndicatedFailure(const QString&))));

        serverThread->start();
        QTest::qWait(500);
        clientThread->start();

        QTimer::singleShot(6000, server, SLOT(stop()));
        QTimer::singleShot(6000, client, SLOT(stop()));
    }
    catch (std::exception& ex)
    {
        QFAIL(ex.what());
    }

    QTest::qWait(8000);

    qDebug() << "Client pings received:" << clientReceivedPings_;
    if (!clientIndicatedFailures_.isEmpty())
        qDebug() << "Client indicated failures:" << clientIndicatedFailures_;

    qDebug() << "Server pings sent:" << serverSentPings_;
    if (!serverIndicatedFailures_.isEmpty())
        qDebug() << "Server indicated failures:" << serverIndicatedFailures_;

    QVERIFY2(serverIndicatedFailures_.isEmpty(), "Server indicated failures.");
    QVERIFY2(clientIndicatedFailures_.isEmpty(), "Client indicated failures.");

    QVERIFY2(serverSentPings_.size() > 3, "Server didn't send any/enough pings.");
    QVERIFY2(clientReceivedPings_.size() > 3, "Client didn't receive any/enough pings.");

    QVERIFY2(qAbs(serverSentPings_.size() - clientReceivedPings_.size()) < 3, "Server and client communication flawed.");
}

}

QTEST_MAIN(nzmqt::NzmqtTest)

#include "nzmqt_test.moc"

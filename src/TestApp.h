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

#ifndef TESTAPP_H
#define TESTAPP_H

#include <stdexcept>
#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QTextStream>

#include "nzmqt/nzmqt.hpp"

#include "pubsub/PubSubClient.h"
#include "pubsub/PubSubServer.h"
#include "reqrep/ReqRepClient.h"
#include "reqrep/ReqRepServer.h"


class TestApp : public QCoreApplication
{
    Q_OBJECT

    typedef QCoreApplication super;

public:
    explicit TestApp(int& argc, char** argv)
        : super(argc, argv)
    {
    }

    bool run()
    {
        QTextStream cout(stdout);
        try
        {
            QStringList args = arguments();

            if (args.size() < 4)
            {
                printUsage(cout);
                return false;
            }

            QString command = args[1];
            QString address = args[2];

            if ("pubsub-server" == command)
            {
                QString topic = args[3];
                PubSubServer* server = new PubSubServer(address, topic, this);
                server->run();
                return true;
            }
            else if ("pubsub-client" == command)
            {
                QString topic = args[3];
                PubSubClient* client = new PubSubClient(address, topic, this);
                client->run();
                return true;
            }
            else if ("reqrep-server" == command)
            {
                QString responseMsg = args[3];
                ReqRepServer* server = new ReqRepServer(address, responseMsg, this);
                server->run();
                return true;
            }
            else if ("reqrep-client" == command)
            {
                QString requestMsg = args[3];
                ReqRepClient* client = new ReqRepClient(address, requestMsg, this);
                client->run();
                return true;
            }
            else
            {
                printUsage(cout);
                return false;
            }
        }
        catch (std::exception& ex)
        {
            qWarning() << ex.what();
            return false;
        }
    }

protected:
    bool notify(QObject *obj, QEvent *event)
    {
        try
        {
            return super::notify(obj, event);
        }
        catch (std::exception& ex)
        {
            qWarning() << ex.what();
            return false;
        }
    }

    void printUsage(QTextStream& out)
    {
        QString executable = arguments().at(0);
        out << QString(
"\n\
USAGE: %1 <pubsub-server|pubsub-client> <address> <topic>\n\
USAGE: %1 <reqrep-server|reqrep-client> <address> <reply-msg|request-msg>\n\
\n\
Publish-Subscribe Sample:\n\
* Server: %1 pubsub-server tcp://127.0.0.1:1234 ping\n\
* Client: %1 pubsub-client tcp://127.0.0.1:1234 ping\n\
\n\
Request-Reply Sample:\n\
* Server: %1 reqrep-server tcp://127.0.0.1:1234 World\n\
* Client: %1 reqrep-client tcp://127.0.0.1:1234 Hello\n\
\n").arg(executable);
    }
};

#endif // TESTAPP_H

/*
  DMX Total Control
  singleinstance.cpp

  Copyright (c) DMX Total Control contributors

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QElapsedTimer>
#include <QLockFile>
#include <QFileInfo>
#include <QThread>
#include <QDebug>
#include <QDir>

#include "singleinstance.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// (what a second launch says first: a connection that isn't one says nothing)
#define KMessageOpen QByteArrayLiteral("DTC-OPEN\t")
// what the one running answers when it has it
#define KAnswerTaken QByteArrayLiteral("OK\n")
// how long the one running may take to read it, once reached (busy opening a show)
#define KReadTimeout 15000

SingleInstance::SingleInstance(const QString &key, const QString &lockFile, QObject *parent)
    : QObject(parent)
    , m_key(key)
    , m_lock(nullptr)
    , m_server(nullptr)
{
    QDir().mkpath(QFileInfo(lockFile).absolutePath());
    m_lock = new QLockFile(lockFile);
    // held while the program runs, for hours: only the lock of a program
    // that is gone is stale, not an old one
    m_lock->setStaleLockTime(0);
}

SingleInstance::~SingleInstance()
{
    delete m_server;
    delete m_lock;
}

QString SingleInstance::defaultKey()
{
    QString user = qEnvironmentVariable("USERNAME");
    if (user.isEmpty())
        user = qEnvironmentVariable("USER");
    return QStringLiteral("DMXTotalControl-") + user;
}

bool SingleInstance::start(int timeout)
{
    // (a lock left by a program that crashed is taken over)
    if (m_lock->tryLock(timeout) == false)
        return false;

    // (a server left by a program that is gone, where the system keeps them)
    QLocalServer::removeServer(m_key);
    m_server = new QLocalServer(this);
    // only the programs of this user reach it
    m_server->setSocketOptions(QLocalServer::UserAccessOption);
    connect(m_server, &QLocalServer::newConnection, this, &SingleInstance::slotNewConnection);
    // (it runs anyway: a second launch just can't reach it)
    if (m_server->listen(m_key) == false)
        qWarning() << "Single instance: unable to listen on" << m_key << m_server->errorString();
    // a program that quits takes no more shows: they would open nowhere
    if (QCoreApplication::instance() != nullptr)
        connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &SingleInstance::stopListening);
    return true;
}

void SingleInstance::stopListening()
{
    if (m_server != nullptr)
        m_server->close();
}

bool SingleInstance::sendToRunning(const QString &key, const QString &file, int timeout)
{
    QElapsedTimer clock;
    clock.start();
    auto left = [&clock](int limit) { return int(qMax<qint64>(1, limit - clock.elapsed())); };

    // (one that starts at the same time listens a moment later)
    QLocalSocket socket;
    for (;;)
    {
        socket.connectToServer(key);
        if (socket.waitForConnected(qMin(left(timeout), 500)))
            break;
        socket.abort();
        if (clock.elapsed() >= timeout)
            return false;
        QThread::msleep(quint64(qMin(left(timeout), 100)));
    }

#ifdef Q_OS_WIN
    // the running program may come to the front: a launch from the Explorer
    // has the right to give it
    AllowSetForegroundWindow(ASFW_ANY);
#endif

    // one line: the path of the show, in UTF-8. It is taken when the one
    // running reads it (a pipe without a buffer), and it answers then. A
    // pipe reached isn't enough: a program that quits or crashed may still
    // have it, and never read
    socket.write(KMessageOpen + file.toUtf8() + '\n');
    const int limit = timeout + KReadTimeout;
    while (socket.bytesToWrite() > 0 && socket.state() == QLocalSocket::ConnectedState && clock.elapsed() < limit)
        socket.waitForBytesWritten(left(limit));
    QByteArray answer;
    while (socket.bytesToWrite() == 0 && clock.elapsed() < limit)
    {
        answer += socket.readAll();
        if (answer.contains('\n') || socket.state() != QLocalSocket::ConnectedState)
            break;
        socket.waitForReadyRead(left(limit));
    }
    answer += socket.readAll();
    if (answer.startsWith(KAnswerTaken) == false)
    {
        qWarning() << "Single instance: the program running didn't take" << file;
        return false;
    }
    return true;
}

void SingleInstance::slotNewConnection()
{
    while (QLocalSocket *socket = m_server->nextPendingConnection())
    {
        // what came, until the end of the line (or of the connection)
        auto read = [this, socket]() {
            if (socket->property("told").toBool())
                return;
            QByteArray data = socket->property("data").toByteArray() + socket->readAll();
            const qsizetype end = data.indexOf('\n');
            if (end < 0 && socket->state() == QLocalSocket::ConnectedState)
            {
                socket->setProperty("data", data);
                return;
            }
            socket->setProperty("told", true);
            if (end >= 0)
                data.truncate(end);
            if (data.startsWith(KMessageOpen))
            {
                // (the answer first: the launch goes while the show opens)
                if (end >= 0 && socket->state() == QLocalSocket::ConnectedState)
                    socket->write(KAnswerTaken);
                emit activationRequested(QString::fromUtf8(data.mid(KMessageOpen.size())));
            }
            // (once the answer is written)
            socket->disconnectFromServer();
        };
        connect(socket, &QLocalSocket::readyRead, this, read);
        connect(socket, &QLocalSocket::disconnected, this, read);
        connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
        read();
    }
}

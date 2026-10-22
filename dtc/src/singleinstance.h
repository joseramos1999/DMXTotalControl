/*
  DMX Total Control
  singleinstance.h

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

#ifndef SINGLEINSTANCE_H
#define SINGLEINSTANCE_H

#include <QObject>
#include <QString>

class QLocalServer;
class QLockFile;

/** One program running per user. A second launch (a show opened from the
 *  Explorer while the program runs) gives the running one the show it was
 *  asked to open, or nothing, and quits: the running one comes to the
 *  front and opens it (see main). Two engines would send to the same
 *  universes and controllers, and share the settings and the recovery */
class SingleInstance : public QObject
{
    Q_OBJECT

public:
    /** $key names the running program (see defaultKey()); $lockFile is the
     *  file that says it runs */
    SingleInstance(const QString &key, const QString &lockFile, QObject *parent = nullptr);
    ~SingleInstance();

    /** "DMXTotalControl-<user name>" */
    static QString defaultKey();

    /** Become the running program: false when another one is, after waiting
     *  up to $timeout ms for it to go. Two launches at once: one of them
     *  gets the lock, the other gives it its show */
    bool start(int timeout = 0);

    /** Take no more shows (the program quits: a launch meanwhile waits for
     *  it to go). It still runs: the lock stays until it is deleted */
    void stopListening();

    /** Give $file (an absolute path; empty: just the window) to the program
     *  running with $key, and let it come to the front. @return true once
     *  it said it has it; false when none is reached within $timeout ms, or
     *  the one reached doesn't take it (it quits, or it crashed and the
     *  system still holds it). One that is busy (opening a show) gets a
     *  while longer to read it */
    static bool sendToRunning(const QString &key, const QString &file, int timeout = 3000);

signals:
    /** A second launch asked for the window, and to open $file (empty: none) */
    void activationRequested(const QString &file);

private slots:
    void slotNewConnection();

private:
    QString m_key;
    QLockFile *m_lock;
    QLocalServer *m_server;
};

#endif

/*
  DMX Total Control
  remoteserver.h

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

#ifndef REMOTESERVER_H
#define REMOTESERVER_H

#include <QObject>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QPair>
#include <QSet>

class QWebSocketServer;
class QWebSocket;
class QTcpServer;
class QTcpSocket;
class QTimer;
class DtcCore;

/**
 * Remote control from a phone or a tablet on the local network: a small
 * web server that gives a page with the SHOW screen, and a
 * WebSocket that sends what the controls show and takes what the fingers do.
 * Nothing to install: the browser of the device opens http://<this computer>:<port>.
 *
 * Messages (JSON). To the page: {type: "auth"|"layout"|"state"}. From it:
 * {cmd: "hello", pin}, then press/release/level/encoder (page, section, cell),
 * faderFlash (page, section, cell, down: the Flash of a SHOW fader, full
 * while it is down, back to its level after),
 * colour/xy/releaseSection (page, section), scene/groupMaster (zone),
 * global (action: blackout, stopAll, tap, beatgo), grandMaster (value),
 * playback (action: go, back, pauseBack, goto, flash, release, level, select,
 * pageNext, pagePrevious, page; pbPage: the playback page the phone shows,
 * slot, value for level, down for flash, cue for goto; page: pbPage).
 *
 * The layout gets a page "Playbacks" of its own (a "playbacks" section on
 * the current playback page) when a playback holds something. The state
 * carries the playbacks the page can show: playbacks {"<page>/<slot>": {name,
 * type, color, state, level, flashing, cueNumber, cueName, nextNumber,
 * nextName, progress}} (only the ones that hold something), playbackPage,
 * playbackPages and playbackPageNames.
 */
class RemoteServer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY settingsChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY settingsChanged)
    /** Empty: no PIN asked */
    Q_PROPERTY(QString pin READ pin WRITE setPin NOTIFY settingsChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    /** Where to open it: http://<address>:<port>/ for each network of this computer */
    Q_PROPERTY(QStringList urls READ urls NOTIFY runningChanged)
    Q_PROPERTY(int clients READ clients NOTIFY clientsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY runningChanged)

public:
    RemoteServer(DtcCore *core, QObject *parent = nullptr);
    ~RemoteServer();

    bool enabled() const;
    void setEnabled(bool enable);
    int port() const;
    void setPort(int port);
    QString pin() const;
    void setPin(const QString &pin);

    bool running() const;
    QStringList urls() const;
    int clients() const;
    QString lastError() const;

    /** Start or stop as the settings say */
    void apply();
    /** Start on a port without touching the settings (developer option --remote) */
    void startOn(int port);

    /** The SHOW for the page: titles, labels and pictures resolved here */
    QVariantMap layoutMessage() const;
    /** What the controls show now */
    QVariantMap stateMessage() const;
    /** Run a message of the page (tests call it directly) */
    void handleCommand(const QVariantMap &message);

signals:
    void settingsChanged();
    void runningChanged();
    void clientsChanged();

private slots:
    void slotNewConnection();
    void slotHttpReadyRead();
    void slotWebSocket();
    void slotTextMessage(const QString &text);
    void slotSocketGone();
    void slotLayoutChanged();
    /** The groups are in the layout (their names, colours and scenes): it
     *  follows them */
    void slotZonesChanged();
    void slotZoneEdited();
    void slotTick();
    /** The flashes the phones hold follow their playback pages */
    void slotPlaybackPagesRenumbered(const QList<int> &newIndexes, const QHash<int, quint32> &removedSlots);

private:
    void stop();
    void serveHttp(QTcpSocket *socket, const QByteArray &request);
    void send(QWebSocket *socket, const QVariantMap &message);
    void broadcast(const QVariantMap &message);
    QVariantMap itemAt(int page, const QString &section, int cell) const;
    QVariantMap describeNode(const QVariantMap &node, int page) const;
    QString itemLabel(const QVariantMap &item, const QString &sectionType) const;
    QString imagePath(int page, const QString &section, int cell) const;
    /** The playbacks of the state: the current playback page and the pages
     *  the "playbacks" sections show */
    QVariantMap playbacksState() const;
    /** A flash a phone holds (page, slot): let go of it when the phone goes.
     *  False for an "up" of a flash the phone doesn't hold (already let go
     *  of: it would release a hold of somebody else). The phone lets go
     *  with the page it pressed on: $message gets the page that page is at
     *  now (playback pages moved meanwhile) */
    bool trackFlash(QWebSocket *socket, QVariantMap &message);
    /** Let go of every flash a phone holds */
    void releaseFlashes(QWebSocket *socket);
    /** A SHOW button (press, release) or the Flash of a SHOW fader
     *  (faderFlash) a phone holds down. Each up lets go of what its down
     *  pressed, even when the SHOW changed meanwhile (an up of something the
     *  phone doesn't hold does nothing), and what a phone holds is let go of
     *  when it goes. @return false for the other messages */
    bool trackPress(QWebSocket *socket, const QVariantMap &message);
    /** Let go of every SHOW button and fader flash a phone holds */
    void releasePresses(QWebSocket *socket);

private:
    DtcCore *m_core;
    QTcpServer *m_server;
    QWebSocketServer *m_ws;
    QList<QWebSocket *> m_sockets;
    QSet<QWebSocket *> m_authorized;
    QTimer *m_tick;
    QByteArray m_lastState;
    QString m_lastError;
    bool m_layoutDirty;
    /** The groups as the layout last had them */
    QString m_groupsLaidOut;
    /** The playback flashes each phone holds down: (page, slot) as the phone
     *  sent them -> how many times (each down holds it once more in the
     *  playbacks), the page they are on now, and whether that page was
     *  deleted (the playbacks let go of them then) */
    struct FlashHold
    {
        int count = 0;
        int page = -1;
        bool gone = false;
    };
    QHash<QWebSocket *, QHash<QPair<int, int>, FlashHold>> m_flashes;
    /** The SHOW buttons and fader flashes each phone holds down, by
     *  "kind/page/section/cell" as the phone sent them: how many times, the
     *  item pressed, and the level a fader goes back to */
    struct PressHold
    {
        int count = 0;
        QVariantMap item;
        bool fader = false;
        qreal level = 0.0;
    };
    QHash<QWebSocket *, QHash<QString, PressHold>> m_presses;
};

#endif

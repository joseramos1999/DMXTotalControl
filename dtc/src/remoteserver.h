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

#include <QNetworkInterface>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QObject>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QPair>
#include <QSet>

class QWebSocketServer;
class MdnsResponder;
class QWebSocket;
class QTcpServer;
class QTcpSocket;
class QTimer;
class DtcCore;

/**
 * Remote control from a phone or a tablet on the local network: a small
 * web server that gives a page with the SHOW screen, and a
 * WebSocket that sends what the controls show and takes what the fingers do.
 * Nothing to install: the browser of the device opens http://<this computer>:<port>,
 * or http://<name>.local:<port>/ by the name of the computer on the network
 * (MdnsResponder), which stays when its address changes.
 *
 * A page opened by an address (the QR code) goes to the name when it reaches
 * this same computer by it: the layout carries instance (this program, a new
 * one each time it starts), host (the name, "" when there is none) and port,
 * and GET /whoami answers {id: the instance, host} to any page (CORS: the
 * page of the address asks the name, another origin).
 *
 * Messages (JSON). To the page: {type: "auth"|"layout"|"state"|"clock"}. From it:
 * {cmd: "hello", pin}, then press/release/level/encoder (page, section, cell),
 * faderFlash (page, section, cell, down: the Flash of a SHOW fader, full
 * while it is down, back to its level after),
 * colour/xy/releaseSection (page, section), timer (page, section, action:
 * toggle, reset; a timer section only), scene/groupMaster (zone),
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
 *
 * The timers count on the clock of the computer (ShowSurface::clock, ms):
 * the layout carries the clock as it is (clock), the state the timer
 * sections of every page, timers {"<section>": {running: true, origin} or
 * {running: false, elapsed}} (the same while they run: not sent again each
 * tick), and while one runs a {type: "clock", now} every 10 s keeps the
 * phones on time (the clock of a phone may stop while it sleeps).
 *
 * The DMX levels go only to the phones that show them: a phone says which
 * universes its page shows with {cmd: "monitor", universes: [int]} (at most
 * 4; [] while the page is hidden), and gets {type: "levels", universes:
 * {"<universe>": base64 of its 512 bytes}} with the universes that changed
 * since its last one (all of them after a new "monitor").
 *
 * The live mixer: a "mixer" section of the layout lists the mixers, mixers
 * [{id, name, fixtures}] (all the fixtures, -1, then each fixture group), and
 * while a page has one the state carries their settings, mixers {"<id>":
 * {dimmer, colour, hue, saturation, strobe, strobeHz, full, blackout, flash,
 * fullExclusive, flashExclusive}}. The phone sends {cmd: "mixer", mixer,
 * key, value} for dimmer, hue, saturation and strobeHz, {cmd: "mixer",
 * mixer, key, toggle: true} for full, blackout, strobe and fullExclusive
 * (on when it is off and off when it is on: two phones don't fight), {cmd:
 * "mixer", mixer, key: "resetColour"}, and {cmd: "mixerFlash", mixer, key:
 * flash or flashExclusive, down} while a finger is on a flash.
 */
class RemoteServer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY settingsChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY settingsChanged)
    /** Empty: no PIN asked */
    Q_PROPERTY(QString pin READ pin WRITE setPin NOTIFY settingsChanged)
    /** The name of this computer on the network: the phones open
     *  http://<name>.local:<port>/ (lower case, digits and '-'). Empty: no
     *  name */
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY settingsChanged)
    /** The name the phones open now, "<name>.local" ("<name>-2.local" when
     *  another computer has it). Empty while it is off, before the network
     *  said nobody else has it, or with no name */
    Q_PROPERTY(QString hostName READ hostName NOTIFY addressesChanged)
    /** Why the phones can't find it by name (the port of the names is
     *  taken). Empty when they can */
    Q_PROPERTY(QString discoveryError READ discoveryError NOTIFY addressesChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    /** Where to open it: http://<address>:<port>/ for each network of this
     *  computer, as addresses has them (http://127.0.0.1:<port>/ when it is
     *  on no network) */
    Q_PROPERTY(QStringList urls READ urls NOTIFY addressesChanged)
    /** Where to open it, best first: [{url, label: the network as the
     *  system calls it ("Wi-Fi", "Ethernet 2"), kind: "wifi", "ethernet" or
     *  "other"}], with http://<hostName>:<port>/ first when there is a name
     *  (label "By name", kind "name"). Empty while it is off or on no
     *  network. It follows the networks the computer joins and leaves */
    Q_PROPERTY(QVariantList addresses READ addresses NOTIFY addressesChanged)
    /** False when it runs on a computer that is on no network (only its own
     *  127.0.0.1: no phone can reach it) */
    Q_PROPERTY(bool onNetwork READ onNetwork NOTIFY addressesChanged)
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
    QString name() const;
    /** Kept as a name of the network can be: lower case, spaces and '_' as
     *  '-', the other characters out, no '-' at the ends, 63 at most ("DMX
     *  Total_Control!" is dmx-total-control) */
    void setName(const QString &name);
    QString hostName() const;
    QString discoveryError() const;
    /** What answers for the name on the network (the tests start it on the
     *  loopback) */
    MdnsResponder *discovery() const;

    bool running() const;
    QStringList urls() const;
    QVariantList addresses() const;
    bool onNetwork() const;
    int clients() const;
    QString lastError() const;

    /** The QR code of a text (an address), for Options to draw: '1' dark,
     *  '0' light, row by row, without the quiet zone. Empty when the text
     *  is too long for one */
    Q_INVOKABLE QStringList qrCode(const QString &text) const;
    /** The address a QR code carries: $url, and the PIN after a '#' when
     *  there is one ("#pin=1234"), so that whoever scans it gets in without
     *  typing it. The fragment never reaches this server: the page reads it
     *  and takes it out of the address bar of the phone */
    Q_INVOKABLE QString shareUrl(const QString &url) const;

    /** An address of a network of this computer: the name of the network
     *  as the system calls it, its kind and the address */
    struct NetworkAddress
    {
        QString name;
        QNetworkInterface::InterfaceType type = QNetworkInterface::Unknown;
        QHostAddress ip;
    };
    /** The addresses phones can open, as addresses has them: IPv4 that is
     *  not loopback nor link-local, the Wi-Fi first, then the Ethernet, then
     *  the rest (the virtual adapters and the VPNs too, by their names: the
     *  phones aren't there), each kind by name. The $host name before them
     *  all, when there is one and the computer is on a network */
    static QVariantList describeAddresses(const QList<NetworkAddress> &found, int port,
                                          const QString &host = QString());

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
    void addressesChanged();
    void clientsChanged();

private slots:
    void slotNewConnection();
    void slotHttpReadyRead();
    void slotWebSocket();
    void slotTextMessage(const QString &text);
    void slotSocketGone();
    void slotLayoutChanged();
    /** Another show: what the phones hold is let go of, not given back */
    void slotShowCleared();
    /** The groups are in the layout (their names, colours and scenes): it
     *  follows them */
    void slotZonesChanged();
    void slotZoneEdited();
    /** The mixers are in the layout (their names and fixtures): it follows
     *  them, not their settings (the state carries those) */
    void slotMixersChanged();
    void slotTick();
    /** A network joined or left, a new address: the addresses follow, and
     *  the name is told there */
    void slotNetworkCheck();
    /** The name is the computer's (or another one's, or none): the
     *  addresses follow */
    void slotHostChanged();
    /** The flashes the phones hold follow their playback pages */
    void slotPlaybackPagesRenumbered(const QList<int> &newIndexes, const QHash<int, quint32> &removedSlots);

private:
    void stop();
    /** Answer for the name on the network (with the server running), or
     *  stop when there is none */
    void startDiscovery();
    /** The layout when it changed, the clock while a timer runs, and the
     *  state when it changed: to every phone */
    void sendState();
    /** The DMX levels that changed: to each phone that shows them */
    void sendLevels();
    /** The universes a phone shows ("monitor"): the ones there are, at most
     *  4; what it was sent is sent again */
    void watchLevels(QWebSocket *socket, const QVariantList &universes);
    /** Read the addresses of the networks again (none while it is off):
     *  @return true when they changed */
    bool readAddresses();
    void serveHttp(QTcpSocket *socket, const QByteArray &request);
    /** The answer of GET /whoami: {id: the instance, host: the name} */
    QByteArray whoAmI() const;
    void send(QWebSocket *socket, const QVariantMap &message);
    void broadcast(const QVariantMap &message);
    QVariantMap itemAt(int page, const QString &section, int cell) const;
    QVariantMap describeNode(const QVariantMap &node, int page) const;
    QString itemLabel(const QVariantMap &item, const QString &sectionType) const;
    QString imagePath(int page, const QString &section, int cell) const;
    /** The playbacks of the state: the current playback page and the pages
     *  the "playbacks" sections show */
    QVariantMap playbacksState() const;
    /** The step of each scene with steps that plays: ID -> step (the rhythm bars) */
    QVariantMap stepsState() const;
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
    /** A flash of the mixer (mixerFlash) a phone holds down: on with the
     *  first down, off with the last up of every phone, and let go of when
     *  the phone goes (an up of a flash the phone doesn't hold does
     *  nothing). @return false for the other messages */
    bool trackMixerFlash(QWebSocket *socket, const QVariantMap &message);
    /** Let go of every flash of the mixer a phone holds */
    void releaseMixerFlashes(QWebSocket *socket);
    /** The flash of a mixer goes off when no phone holds it any more */
    void mixerFlashUp(const QString &held);
    /** The settings of the mixers for the state: ID -> settings */
    QVariantMap mixersState() const;

private:
    DtcCore *m_core;
    QTcpServer *m_server;
    QWebSocketServer *m_ws;
    QList<QWebSocket *> m_sockets;
    QSet<QWebSocket *> m_authorized;
    QTimer *m_tick;
    /** Looks at the networks every 5 s while it runs */
    QTimer *m_network;
    /** The name on the network, while it runs */
    MdnsResponder *m_mdns;
    /** This program, for a page to tell it from another computer that
     *  answers for the name (a new one each time it starts) */
    QString m_instance;
    /** The addresses as addresses() gives them, read last */
    QVariantList m_addresses;
    QByteArray m_lastState;
    /** When the phones were last given the clock, while a timer runs */
    QElapsedTimer m_clockSent;
    QString m_lastError;
    bool m_layoutDirty;
    /** The groups as the layout last had them */
    QString m_groupsLaidOut;
    /** The mixers as the layout last had them */
    QString m_mixersLaidOut;
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
        // (a fader: the values of its targets before, given back exactly)
        QVariantList levels;
    };
    QHash<QWebSocket *, QHash<QString, PressHold>> m_presses;
    /** The flashes of the mixer each phone holds down: "<mixer>/<key>" ->
     *  how many times */
    QHash<QWebSocket *, QHash<QString, int>> m_mixerFlashes;
    /** The universes each phone shows, and their levels as it was last sent
     *  them (only the phones that show some) */
    QHash<QWebSocket *, QSet<int>> m_watch;
    QHash<QWebSocket *, QHash<int, QByteArray>> m_sentLevels;
};

#endif

/*
  DMX Total Control
  mdnsresponder.h

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

#ifndef MDNSRESPONDER_H
#define MDNSRESPONDER_H

#include <QNetworkInterface>
#include <QHostAddress>
#include <QByteArray>
#include <QObject>
#include <QPair>
#include <QList>

class QUdpSocket;
class QTimer;

/**
 * A name for this computer on the local network, as Multicast DNS (RFC
 * 6762) gives it: the phones open http://<name>.local:<port>/ and get the
 * address of this computer on their network, also when it changes. Only the
 * A record of the name (IPv4), on UDP 5353 shared with the other programs
 * that use it (the DNS Client of Windows among them).
 *
 * The name is probed first (three queries 250 ms apart): when another
 * computer answers that it has it, this one tries name-2, then name-3 … up
 * to name-9. Then it is announced twice, 1 s apart, and given up (a goodbye)
 * when it stops. A computer that claims it later keeps it when its address
 * is the lower one on that network; otherwise this one takes the next.
 *
 * The queries are answered on the network they come from, with the address
 * this computer has there: to the group, to the port of a phone that asks
 * from a port of its own (Android: a legacy unicast response, with its
 * question and its ID), or to the one that asked, when it asks for it (QU).
 * A query for the AAAA record gets an NSEC that says there is only the A
 * record, so that the phone doesn't wait for an IPv6 address.
 */
class MdnsResponder : public QObject
{
    Q_OBJECT

public:
    explicit MdnsResponder(QObject *parent = nullptr);
    ~MdnsResponder();

    /** Tests: false and start() opens nothing (no multicast on the networks
     *  of the computer); startOn() still works */
    static void setNetworkEnabled(bool enabled);
    static bool networkEnabled();

    /** Answer for $name.local on UDP 5353 of every network. False when it
     *  can't (the port isn't available: failed(); or networkEnabled() is
     *  false) */
    bool start(const QString &name);
    /** Answer for $name.local on a port of $bind (0: any free one): the
     *  queries come and the probes and announcements go to the group of
     *  each network with $multicast, to $bind itself without it (the tests:
     *  the loopback, nothing on the networks). False when the port can't be
     *  opened (failed()) */
    bool startOn(const QString &name, quint16 port, const QHostAddress &bind, bool multicast);
    /** Give the name up (a goodbye) and close the port */
    void stop();
    /** The networks again: on a new network or a new address the name is
     *  probed again there and announced. Nothing when they didn't change */
    void refreshInterfaces();

    /** "<name>.local", or "<name>-2.local" … when another computer has the
     *  name. Empty while it is not announced yet (probing), when it is
     *  stopped, or when name-9 was taken too */
    QString hostName() const;
    /** The name asked for, without the suffix (empty when it is stopped) */
    QString name() const;
    /** The port couldn't be opened */
    bool failed() const;
    /** It answers, but no name is its own: other computers have the name and
     *  name-2 … name-9 (until the networks change) */
    bool gaveUp() const;
    /** The port it answers on (0 when it is stopped) */
    quint16 port() const;

    /** The answer to a $query for $host, with $address: empty when the query
     *  doesn't ask for it (or is malformed). $legacy: to a port that is not
     *  5353 (its ID and its question repeated, TTL 10, no cache flush);
     *  $unicast is true when it goes to the one that asked */
    static QByteArray reply(const QByteArray &query, const QString &host, const QHostAddress &address,
                            bool legacy, bool *unicast);
    /** Whether a $response says that $host is at an address that is not one
     *  of $ours (another computer has it): that address in $other. False
     *  for a query, a goodbye (TTL 0) and anything malformed */
    static bool claims(const QByteArray &response, const QString &host, const QList<QHostAddress> &ours,
                       QHostAddress *other = nullptr);
    /** The question that probes $host: does anyone have it? (QM, any type) */
    static QByteArray probe(const QString &host);
    /** The A record of $host at $address, as a response to the group (TTL 0:
     *  the goodbye) */
    static QByteArray announcement(const QString &host, const QHostAddress &address, quint32 ttl);
    /** The name of an $attempt: $name the first, $name-2 … after (at most 63
     *  characters) */
    static QString candidate(const QString &name, int attempt);
    /** The first of $networks (an address and its prefix length each) that
     *  $address is on: -1 for none. The packets from no network of this
     *  computer are ignored, the claims too (RFC 6762 §11) */
    static int networkOf(const QHostAddress &address, const QList<QPair<QHostAddress, int>> &networks);

signals:
    /** hostName(), failed() or gaveUp() changed */
    void hostChanged();

private slots:
    void slotReadyRead();
    /** The next probe, then the name is ours and announced */
    void slotTimer();

private:
    /** An IPv4 address of a network of this computer */
    struct Local
    {
        QNetworkInterface network;
        QHostAddress ip;
        int prefix = 32;
    };

    /** Read the networks: @return true when they changed */
    bool readInterfaces();
    /** Join the group on each network (again) */
    void joinGroups();
    /** The networks it sends on: one address each (the bind address
     *  without multicast) */
    QList<Local> links() const;
    /** The address of this computer on the network of $address (null: none) */
    const Local *localFor(const QHostAddress &address) const;
    /** Every address of this computer, also the ones it had before the last
     *  change (its own packets that come back are no conflict) */
    QList<QHostAddress> ours() const;
    void sendToGroup(const QByteArray &packet, const Local &link);
    void announce(quint32 ttl);
    /** Probe the candidate of the attempt (again) */
    void startProbing();
    /** Another computer has the candidate: the next one */
    void nextCandidate();
    void handlePacket(const QByteArray &packet, const QHostAddress &sender, quint16 senderPort);

private:
    QUdpSocket *m_socket;
    QTimer *m_timer;
    /** The name asked for, without the suffix */
    QString m_name;
    /** 1: the name, 2 to 9: name-N; 0: none */
    int m_attempt;
    bool m_probing;
    /** The probes sent of this round */
    int m_step;
    /** The announcements still to send */
    int m_announcements;
    QString m_hostName;
    bool m_failed;
    quint16 m_port;
    QHostAddress m_bind;
    bool m_multicast;
    QList<Local> m_locals;
    QList<Local> m_links;
    QList<QHostAddress> m_previous;
    /** The networks and their addresses as last read, to tell a change */
    QString m_signature;
};

#endif

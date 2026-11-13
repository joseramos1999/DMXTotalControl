/*
  DMX Total Control
  mdnsresponder.cpp

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

#include <QNetworkDatagram>
#include <QRandomGenerator>
#include <QUdpSocket>
#include <QVariant>
#include <QTimer>
#include <QDebug>

#include "mdnsresponder.h"

// the group and the port of Multicast DNS on IPv4 (RFC 6762 §3)
#define KGroup QStringLiteral("224.0.0.251")
static const quint16 KPort = 5353;
// bigger packets are not read (RFC 6762 §17)
static const int KMaxPacket = 9000;

namespace
{

enum : quint16
{
    TypeA = 1,
    TypeAAAA = 28,
    TypeNSEC = 47,
    TypeAny = 255,
    ClassIN = 1,
    ClassAny = 255,
    // the top bit of the class: QU in a question, cache flush in a record
    TopBit = 0x8000,
    FlagResponse = 0x8000,
    FlagAuthoritative = 0x0400,
    OpcodeMask = 0x7800,
};

bool s_networkEnabled = true;

/** Reads a DNS packet from its start: false when something is missing or
 *  malformed (the reading stops there) */
class PacketReader
{
public:
    explicit PacketReader(const QByteArray &data)
        : m_data(data)
    {
    }

    bool u16(quint16 *value)
    {
        if (m_pos + 2 > m_data.size())
            return false;
        *value = quint16((quint8(m_data.at(m_pos)) << 8) | quint8(m_data.at(m_pos + 1)));
        m_pos += 2;
        return true;
    }

    bool u32(quint32 *value)
    {
        quint16 high = 0, low = 0;
        if (u16(&high) == false || u16(&low) == false)
            return false;
        *value = (quint32(high) << 16) | low;
        return true;
    }

    bool bytes(int count, QByteArray *value)
    {
        if (m_pos + count > m_data.size())
            return false;
        *value = m_data.mid(m_pos, count);
        m_pos += count;
        return true;
    }

    /** A name: its labels in lower case (DNS compares them without case),
     *  the pointers of the compression followed (16 at most: a loop ends),
     *  255 bytes at most */
    bool name(QList<QByteArray> *labels)
    {
        labels->clear();
        int pos = m_pos;
        int jumps = 0;
        // (the zero at the end)
        int length = 1;
        bool jumped = false;
        for (;;)
        {
            if (pos >= m_data.size())
                return false;
            const quint8 size = quint8(m_data.at(pos));
            if ((size & 0xC0) == 0xC0)
            {
                if (pos + 1 >= m_data.size() || ++jumps > 16)
                    return false;
                // (the name goes on somewhere else; the reading, after the pointer)
                if (jumped == false)
                    m_pos = pos + 2;
                jumped = true;
                pos = ((size & 0x3F) << 8) | quint8(m_data.at(pos + 1));
                continue;
            }
            // (0x40 and 0x80: kinds of labels nobody uses)
            if (size & 0xC0)
                return false;
            if (size == 0)
            {
                if (jumped == false)
                    m_pos = pos + 1;
                return true;
            }
            length += size + 1;
            if (length > 255 || pos + 1 + size > m_data.size())
                return false;
            labels->append(m_data.mid(pos + 1, size).toLower());
            pos += size + 1;
        }
    }

private:
    const QByteArray &m_data;
    int m_pos = 0;
};

struct Header
{
    quint16 id = 0;
    quint16 flags = 0;
    quint16 questions = 0;
    quint16 answers = 0;
    quint16 authority = 0;
    quint16 additional = 0;
};

bool readHeader(PacketReader &in, Header *header)
{
    return in.u16(&header->id) && in.u16(&header->flags) && in.u16(&header->questions)
           && in.u16(&header->answers) && in.u16(&header->authority) && in.u16(&header->additional);
}

/** The labels of a host name in lower case ("name.local"): empty when it
 *  isn't one (an empty label, one over 63 bytes) */
QList<QByteArray> labelsOf(const QString &host)
{
    QByteArray name = host.toLatin1().toLower();
    if (name.endsWith('.'))
        name.chop(1);
    const QList<QByteArray> labels = name.split('.');
    for (const QByteArray &label : labels)
        if (label.isEmpty() || label.size() > 63)
            return {};
    return labels;
}

void putU16(QByteArray &out, quint16 value)
{
    out.append(char(value >> 8));
    out.append(char(value & 0xFF));
}

void putU32(QByteArray &out, quint32 value)
{
    putU16(out, quint16(value >> 16));
    putU16(out, quint16(value & 0xFFFF));
}

void putName(QByteArray &out, const QList<QByteArray> &labels)
{
    for (const QByteArray &label : labels)
    {
        out.append(char(label.size()));
        out.append(label);
    }
    out.append('\0');
}

/** A pointer to the name at 12, the first after the header */
void putFirstName(QByteArray &out)
{
    out.append(char(0xC0));
    out.append(char(0x0C));
}

void putHeader(QByteArray &out, quint16 id, quint16 flags, quint16 questions, quint16 answers)
{
    putU16(out, id);
    putU16(out, flags);
    putU16(out, questions);
    putU16(out, answers);
    putU16(out, 0);
    putU16(out, 0);
}

} // namespace

MdnsResponder::MdnsResponder(QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_timer(new QTimer(this))
    , m_attempt(0)
    , m_probing(false)
    , m_step(0)
    , m_announcements(0)
    , m_failed(false)
    , m_port(0)
    , m_multicast(false)
{
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &MdnsResponder::slotTimer);
}

MdnsResponder::~MdnsResponder()
{
    // (the goodbye, nobody told: the owner is going too)
    blockSignals(true);
    stop();
}

void MdnsResponder::setNetworkEnabled(bool enabled)
{
    s_networkEnabled = enabled;
}

bool MdnsResponder::networkEnabled()
{
    return s_networkEnabled;
}

/*********************************************************************
 * Start and stop
 *********************************************************************/

bool MdnsResponder::start(const QString &name)
{
    if (s_networkEnabled == false)
    {
        stop();
        return false;
    }
    return startOn(name, KPort, QHostAddress::AnyIPv4, true);
}

bool MdnsResponder::startOn(const QString &name, quint16 port, const QHostAddress &bind, bool multicast)
{
    stop();
    if (name.isEmpty())
        return false;

    m_socket = new QUdpSocket(this);
    // (shared: the DNS Client of Windows and other programs use 5353 too)
    if (m_socket->bind(bind, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint) == false)
    {
        qWarning() << "[mDNS] port" << port << m_socket->errorString();
        delete m_socket;
        m_socket = nullptr;
        m_failed = true;
        emit hostChanged();
        return false;
    }
    m_name = name;
    m_port = m_socket->localPort();
    m_bind = bind;
    m_multicast = multicast;
    connect(m_socket, &QUdpSocket::readyRead, this, &MdnsResponder::slotReadyRead);
    readInterfaces();
    if (multicast)
    {
        // (RFC 6762 §11: 255, what a packet from the same network has)
        m_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, QVariant(255));
        joinGroups();
    }
    nextCandidate();
    return true;
}

void MdnsResponder::stop()
{
    m_timer->stop();
    if (m_socket != nullptr)
    {
        // the goodbye: the phones forget the address now, not in 2 minutes
        if (m_hostName.isEmpty() == false)
            announce(0);
        // (later: this may be in its readyRead)
        m_socket->disconnect(this);
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    const bool told = m_hostName.isEmpty() == false || m_failed;
    m_hostName.clear();
    m_name.clear();
    m_failed = false;
    m_attempt = 0;
    m_probing = false;
    m_step = 0;
    m_announcements = 0;
    m_port = 0;
    m_locals.clear();
    m_links.clear();
    m_previous.clear();
    m_signature.clear();
    if (told)
        emit hostChanged();
}

void MdnsResponder::refreshInterfaces()
{
    if (m_socket == nullptr)
        return;
    const QList<Local> before = m_links;
    if (readInterfaces() == false)
        return;
    if (m_multicast)
    {
        const QHostAddress group(KGroup);
        for (const Local &link : before)
            m_socket->leaveMulticastGroup(group, link.network);
        joinGroups();
    }
    // the name is probed on the new network (from the start again when
    // name-9 was taken on the old one: it no longer gives up)
    if (m_attempt == 0)
    {
        nextCandidate();
        emit hostChanged();
    }
    else
    {
        startProbing();
    }
}

QString MdnsResponder::hostName() const
{
    return m_hostName;
}

QString MdnsResponder::name() const
{
    return m_name;
}

bool MdnsResponder::failed() const
{
    return m_failed;
}

bool MdnsResponder::gaveUp() const
{
    return m_socket != nullptr && m_attempt == 0 && m_hostName.isEmpty();
}

quint16 MdnsResponder::port() const
{
    return m_port;
}

/*********************************************************************
 * Networks
 *********************************************************************/

bool MdnsResponder::readInterfaces()
{
    QList<Local> locals;
    QList<Local> links;
    QStringList signature;
    for (const QNetworkInterface &network : QNetworkInterface::allInterfaces())
    {
        const QNetworkInterface::InterfaceFlags flags = network.flags();
        if (flags.testFlag(QNetworkInterface::IsUp) == false || flags.testFlag(QNetworkInterface::IsRunning) == false)
            continue;
        const bool sends = flags.testFlag(QNetworkInterface::CanMulticast)
                           && flags.testFlag(QNetworkInterface::IsLoopBack) == false;
        bool linked = false;
        for (const QNetworkAddressEntry &entry : network.addressEntries())
        {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
                continue;
            const Local local { network, entry.ip(), entry.prefixLength() };
            locals.append(local);
            // (the first address of a network is the one it announces there)
            if (sends && linked == false)
                links.append(local);
            linked = true;
            signature.append(QString("%1 %2/%3").arg(network.index()).arg(entry.ip().toString()).arg(entry.prefixLength()));
        }
    }
    const QString read = signature.join(',');
    if (read == m_signature)
        return false;
    m_previous.clear();
    for (const Local &local : std::as_const(m_locals))
        m_previous.append(local.ip);
    m_locals = locals;
    m_links = links;
    m_signature = read;
    return true;
}

void MdnsResponder::joinGroups()
{
    const QHostAddress group(KGroup);
    for (const Local &link : std::as_const(m_links))
        if (m_socket->joinMulticastGroup(group, link.network) == false)
            qWarning() << "[mDNS]" << link.network.humanReadableName() << m_socket->errorString();
}

QList<MdnsResponder::Local> MdnsResponder::links() const
{
    if (m_multicast)
        return m_links;
    return { Local { QNetworkInterface(), m_bind, 32 } };
}

int MdnsResponder::networkOf(const QHostAddress &address, const QList<QPair<QHostAddress, int>> &networks)
{
    for (int i = 0; i < networks.count(); i++)
        if (networks.at(i).second >= 0 && address.isInSubnet(networks.at(i).first, networks.at(i).second))
            return i;
    return -1;
}

const MdnsResponder::Local *MdnsResponder::localFor(const QHostAddress &address) const
{
    QList<QPair<QHostAddress, int>> networks;
    for (const Local &local : m_locals)
        networks.append(qMakePair(local.ip, local.prefix));
    const int index = networkOf(address, networks);
    return index < 0 ? nullptr : &m_locals.at(index);
}

QList<QHostAddress> MdnsResponder::ours() const
{
    QList<QHostAddress> list = m_previous;
    for (const Local &local : m_locals)
        list.append(local.ip);
    if (m_multicast == false)
        list.append(m_bind);
    return list;
}

void MdnsResponder::sendToGroup(const QByteArray &packet, const Local &link)
{
    if (m_socket == nullptr || packet.isEmpty())
        return;
    if (m_multicast)
    {
        m_socket->setMulticastInterface(link.network);
        m_socket->writeDatagram(packet, QHostAddress(KGroup), m_port);
    }
    else
        m_socket->writeDatagram(packet, m_bind, m_port);
}

void MdnsResponder::announce(quint32 ttl)
{
    // (each network with the address this computer has there)
    for (const Local &link : links())
        sendToGroup(announcement(m_hostName, link.ip, ttl), link);
}

/*********************************************************************
 * The name
 *********************************************************************/

QString MdnsResponder::candidate(const QString &name, int attempt)
{
    if (attempt <= 1)
        return name;
    const QString suffix = QString("-%1").arg(attempt);
    return name.left(63 - suffix.length()) + suffix;
}

void MdnsResponder::startProbing()
{
    m_probing = true;
    m_step = 0;
    m_announcements = 0;
    // (a random wait first, RFC 6762 §8.1: two computers that start
    // together don't probe together)
    m_timer->start(int(QRandomGenerator::global()->bounded(250)));
}

void MdnsResponder::nextCandidate()
{
    m_attempt++;
    if (m_attempt <= 9)
    {
        startProbing();
        return;
    }
    // name-2 … name-9 are taken too: no name (the addresses still work)
    qWarning() << "[mDNS] no free name for" << m_name;
    m_attempt = 0;
    m_probing = false;
    m_timer->stop();
    m_hostName.clear();
    // (told also when no name was announced yet: gaveUp() changed)
    emit hostChanged();
}

void MdnsResponder::slotTimer()
{
    if (m_socket == nullptr || m_attempt == 0)
        return;
    bool renamed = false;
    if (m_probing)
    {
        const QString host = candidate(m_name, m_attempt) + ".local";
        if (m_step < 3)
        {
            const QByteArray packet = probe(host);
            for (const Local &link : links())
                sendToGroup(packet, link);
            m_step++;
            m_timer->start(250);
            return;
        }
        // nobody said it has it: it is this computer's
        m_probing = false;
        m_announcements = 2;
        renamed = host != m_hostName;
        m_hostName = host;
    }
    if (m_announcements > 0)
    {
        announce(120);
        if (--m_announcements > 0)
            m_timer->start(1000);
    }
    if (renamed)
        emit hostChanged();
}

void MdnsResponder::slotReadyRead()
{
    while (m_socket != nullptr && m_socket->hasPendingDatagrams())
    {
        const QNetworkDatagram datagram = m_socket->receiveDatagram(KMaxPacket + 1);
        if (datagram.isValid() == false || datagram.data().size() > KMaxPacket)
            continue;
        handlePacket(datagram.data(), datagram.senderAddress(), quint16(datagram.senderPort()));
    }
}

void MdnsResponder::handlePacket(const QByteArray &packet, const QHostAddress &sender, quint16 senderPort)
{
    // not from a network of this computer: ignored, a claim too (RFC 6762
    // §11), so that a host of another network can't take the name away
    const Local *local = localFor(sender);
    if (local == nullptr)
        return;

    // another computer says it has the name
    QHostAddress other;
    if (m_attempt > 0 && claims(packet, candidate(m_name, m_attempt) + ".local", ours(), &other))
    {
        if (m_probing == false)
        {
            // (claimed after it was announced: the lower address on that
            // network keeps it, and says so again)
            const Local *mine = localFor(other);
            if (mine != nullptr && mine->ip.toIPv4Address() < other.toIPv4Address())
            {
                announce(120);
                return;
            }
        }
        qInfo() << "[mDNS]" << candidate(m_name, m_attempt) << "is used by" << other.toString();
        const bool told = m_hostName.isEmpty() == false;
        m_hostName.clear();
        nextCandidate();
        // (when it gives up, nextCandidate() has told)
        if (told && m_attempt > 0)
            emit hostChanged();
        return;
    }

    // a question: answered once the name is this computer's
    if (m_probing || m_hostName.isEmpty())
        return;
    const bool legacy = senderPort != m_port;
    bool unicast = false;
    const QByteArray answer = reply(packet, m_hostName, local->ip, legacy, &unicast);
    if (answer.isEmpty())
        return;
    if (unicast)
        m_socket->writeDatagram(answer, sender, legacy ? senderPort : m_port);
    else
        sendToGroup(answer, *local);
}

/*********************************************************************
 * Packets
 *********************************************************************/

QByteArray MdnsResponder::reply(const QByteArray &query, const QString &host, const QHostAddress &address,
                                bool legacy, bool *unicast)
{
    if (unicast != nullptr)
        *unicast = false;
    const QList<QByteArray> wanted = labelsOf(host);
    bool isIPv4 = false;
    const quint32 ip = address.toIPv4Address(&isIPv4);
    if (wanted.isEmpty() || isIPv4 == false)
        return QByteArray();

    PacketReader in(query);
    Header header;
    // (a response, or not a standard query)
    if (readHeader(in, &header) == false || (header.flags & FlagResponse) || (header.flags & OpcodeMask))
        return QByteArray();

    bool answerA = false;
    bool answerNsec = false;
    bool qu = false;
    quint16 firstType = 0;
    quint16 firstClass = ClassIN;
    for (int i = 0; i < header.questions; i++)
    {
        QList<QByteArray> labels;
        quint16 type = 0, klass = 0;
        if (in.name(&labels) == false || in.u16(&type) == false || in.u16(&klass) == false)
            return QByteArray();
        const quint16 recordClass = klass & ~TopBit;
        if (labels != wanted || (recordClass != ClassIN && recordClass != ClassAny))
            continue;
        if (type == TypeA || type == TypeAny)
            answerA = true;
        // (there is no IPv6 address: an NSEC says so)
        else if (type == TypeAAAA)
            answerNsec = true;
        else
            continue;
        if (firstType == 0)
        {
            firstType = type;
            firstClass = recordClass;
        }
        if (klass & TopBit)
            qu = true;
    }
    if (answerA == false && answerNsec == false)
        return QByteArray();

    // to a port that is not 5353: a plain DNS answer (RFC 6762 §6.7)
    const quint16 recordClass = legacy ? quint16(ClassIN) : quint16(ClassIN | TopBit);
    const quint32 ttl = legacy ? 10 : 120;
    QByteArray out;
    putHeader(out, legacy ? header.id : 0, FlagResponse | FlagAuthoritative, legacy ? 1 : 0,
              quint16(int(answerA) + int(answerNsec)));
    // the name goes once, at 12 (the question, or the first record); the
    // records point at it
    bool named = false;
    auto putOwner = [&out, &named, &wanted]() {
        if (named)
            putFirstName(out);
        else
            putName(out, wanted);
        named = true;
    };
    if (legacy)
    {
        putOwner();
        putU16(out, firstType);
        putU16(out, firstClass);
    }
    if (answerA)
    {
        putOwner();
        putU16(out, TypeA);
        putU16(out, recordClass);
        putU32(out, ttl);
        putU16(out, 4);
        putU32(out, ip);
    }
    if (answerNsec)
    {
        putOwner();
        putU16(out, TypeNSEC);
        putU16(out, recordClass);
        putU32(out, ttl);
        putU16(out, 5);
        // the next name is the name itself; the types: window 0, 1 byte,
        // only A (bit 1)
        putFirstName(out);
        out.append(char(0x00));
        out.append(char(0x01));
        out.append(char(0x40));
    }
    if (unicast != nullptr)
        *unicast = legacy || qu;
    return out;
}

bool MdnsResponder::claims(const QByteArray &response, const QString &host, const QList<QHostAddress> &ours,
                           QHostAddress *other)
{
    const QList<QByteArray> wanted = labelsOf(host);
    PacketReader in(response);
    Header header;
    if (wanted.isEmpty() || readHeader(in, &header) == false || (header.flags & FlagResponse) == 0)
        return false;
    for (int i = 0; i < header.questions; i++)
    {
        QList<QByteArray> labels;
        quint16 type = 0, klass = 0;
        if (in.name(&labels) == false || in.u16(&type) == false || in.u16(&klass) == false)
            return false;
    }
    const int records = header.answers + header.authority + header.additional;
    for (int i = 0; i < records; i++)
    {
        QList<QByteArray> labels;
        quint16 type = 0, klass = 0, length = 0;
        quint32 ttl = 0;
        QByteArray data;
        if (in.name(&labels) == false || in.u16(&type) == false || in.u16(&klass) == false
            || in.u32(&ttl) == false || in.u16(&length) == false || in.bytes(length, &data) == false)
            return false;
        // (a goodbye gives the name up)
        if (labels != wanted || type != TypeA || (klass & ~TopBit) != ClassIN || length != 4 || ttl == 0)
            continue;
        const QHostAddress address(quint32((quint8(data.at(0)) << 24) | (quint8(data.at(1)) << 16)
                                           | (quint8(data.at(2)) << 8) | quint8(data.at(3))));
        if (ours.contains(address))
            continue;
        if (other != nullptr)
            *other = address;
        return true;
    }
    return false;
}

QByteArray MdnsResponder::probe(const QString &host)
{
    const QList<QByteArray> labels = labelsOf(host);
    if (labels.isEmpty())
        return QByteArray();
    QByteArray out;
    putHeader(out, 0, 0, 1, 0);
    putName(out, labels);
    putU16(out, TypeAny);
    putU16(out, ClassIN);
    return out;
}

QByteArray MdnsResponder::announcement(const QString &host, const QHostAddress &address, quint32 ttl)
{
    const QList<QByteArray> labels = labelsOf(host);
    bool isIPv4 = false;
    const quint32 ip = address.toIPv4Address(&isIPv4);
    if (labels.isEmpty() || isIPv4 == false)
        return QByteArray();
    QByteArray out;
    putHeader(out, 0, FlagResponse | FlagAuthoritative, 0, 1);
    putName(out, labels);
    putU16(out, TypeA);
    putU16(out, ClassIN | TopBit);
    putU32(out, ttl);
    putU16(out, 4);
    putU32(out, ip);
    return out;
}

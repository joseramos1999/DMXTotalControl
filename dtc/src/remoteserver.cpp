/*
  DMX Total Control
  remoteserver.cpp

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

#include <QNetworkInterface>
#include <QWebSocketServer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QWebSocket>
#include <QFileInfo>
#include <QSettings>
#include <QUrlQuery>
#include <QTimer>
#include <QFile>
#include <QUrl>
#include <QDebug>
#include <QColor>
#include <functional>

#include "remoteserver.h"
#include "playbackmanager.h"
#include "showsurface.h"
#include "zonemanager.h"
#include "controlmapper.h"
#include "drivecontrol.h"
#include "livemixer.h"
#include "dtccore.h"
#include "zone.h"

#include "mastertimer.h"
#include "function.h"
#include "doc.h"

#define KSettingRemoteEnabled QStringLiteral("remote/enabled")
#define KSettingRemotePort    QStringLiteral("remote/port")
#define KSettingRemotePin     QStringLiteral("remote/pin")

RemoteServer::RemoteServer(DtcCore *core, QObject *parent)
    : QObject(parent)
    , m_core(core)
    , m_server(nullptr)
    , m_ws(nullptr)
    , m_tick(new QTimer(this))
    , m_layoutDirty(false)
{
    m_tick->setInterval(120);
    connect(m_tick, &QTimer::timeout, this, &RemoteServer::slotTick);
    connect(m_core->surface(), &ShowSurface::pagesChanged, this, &RemoteServer::slotLayoutChanged);
    connect(m_core->surface(), &ShowSurface::cleared, this, &RemoteServer::slotShowCleared);
    connect(m_core, &DtcCore::functionsChanged, this, &RemoteServer::slotLayoutChanged);
    connect(m_core->palettes(), &PaletteManager::palettesChanged, this, &RemoteServer::slotLayoutChanged);
    connect(m_core, &DtcCore::fileNameChanged, this, &RemoteServer::slotLayoutChanged);
    // the theme of the program is the theme of the remote
    connect(m_core->settings(), &AppSettings::themeChanged, this, &RemoteServer::slotLayoutChanged);
    // the page of the playbacks comes and goes with them, and has a strip per slot
    connect(m_core->playbacks(), &PlaybackManager::pagesChanged, this, &RemoteServer::slotLayoutChanged);
    connect(m_core->playbacks(), &PlaybackManager::assignmentsChanged, this, &RemoteServer::slotLayoutChanged);
    // the flashes the phones hold on a page that moves
    connect(m_core->playbacks(), &PlaybackManager::pagesRenumbered, this, &RemoteServer::slotPlaybackPagesRenumbered);
    // the groups and their scenes (added, deleted, renamed, moved)
    connect(m_core->zones(), &ZoneManager::countChanged, this, &RemoteServer::slotZonesChanged);
    connect(m_core->zones(), &ZoneManager::modelReset, this, &RemoteServer::slotZonesChanged);
    connect(m_core->zones(), &ZoneManager::rowsMoved, this, &RemoteServer::slotZonesChanged);
    slotZonesChanged();
}

RemoteServer::~RemoteServer()
{
    stop();
}

/*********************************************************************
 * Settings
 *********************************************************************/

bool RemoteServer::enabled() const
{
    return QSettings().value(KSettingRemoteEnabled, false).toBool();
}

void RemoteServer::setEnabled(bool enable)
{
    if (enable == enabled())
        return;
    QSettings().setValue(KSettingRemoteEnabled, enable);
    emit settingsChanged();
    apply();
}

int RemoteServer::port() const
{
    return QSettings().value(KSettingRemotePort, 8800).toInt();
}

void RemoteServer::setPort(int port)
{
    port = qBound(1024, port, 65535);
    if (port == this->port())
        return;
    QSettings().setValue(KSettingRemotePort, port);
    emit settingsChanged();
    // on the new port, also when it couldn't start on the old one (in use)
    if (enabled())
    {
        stop();
        apply();
    }
}

QString RemoteServer::pin() const
{
    return QSettings().value(KSettingRemotePin).toString();
}

void RemoteServer::setPin(const QString &pin)
{
    if (pin == this->pin())
        return;
    QSettings().setValue(KSettingRemotePin, pin.trimmed());
    emit settingsChanged();
}

bool RemoteServer::running() const
{
    return m_server != nullptr && m_server->isListening();
}

QStringList RemoteServer::urls() const
{
    QStringList list;
    if (running() == false)
        return list;
    for (const QHostAddress &address : QNetworkInterface::allAddresses())
    {
        if (address.protocol() != QAbstractSocket::IPv4Protocol || address.isLoopback() || address.isLinkLocal())
            continue;
        list.append(QString("http://%1:%2/").arg(address.toString()).arg(m_server->serverPort()));
    }
    if (list.isEmpty())
        list.append(QString("http://127.0.0.1:%1/").arg(m_server->serverPort()));
    return list;
}

int RemoteServer::clients() const
{
    return m_sockets.count();
}

QString RemoteServer::lastError() const
{
    return m_lastError;
}

void RemoteServer::apply()
{
    if (enabled() == false)
    {
        stop();
        return;
    }
    if (running())
        return;

    m_server = new QTcpServer(this);
    m_ws = new QWebSocketServer(QStringLiteral("DMX Total Control"), QWebSocketServer::NonSecureMode, this);
    connect(m_server, &QTcpServer::newConnection, this, &RemoteServer::slotNewConnection);
    connect(m_ws, &QWebSocketServer::newConnection, this, &RemoteServer::slotWebSocket);
    if (m_server->listen(QHostAddress::Any, quint16(port())) == false)
    {
        m_lastError = tr("Port %1 is in use: choose another one").arg(port());
        qWarning() << "[Remote]" << m_server->errorString();
        delete m_server;
        m_server = nullptr;
        delete m_ws;
        m_ws = nullptr;
    }
    else
    {
        m_lastError.clear();
        m_tick->start();
    }
    emit runningChanged();
}

void RemoteServer::startOn(int port)
{
    stop();
    m_server = new QTcpServer(this);
    m_ws = new QWebSocketServer(QStringLiteral("DMX Total Control"), QWebSocketServer::NonSecureMode, this);
    connect(m_server, &QTcpServer::newConnection, this, &RemoteServer::slotNewConnection);
    connect(m_ws, &QWebSocketServer::newConnection, this, &RemoteServer::slotWebSocket);
    if (m_server->listen(QHostAddress::Any, quint16(port)))
        m_tick->start();
    else
        qWarning() << "[Remote]" << m_server->errorString();
    emit runningChanged();
}

void RemoteServer::stop()
{
    m_tick->stop();
    for (QWebSocket *socket : std::as_const(m_sockets))
    {
        socket->disconnect(this);
        socket->close();
        socket->deleteLater();
    }
    m_sockets.clear();
    m_authorized.clear();
    // (the flashes and the SHOW buttons the phones held go up)
    const QList<QWebSocket *> holding = m_flashes.keys();
    for (QWebSocket *socket : holding)
        releaseFlashes(socket);
    m_flashes.clear();
    const QList<QWebSocket *> pressing = m_presses.keys();
    for (QWebSocket *socket : pressing)
        releasePresses(socket);
    m_presses.clear();
    delete m_server;
    m_server = nullptr;
    delete m_ws;
    m_ws = nullptr;
    // (an error of a start that failed: stopped, or trying again)
    m_lastError.clear();
    emit runningChanged();
    emit clientsChanged();
}

/*********************************************************************
 * HTTP
 *********************************************************************/

void RemoteServer::slotNewConnection()
{
    while (m_server != nullptr && m_server->hasPendingConnections())
    {
        QTcpSocket *socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &RemoteServer::slotHttpReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
    }
}

void RemoteServer::slotHttpReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == nullptr)
        return;

    // the whole request head first; it stays in the socket for the WebSocket handshake
    QByteArray head = socket->peek(16384);
    if (head.contains("\r\n\r\n") == false)
    {
        if (head.size() >= 16384)
            socket->abort();
        return;
    }
    if (head.toLower().contains("upgrade: websocket"))
    {
        disconnect(socket, nullptr, this, nullptr);
        disconnect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        if (m_ws != nullptr)
            m_ws->handleConnection(socket);
        return;
    }
    serveHttp(socket, socket->readAll());
}

static QByteArray contentType(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == "html") return "text/html; charset=utf-8";
    if (suffix == "js") return "application/javascript; charset=utf-8";
    if (suffix == "css") return "text/css; charset=utf-8";
    if (suffix == "otf") return "font/otf";
    if (suffix == "webmanifest") return "application/manifest+json";
    if (suffix == "png") return "image/png";
    if (suffix == "jpg" || suffix == "jpeg") return "image/jpeg";
    if (suffix == "gif") return "image/gif";
    if (suffix == "svg") return "image/svg+xml";
    if (suffix == "webp") return "image/webp";
    if (suffix == "bmp") return "image/bmp";
    return "application/octet-stream";
}

void RemoteServer::serveHttp(QTcpSocket *socket, const QByteArray &request)
{
    const QList<QByteArray> line = request.left(request.indexOf("\r\n")).split(' ');
    QByteArray status = "200 OK";
    QByteArray type;
    QByteArray body;

    QUrl url(line.size() >= 2 ? QString::fromUtf8(line.at(1)) : QString("/"));
    const QString path = url.path();
    QString file;
    if (line.value(0) != "GET")
        status = "405 Method Not Allowed";
    else if (path == "/" || path == "/index.html")
        file = ":/remote/index.html";
    else if (path == "/remote.js")
        file = ":/remote/remote.js";
    else if (path == "/remote.css")
        file = ":/remote/remote.css";
    else if (path == "/icons.otf")
        file = ":/FontAwesome7-Free-Solid-900.otf";
    else if (path == "/manifest.webmanifest")
        file = ":/remote/manifest.webmanifest";
    // the icon of the application: the tab, the home screen of the phone
    else if (path == "/apple-touch-icon.png")
        file = ":/icons/dtc-180.png";
    else if (path == "/favicon.ico" || path == "/icon-32.png")
        file = ":/icons/dtc-32.png";
    else if (path == "/icon-16.png" || path == "/icon-64.png" || path == "/icon-192.png" || path == "/icon-512.png")
        file = QStringLiteral(":/icons/dtc-%1.png").arg(path.mid(6, path.length() - 10));
    else if (path == "/image")
    {
        // only the pictures the SHOW shows
        QUrlQuery query(url);
        file = imagePath(query.queryItemValue("page").toInt(), query.queryItemValue("section"),
                         query.queryItemValue("cell").isEmpty() ? -1 : query.queryItemValue("cell").toInt());
    }

    if (status == "200 OK")
    {
        QFile f(file);
        if (file.isEmpty() || f.open(QIODevice::ReadOnly) == false)
        {
            status = "404 Not Found";
            body = "Not found";
            type = "text/plain";
        }
        else
        {
            body = f.readAll();
            type = contentType(file);
        }
    }

    QByteArray response = "HTTP/1.1 " + status + "\r\n"
                          "Content-Type: " + (type.isEmpty() ? QByteArray("text/plain") : type) + "\r\n"
                          "Content-Length: " + QByteArray::number(body.size()) + "\r\n"
                          "Cache-Control: no-cache\r\n"
                          "Connection: close\r\n\r\n";
    socket->write(response + body);
    socket->disconnectFromHost();
}

QString RemoteServer::imagePath(int page, const QString &section, int cell) const
{
    QVariantMap node = m_core->surface()->section(page, section);
    QString image = cell >= 0 ? itemAt(page, section, cell).value("image").toString() : node.value("image").toString();
    if (image.isEmpty())
        return QString();
    return image.startsWith("file:") ? QUrl(image).toLocalFile() : image;
}

/*********************************************************************
 * WebSocket
 *********************************************************************/

void RemoteServer::slotWebSocket()
{
    while (m_ws != nullptr && m_ws->hasPendingConnections())
    {
        QWebSocket *socket = m_ws->nextPendingConnection();
        connect(socket, &QWebSocket::textMessageReceived, this, &RemoteServer::slotTextMessage);
        connect(socket, &QWebSocket::disconnected, this, &RemoteServer::slotSocketGone);
        m_sockets.append(socket);
        emit clientsChanged();
    }
}

void RemoteServer::slotSocketGone()
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (socket == nullptr)
        return;
    m_sockets.removeAll(socket);
    m_authorized.remove(socket);
    // a phone that goes away with a finger on a flash (or on a SHOW button,
    // or the Flash of a fader): it goes up
    releaseFlashes(socket);
    releasePresses(socket);
    socket->deleteLater();
    emit clientsChanged();
}

bool RemoteServer::trackPress(QWebSocket *socket, const QVariantMap &message)
{
    const QString cmd = message.value("cmd").toString();
    const bool fader = cmd == "faderFlash";
    if (cmd != "press" && cmd != "release" && fader == false)
        return false;

    const bool down = fader ? message.value("down").toBool() : cmd == "press";
    const int page = message.value("page").toInt();
    const QString section = message.value("section").toString();
    const int cell = message.value("cell").toInt();
    const QString key = QString("%1/%2/%3/%4").arg(fader ? "f" : "b").arg(page).arg(section).arg(cell);
    ShowSurface *surface = m_core->surface();

    if (down)
    {
        PressHold hold = m_presses.value(socket).value(key);
        if (hold.count <= 0)
        {
            hold = PressHold();
            hold.item = itemAt(page, section, cell);
            if (hold.item.isEmpty())
                return true;
            hold.fader = fader;
            if (fader)
                hold.levels = surface->holdLevels(hold.item);
        }
        hold.count++;
        m_presses[socket].insert(key, hold);
        if (fader)
            surface->setLevel(hold.item, 1.0);
        else
            surface->press(hold.item);
        return true;
    }

    // (an up of something this phone doesn't hold: not somebody else's hold)
    auto held = m_presses.find(socket);
    if (held == m_presses.end() || held.value().contains(key) == false)
        return true;
    PressHold hold = held.value().value(key);
    hold.count--;
    if (hold.count > 0)
        held.value().insert(key, hold);
    else
        held.value().remove(key);
    if (held.value().isEmpty())
        m_presses.erase(held);
    // what it pressed, wherever it is now
    if (fader == false)
        surface->release(hold.item);
    else if (hold.count <= 0)
        surface->restoreLevels(hold.item, hold.levels);
    return true;
}

void RemoteServer::releasePresses(QWebSocket *socket)
{
    const QHash<QString, PressHold> held = m_presses.take(socket);
    for (const PressHold &hold : held)
    {
        if (hold.fader)
        {
            m_core->surface()->restoreLevels(hold.item, hold.levels);
            continue;
        }
        for (int i = 0; i < hold.count; i++)
            m_core->surface()->release(hold.item);
    }
}

bool RemoteServer::trackFlash(QWebSocket *socket, QVariantMap &message)
{
    if (message.value("cmd").toString() != "playback" || message.value("action").toString() != "flash")
        return true;
    const QPair<int, int> flash(message.value("pbPage", -1).toInt(), message.value("slot").toInt());
    // (nothing past the slots: the command does nothing either)
    if (flash.second < 0 || flash.second >= PlaybackManager::maxSlots())
        return true;
    if (message.value("down").toBool())
    {
        FlashHold &hold = m_flashes[socket][flash];
        if (hold.count <= 0 || hold.gone)
            hold = FlashHold{ 0, flash.first, false };
        hold.count++;
        message.insert("pbPage", hold.page);
        return true;
    }
    auto held = m_flashes.find(socket);
    if (held == m_flashes.end() || held.value().contains(flash) == false)
        return false;
    // on the page it is on now; its page deleted, the playbacks let go of it
    const FlashHold hold = held.value().value(flash);
    message.insert("pbPage", hold.page);
    // (two fingers on it: up when both are)
    if (hold.count <= 1)
        held.value().remove(flash);
    else
        held.value()[flash].count--;
    if (held.value().isEmpty())
        m_flashes.erase(held);
    return hold.gone == false;
}

void RemoteServer::slotShowCleared()
{
    // what the phones hold down belongs to the show before, and the phone's
    // up of it will do nothing: the buttons go up now (a blackout flash goes
    // off; the functions are still there), a fader flash gives back only
    // what isn't of a show (the grand master, the BPM): its scenes, groups
    // and dimmers aren't these
    ShowSurface *surface = m_core->surface();
    const auto presses = m_presses;
    m_presses.clear();
    for (const QHash<QString, PressHold> &held : presses)
    {
        for (const PressHold &hold : held)
        {
            if (hold.fader == false)
            {
                for (int i = 0; i < hold.count; i++)
                    surface->release(hold.item);
                continue;
            }
            const QVariantList targets = ShowSurface::faderTargets(hold.item);
            QVariantList global, levels;
            for (int i = 0; i < targets.count() && i < hold.levels.count(); i++)
            {
                const QString kind = targets.at(i).toMap().value("target").toString();
                if (kind == "grandMaster" || kind == "bpm")
                {
                    global.append(targets.at(i));
                    levels.append(hold.levels.at(i));
                }
            }
            if (global.isEmpty() == false)
                surface->restoreLevels(QVariantMap({ { "targets", global } }), levels);
        }
    }
    for (auto socket = m_flashes.begin(); socket != m_flashes.end(); ++socket)
        for (auto it = socket.value().begin(); it != socket.value().end(); ++it)
            it.value().gone = true;
}

void RemoteServer::releaseFlashes(QWebSocket *socket)
{
    const QHash<QPair<int, int>, FlashHold> held = m_flashes.take(socket);
    for (auto it = held.cbegin(); it != held.cend(); ++it)
    {
        if (it.value().gone)
            continue;
        for (int i = 0; i < it.value().count; i++)
            m_core->playbacks()->flash(it.value().page, it.key().second, false);
    }
}

void RemoteServer::slotPlaybackPagesRenumbered(const QList<int> &newIndexes, const QHash<int, quint32> &removedSlots)
{
    Q_UNUSED(removedSlots);
    // (a flash on the current page, -1, is on whatever page that is)
    for (auto socket = m_flashes.begin(); socket != m_flashes.end(); ++socket)
    {
        for (auto it = socket.value().begin(); it != socket.value().end(); ++it)
        {
            FlashHold &hold = it.value();
            if (hold.gone || hold.page < 0 || hold.page >= newIndexes.count())
                continue;
            hold.page = newIndexes.at(hold.page);
            hold.gone = hold.page < 0;
        }
    }
}

void RemoteServer::send(QWebSocket *socket, const QVariantMap &message)
{
    socket->sendTextMessage(QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(message)).toJson(QJsonDocument::Compact)));
}

void RemoteServer::broadcast(const QVariantMap &message)
{
    const QString text = QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(message)).toJson(QJsonDocument::Compact));
    for (QWebSocket *socket : std::as_const(m_sockets))
        if (m_authorized.contains(socket))
            socket->sendTextMessage(text);
}

void RemoteServer::slotTextMessage(const QString &text)
{
    QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
    if (socket == nullptr)
        return;
    QVariantMap message = QJsonDocument::fromJson(text.toUtf8()).object().toVariantMap();

    if (message.value("cmd").toString() == "hello")
    {
        // the PIN, when there is one
        bool ok = pin().isEmpty() || message.value("pin").toString().trimmed() == pin();
        send(socket, { { "type", "auth" }, { "ok", ok }, { "pin", pin().isEmpty() == false } });
        if (ok)
        {
            m_authorized.insert(socket);
            send(socket, layoutMessage());
            send(socket, stateMessage());
        }
        return;
    }
    if (m_authorized.contains(socket) == false)
        return;

    if (trackPress(socket, message) == false)
    {
        if (trackFlash(socket, message) == false)
            return;
        handleCommand(message);
    }
    // what changed goes back at once
    slotTick();
}

void RemoteServer::slotLayoutChanged()
{
    m_layoutDirty = true;
}

void RemoteServer::slotZonesChanged()
{
    ZoneManager *zones = m_core->zones();
    for (int i = 0; i < zones->count(); i++)
    {
        Zone *zone = zones->zone(i);
        connect(zone, &Zone::nameChanged, this, &RemoteServer::slotZoneEdited, Qt::UniqueConnection);
        connect(zone, &Zone::colorChanged, this, &RemoteServer::slotZoneEdited, Qt::UniqueConnection);
        connect(zone, &Zone::functionsChanged, this, &RemoteServer::slotZoneEdited, Qt::UniqueConnection);
    }
    slotZoneEdited();
}

void RemoteServer::slotZoneEdited()
{
    // what the layout has of the groups: their order, names, colours and
    // scenes (a group says its functions changed when one of them starts or
    // stops too: the state carries that, the page isn't laid out again)
    QString groups;
    ZoneManager *zones = m_core->zones();
    for (int i = 0; i < zones->count(); i++)
    {
        Zone *zone = zones->zone(i);
        groups += QString::number(zone->id()) + '\x1f' + zone->name() + '\x1f' + zone->color().name() + '\x1f';
        for (quint32 id : zone->functionIDs())
            groups += QString::number(id) + ',';
        groups += '\x1e';
    }
    if (groups == m_groupsLaidOut)
        return;
    m_groupsLaidOut = groups;
    slotLayoutChanged();
}

void RemoteServer::slotTick()
{
    if (m_authorized.isEmpty())
        return;
    if (m_layoutDirty)
    {
        m_layoutDirty = false;
        broadcast(layoutMessage());
        m_lastState.clear();
    }
    QVariantMap state = stateMessage();
    QByteArray json = QJsonDocument(QJsonObject::fromVariantMap(state)).toJson(QJsonDocument::Compact);
    if (json == m_lastState)
        return;
    m_lastState = json;
    const QString text = QString::fromUtf8(json);
    for (QWebSocket *socket : std::as_const(m_sockets))
        if (m_authorized.contains(socket))
            socket->sendTextMessage(text);
}

/*********************************************************************
 * The SHOW for the page
 *********************************************************************/

QVariantMap RemoteServer::itemAt(int page, const QString &section, int cell) const
{
    return m_core->surface()->item(page, section, cell);
}

QString RemoteServer::itemLabel(const QVariantMap &item, const QString &sectionType) const
{
    if (item.value("label").toString().isEmpty() == false)
        return item.value("label").toString();

    auto functionName = [this](const QVariant &id) {
        Function *f = m_core->doc()->function(id.toUInt());
        return f != nullptr ? f->name() : QString();
    };

    if (sectionType == "buttons")
    {
        const QString action = item.value("action").toString();
        if (action == "scene") return functionName(item.value("functionId"));
        if (action == "next") return tr("Next");
        if (action == "previous") return tr("Previous");
        if (action == "release") return tr("Release");
        if (action == "blackout") return tr("Blackout");
        if (action == "stopAll") return tr("Stop all");
        if (action == "tap") return tr("Tap");
        if (action == "beatgo") return QStringLiteral("BeatGO");
        if (action == "map") return m_core->surface()->mapLabel(item.value("map").toMap());
        if (action == "palette") return m_core->palettes()->palette(item.value("paletteId").toInt()).value("name").toString();
        return QString();
    }

    QStringList names;
    for (const QVariant &v : ShowSurface::faderTargets(item))
    {
        const QVariantMap t = v.toMap();
        const QString target = t.value("target").toString();
        if (target == "grandMaster") names.append(tr("Grand master"));
        else if (target == "bpm") names.append(tr("BPM"));
        else if (target == "group")
        {
            Zone *zone = m_core->zones()->zoneById(t.value("zoneId").toUInt());
            names.append(zone != nullptr ? zone->name() : QString());
        }
        else if (target == "scene" || target == "sceneRun") names.append(functionName(t.value("functionId")));
        else if (target == "sceneDimmer") names.append(tr("Dimmer of %1").arg(functionName(t.value("functionId"))));
        else if (target == "sceneSpeed") names.append(tr("Speed of %1").arg(functionName(t.value("functionId"))));
        else if (target == "mixerDimmer") names.append(tr("Dimmer"));
        else if (target == "moveSpeed") names.append(tr("Movement speed"));
        else if (target == "map") names.append(m_core->mapping()->describeTarget(t.value("map").toMap()));
    }
    return names.join(" + ");
}

QVariantMap RemoteServer::describeNode(const QVariantMap &node, int page) const
{
    QVariantMap out;
    out.insert("id", node.value("id"));
    const QString split = node.value("split").toString();
    if (split == "h" || split == "v")
    {
        out.insert("split", split);
        out.insert("ratio", node.value("ratio"));
        out.insert("a", describeNode(node.value("a").toMap(), page));
        out.insert("b", describeNode(node.value("b").toMap(), page));
        return out;
    }

    const QString type = node.value("type").toString();
    const QString sid = node.value("id").toString();
    out.insert("split", "none");
    out.insert("type", type);
    out.insert("title", node.value("title"));
    for (const char *key : { "cols", "rows", "text", "fontSize", "seconds", "date", "fit", "mode", "minutes",
                             "playbackPage", "first", "showBack", "showFlash", "showCue", "background" })
        if (node.contains(key))
            out.insert(key, node.value(key));

    const QVariantMap items = node.value("items").toMap();
    QVariantMap described;
    for (auto it = items.cbegin(); it != items.cend(); ++it)
    {
        const QVariantMap item = it.value().toMap();
        QVariantMap d;
        d.insert("label", itemLabel(item, type));
        d.insert("color", item.value("color", "#3d8bfd"));
        if (type == "buttons")
        {
            d.insert("icon", item.value("icon"));
            d.insert("flash", item.value("mode").toString() == "flash");
            // a scene: its rhythm bar follows its steps
            if (item.value("action").toString() == "scene")
                d.insert("functionId", item.value("functionId"));
            if (item.value("image").toString().isEmpty() == false)
                d.insert("image", QString("/image?page=%1&section=%2&cell=%3").arg(page).arg(sid, it.key()));
        }
        else
        {
            const QVariantList targets = ShowSurface::faderTargets(item);
            d.insert("bpm", targets.count() == 1 && targets.first().toMap().value("target").toString() == "bpm");
            // a speed: x0.1 to x10 (x1 in the middle), when all its targets
            // are speeds (of the movement, of scenes, scenes played at a
            // speed), as on the SHOW; one that plays a scene first shows it
            // stopped at the end that stops it
            d.insert("speed", ShowSurface::isSpeed(item));
            d.insert("run", targets.isEmpty() == false && targets.first().toMap().value("target").toString() == "sceneRun");
            d.insert("flash", type == "faders" && item.value("flash", true).toBool());
            d.insert("encoder", item.value("encoder", false));
            d.insert("step", item.value("step", 2));
            // the range of the fader (0..1): the text shows the value while it moves
            qreal min, max;
            bool invert;
            ControlMapper::rangeOf(item, min, max, invert);
            d.insert("min", min);
            d.insert("max", max);
            d.insert("invert", invert);
        }
        described.insert(it.key(), d);
    }
    out.insert("items", described);

    // the groups and their scenes
    auto zoneInfo = [this](Zone *zone) {
        QVariantList scenes;
        for (const QVariant &v : zone->functionsInfo())
        {
            const QVariantMap f = v.toMap();
            scenes.append(QVariantMap({ { "id", f.value("id") }, { "name", f.value("name") } }));
        }
        return QVariantMap({ { "id", zone->id() }, { "name", zone->name() }, { "color", zone->color().name() },
                             { "scenes", scenes } });
    };
    if (type == "group")
    {
        if (Zone *zone = m_core->zones()->zoneById(node.value("zoneId").toUInt()))
            out.insert("zones", QVariantList({ zoneInfo(zone) }));
    }
    else if (type == "groups")
    {
        QVariantList zones;
        for (int i = 0; i < m_core->zones()->count(); i++)
            zones.append(zoneInfo(m_core->zones()->zone(i)));
        out.insert("zones", zones);
    }
    else if (type == "image" && node.value("image").toString().isEmpty() == false)
    {
        out.insert("image", QString("/image?page=%1&section=%2").arg(page).arg(sid));
    }
    return out;
}

QVariantMap RemoteServer::layoutMessage() const
{
    QVariantList pages;
    const QVariantList surfacePages = m_core->surface()->pages();
    for (int p = 0; p < surfacePages.count(); p++)
    {
        const QVariantMap page = surfacePages.at(p).toMap();
        pages.append(QVariantMap({ { "name", page.value("name") },
                                   { "root", describeNode(page.value("root").toMap(), p) } }));
    }

    // the playbacks, when they hold something: a page of their own that
    // follows the current playback page (as the Playbacks page of the computer)
    PlaybackManager *playbacks = m_core->playbacks();
    if (playbacks->hasAssigned())
    {
        const QVariantMap root({ { "id", "pb" }, { "split", "none" }, { "type", "playbacks" },
                                 { "title", QString() }, { "cols", playbacks->slotsPerPage() },
                                 { "playbackPage", -1 }, { "first", 0 }, { "showBack", true },
                                 { "showFlash", true }, { "showCue", true }, { "items", QVariantMap() } });
        pages.append(QVariantMap({ { "name", tr("Playbacks") }, { "root", root } }));
    }
    return { { "type", "layout" }, { "title", m_core->showTitle() }, { "theme", m_core->settings()->theme() }, { "pages", pages } };
}

QVariantMap RemoteServer::stepsState() const
{
    QVariantMap steps;
    const QHash<quint32, int> all = m_core->steps();
    for (auto it = all.cbegin(); it != all.cend(); ++it)
        steps.insert(QString::number(it.key()), it.value());
    return steps;
}

QVariantMap RemoteServer::playbacksState() const
{
    PlaybackManager *playbacks = m_core->playbacks();

    // the pages shown: the current one, and the ones the sections are fixed on
    QSet<int> shown({ playbacks->currentPage() });
    std::function<void(const QVariantMap &)> walk = [&](const QVariantMap &node) {
        const QString split = node.value("split").toString();
        if (split == "h" || split == "v")
        {
            walk(node.value("a").toMap());
            walk(node.value("b").toMap());
        }
        else if (node.value("type").toString() == "playbacks" && node.value("playbackPage", -1).toInt() >= 0)
        {
            shown.insert(node.value("playbackPage").toInt());
        }
    };
    for (const QVariant &page : m_core->surface()->pages())
        walk(page.toMap().value("root").toMap());

    // the playbacks that hold something (the others are empty on the phone)
    QVariantMap map;
    for (int page : std::as_const(shown))
    {
        if (page < 0 || page >= playbacks->pageCount())
            continue;
        for (int slot = 0; slot < PlaybackManager::maxSlots(); slot++)
        {
            const QVariantMap info = playbacks->slotInfo(page, slot);
            if (info.value("empty").toBool())
                continue;
            const QString color = info.value("color").toString();
            map.insert(QString("%1/%2").arg(page).arg(slot), QVariantMap({
                { "name", info.value("name") },
                { "type", info.value("type") },
                { "color", color },
                { "state", info.value("state") },
                { "level", info.value("level") },
                { "flashing", info.value("flashing") },
                { "cueNumber", info.value("cueNumber") },
                { "cueName", info.value("cueName") },
                { "nextNumber", info.value("nextNumber") },
                { "nextName", info.value("nextName") },
                // (by 5 %: the state is sent only when something changed)
                { "progress", qRound(info.value("progress").toDouble() * 20) / 20.0 } }));
        }
    }
    return map;
}

QVariantMap RemoteServer::stateMessage() const
{
    ShowSurface *surface = m_core->surface();
    QVariantMap items, colours, positions;

    // the controls of every page: lit buttons, levels of faders and dials
    std::function<void(const QVariantMap &, int)> walk = [&](const QVariantMap &node, int page) {
        const QString split = node.value("split").toString();
        if (split == "h" || split == "v")
        {
            walk(node.value("a").toMap(), page);
            walk(node.value("b").toMap(), page);
            return;
        }
        const QString type = node.value("type").toString();
        const QString sid = node.value("id").toString();
        const QString prefix = QString("%1/%2").arg(page).arg(sid);
        const QVariantMap sectionItems = node.value("items").toMap();
        for (auto it = sectionItems.cbegin(); it != sectionItems.cend(); ++it)
        {
            const QVariantMap item = it.value().toMap();
            if (type == "buttons")
                items.insert(prefix + "/" + it.key(), QVariantMap({ { "active", surface->isActive(item) } }));
            else if (type == "faders" || type == "dials")
                items.insert(prefix + "/" + it.key(), QVariantMap({ { "level", surface->level(item) }, { "value", surface->value(item) } }));
        }
        if (type == "colour")
        {
            QColor c = surface->colour(page, sid);
            colours.insert(prefix, c.isValid() ? c.name() : QString());
        }
        else if (type == "xy")
        {
            positions.insert(prefix, surface->xy(page, sid));
        }
    };
    const QVariantList pages = surface->pages();
    for (int p = 0; p < pages.count(); p++)
        walk(pages.at(p).toMap().value("root").toMap(), p);

    // the groups: what plays, the masters
    QVariantMap zones;
    for (int i = 0; i < m_core->zones()->count(); i++)
    {
        Zone *zone = m_core->zones()->zone(i);
        QVariantList active;
        for (const QVariant &v : zone->functionsInfo())
            if (v.toMap().value("active").toBool())
                active.append(v.toMap().value("id"));
        zones.insert(QString::number(zone->id()), QVariantMap({ { "active", active }, { "master", zone->master() } }));
    }

    // what plays now
    QStringList running;
    for (Function *f : m_core->doc()->functions())
        if (f->isVisible() && f->isRunning())
            running.append(f->name());

    return {
        { "type", "state" },
        { "blackout", m_core->blackout() },
        { "grandMaster", m_core->grandMaster() / 255.0 },
        { "bpm", m_core->bpm() },
        // GO and Stop of the scene buttons, and the step of each scene with steps that plays
        { "pressMode", m_core->pressMode() },
        { "steps", stepsState() },
        { "items", items },
        { "colours", colours },
        { "xy", positions },
        { "zones", zones },
        { "running", running },
        { "playbacks", playbacksState() },
        { "playbackPage", m_core->playbacks()->currentPage() },
        { "playbackPages", m_core->playbacks()->pageCount() },
        { "playbackPageNames", m_core->playbacks()->pageNames() }
    };
}

void RemoteServer::handleCommand(const QVariantMap &message)
{
    ShowSurface *surface = m_core->surface();
    const QString cmd = message.value("cmd").toString();
    const int page = message.value("page").toInt();
    const QString section = message.value("section").toString();
    const int cell = message.value("cell").toInt();

    if (cmd == "press" || cmd == "release")
    {
        QVariantMap item = itemAt(page, section, cell);
        if (item.isEmpty())
            return;
        if (cmd == "press")
            surface->press(item);
        else
            surface->release(item);
    }
    else if (cmd == "level")
    {
        QVariantMap item = itemAt(page, section, cell);
        if (item.isEmpty() == false)
            surface->setLevel(item, qBound(0.0, message.value("value").toDouble(), 1.0));
    }
    else if (cmd == "encoder")
    {
        // an incremental wheel: notches of its step
        QVariantMap item = itemAt(page, section, cell);
        if (item.isEmpty() == false)
        {
            surface->stepLevel(item, message.value("notches").toInt());
        }
    }
    else if (cmd == "colour")
    {
        surface->setColour(page, section, QColor(message.value("color").toString()));
    }
    else if (cmd == "xy")
    {
        surface->setXY(page, section, message.value("x").toDouble(), message.value("y").toDouble());
    }
    else if (cmd == "releaseSection")
    {
        surface->releaseSection(page, section);
    }
    else if (cmd == "scene")
    {
        if (Zone *zone = m_core->zones()->zoneById(message.value("zone").toUInt()))
            zone->trigger(message.value("id").toUInt());
    }
    else if (cmd == "groupMaster")
    {
        if (Zone *zone = m_core->zones()->zoneById(message.value("zone").toUInt()))
            zone->setMaster(qBound(0.0, message.value("value").toDouble(), 1.0));
    }
    else if (cmd == "grandMaster")
    {
        m_core->setGrandMaster(qRound(qBound(0.0, message.value("value").toDouble(), 1.0) * 255));
    }
    else if (cmd == "global")
    {
        const QString action = message.value("action").toString();
        if (action == "blackout")
            m_core->setBlackout(!m_core->blackout());
        else if (action == "stopAll")
            m_core->stopAll();
        else if (action == "tap")
            m_core->tapTempo();
        else if (action == "beatgo")
            m_core->drive()->beatGo();
        // (always GO or Stop: pressing the one that is on keeps it)
        else if (action == "pressGo" || action == "pressStop")
            m_core->setPressMode(action == "pressGo" ? "go" : "stop");
    }
    else if (cmd == "playback")
    {
        // the phone sends the page it shows (a press acts on what it sees)
        PlaybackManager *playbacks = m_core->playbacks();
        const QString action = message.value("action").toString();
        const int pbPage = message.value("pbPage", -1).toInt();
        const int slot = message.value("slot").toInt();
        if (slot < 0 || slot >= PlaybackManager::maxSlots())
            return;
        if (action == "go")
            playbacks->go(pbPage, slot);
        else if (action == "back")
            playbacks->back(pbPage, slot);
        else if (action == "pauseBack")
            playbacks->pauseBack(pbPage, slot);
        else if (action == "goto")
            playbacks->goTo(pbPage, slot, message.value("cue").toInt());
        else if (action == "flash")
            playbacks->flash(pbPage, slot, message.value("down").toBool());
        else if (action == "release")
            playbacks->release(pbPage, slot);
        else if (action == "level")
            playbacks->setLevel(pbPage, slot, qBound(0.0, message.value("value").toDouble(), 1.0));
        else if (action == "select")
            playbacks->select(pbPage, slot);
        else if (action == "pageNext")
            playbacks->nextPage();
        else if (action == "pagePrevious")
            playbacks->previousPage();
        else if (action == "page" && pbPage >= 0 && pbPage < playbacks->pageCount())
            playbacks->setCurrentPage(pbPage);
    }
}

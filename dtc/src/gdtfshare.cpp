/*
  DMX Total Control
  gdtfshare.cpp

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

#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCollator>
#include <QFileInfo>
#include <QSaveFile>
#include <QSettings>
#include <QTimeZone>
#include <QThread>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QUrl>

#include <memory>
#include <vector>

#include "gdtfshare.h"
#include "gdtfimporter.h"
#include "fixturelibrary.h"
#include "qlcconfig.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#endif

#define SHARE_API_URL QStringLiteral("https://gdtf-share.com/apis/public/")
// longer bodies are a list or a file, never an error
#define SHORT_ANSWER 4096
#define KSettingUser     QStringLiteral("gdtfShare/user")
// (base64 of what Windows encrypted, never the password itself)
#define KSettingPassword QStringLiteral("gdtfShare/password")
#define LIST_ENDPOINT    QStringLiteral("getList.php")
#define FILE_ENDPOINT    QStringLiteral("downloadFile.php")
// the list isn't asked for again by the program within this time
#define LIST_MAX_AGE     (20 * 3600)
// the check of the start: when it last had a list, and the updates told
#define KSettingLastCheck QStringLiteral("gdtfShare/lastCheck")
#define KSettingAnnounced QStringLiteral("gdtfShare/announced")
// (after those of the program, at 5 s, and of the Open Fixture Library, at 8 s)
#define STARTUP_DELAY    11000
// a download older than this that the record doesn't keep was left behind
#define LEFT_BEHIND_AGE  (24 * 3600)

/* $body as a JSON object: false when it isn't one */
static bool readObject(const QByteArray &body, QJsonObject *object)
{
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(body, &error);
    if (error.error != QJsonParseError::NoError || doc.isObject() == false)
        return false;
    *object = doc.object();
    return true;
}

/* A short JSON answer (an error, a sign-in): its object, else an empty one */
static QJsonObject shortAnswer(const QByteArray &body)
{
    QJsonObject object;
    if (body.size() <= SHORT_ANSWER)
        readObject(body, &object);
    return object;
}

/* GDTF-Share says no: "result" is false (a bool, a number or a text) */
static bool refused(const QJsonObject &answer)
{
    return answer.contains("result") && answer.value("result").toVariant().toBool() == false;
}

static int httpStatus(QNetworkReply *reply)
{
    return reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
}

/* The session isn't valid (any more): a 401, or GDTF-Share says so */
static bool unauthorized(QNetworkReply *reply, const QByteArray &body)
{
    if (httpStatus(reply) == 401)
        return true;
    const QJsonObject answer = shortAnswer(body);
    return refused(answer) && answer.value("error").toVariant().toString().contains("Unauthorized", Qt::CaseInsensitive);
}

GdtfShare::GdtfShare(QObject *parent)
    : QObject(parent)
    , m_network(nullptr)
    , m_state(Idle)
    , m_baseUrl(SHARE_API_URL)
    , m_testUrl(false)
    , m_signedIn(false)
    , m_signedOut(false)
    , m_manual(false)
    , m_progress(0)
    , m_listRevision(0)
    , m_listFetching(false)
    , m_listOffline(false)
    , m_downloadCount(0)
    , m_downloadsOver(0)
    , m_downloadBytes(0)
    , m_downloadedBytes(0)
    , m_downloadSizes(true)
    , m_startupDelay(STARTUP_DELAY)
    , m_check(Check::None)
{
    // nothing is asked here: the network starts with the first request, and
    // the saved list is read when the browser opens (or at the start, see
    // checkOnStartup())
    connect(this, &GdtfShare::indexChanged, this, [this]() {
        m_library.read = false;
        findUpdates();
    });
}

GdtfShare::~GdtfShare()
{
    // (its manager deletes it)
    if (m_reply != nullptr)
    {
        m_reply->disconnect(this);
        m_reply->abort();
    }
    // (a download cut short leaves nothing, and neither do those of its
    // batch that arrived whole, never given to the library to convert)
    closePart(false);
    QStringList arrived;
    for (const Download &download : std::as_const(m_downloads))
        arrived.append(download.path);
    discardDownloads(arrived);
    // a list being read finishes first: its answer comes to this object
    if (m_listThread != nullptr)
        m_listThread->wait();
}

int GdtfShare::state() const
{
    return m_state;
}

bool GdtfShare::busy() const
{
    return m_state != Idle;
}

bool GdtfShare::signedIn() const
{
    return m_signedIn;
}

QString GdtfShare::user() const
{
    return m_user.isEmpty() ? QSettings().value(KSettingUser).toString() : m_user;
}

bool GdtfShare::remembered() const
{
    return QSettings().contains(KSettingPassword);
}

bool GdtfShare::sessionPossible() const
{
    return m_signedIn || m_password.isEmpty() == false || (m_signedOut == false && remembered());
}

QString GdtfShare::error() const
{
    return m_error;
}

QString GdtfShare::notice() const
{
    return m_notice;
}

qreal GdtfShare::progress() const
{
    return m_progress;
}

bool GdtfShare::allowedUrl(const QString &url, bool testOverride)
{
    if (testOverride)
        return true;
    const QUrl address(url);
    return address.isValid() && address.scheme() == QLatin1String("https") && address.host().isEmpty() == false;
}

void GdtfShare::setBaseUrl(const QString &url)
{
    m_baseUrl = url.endsWith('/') ? url : url + '/';
    m_testUrl = true;
}

QString GdtfShare::baseUrl() const
{
    return m_baseUrl;
}

/*********************************************************************
 * The saved password
 *********************************************************************/

#ifdef Q_OS_WIN
/* The fixed bytes mixed in by Windows: a blob of another program doesn't
 * decrypt here */
static QByteArray secretEntropy()
{
    return QByteArrayLiteral("DMXTotalControl/GDTF-Share/1");
}

static DATA_BLOB dataBlob(const QByteArray &bytes)
{
    DATA_BLOB blob;
    blob.cbData = DWORD(bytes.size());
    blob.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(bytes.constData()));
    return blob;
}
#endif

bool GdtfShare::canRemember()
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

QByteArray GdtfShare::protectSecret(const QByteArray &plain)
{
#ifdef Q_OS_WIN
    if (plain.isEmpty())
        return QByteArray();
    const QByteArray entropyBytes = secretEntropy();
    DATA_BLOB in = dataBlob(plain);
    DATA_BLOB entropy = dataBlob(entropyBytes);
    DATA_BLOB out = { 0, nullptr };
    if (CryptProtectData(&in, L"DMX Total Control GDTF-Share", &entropy, nullptr, nullptr,
                         CRYPTPROTECT_UI_FORBIDDEN, &out) == FALSE)
        return QByteArray();
    const QByteArray blob(reinterpret_cast<const char *>(out.pbData), qsizetype(out.cbData));
    LocalFree(out.pbData);
    return blob;
#else
    Q_UNUSED(plain)
    return QByteArray();
#endif
}

QByteArray GdtfShare::unprotectSecret(const QByteArray &blob)
{
#ifdef Q_OS_WIN
    if (blob.isEmpty())
        return QByteArray();
    const QByteArray entropyBytes = secretEntropy();
    DATA_BLOB in = dataBlob(blob);
    DATA_BLOB entropy = dataBlob(entropyBytes);
    DATA_BLOB out = { 0, nullptr };
    if (CryptUnprotectData(&in, nullptr, &entropy, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out) == FALSE)
        return QByteArray();
    const QByteArray plain(reinterpret_cast<const char *>(out.pbData), qsizetype(out.cbData));
    // (the plain text Windows gave is wiped before it is freed)
    SecureZeroMemory(out.pbData, out.cbData);
    LocalFree(out.pbData);
    return plain;
#else
    Q_UNUSED(blob)
    return QByteArray();
#endif
}

QString GdtfShare::passwordForSession(bool *unreadable)
{
    *unreadable = false;
    // (signed out: not the saved one until the user signs in again)
    if (m_password.isEmpty() == false || m_signedOut || remembered() == false)
        return m_password;

    const QByteArray::FromBase64Result saved = QByteArray::fromBase64Encoding(
        QSettings().value(KSettingPassword).toString().toLatin1(), QByteArray::AbortOnBase64DecodingErrors);
    QByteArray plain = saved ? unprotectSecret(saved.decoded) : QByteArray();
    if (plain.isEmpty())
    {
        // copied with the settings to another computer or Windows user
        forgetPassword();
        m_notice = tr("The saved password can't be read on this computer: sign in again");
        emit changed();
        *unreadable = true;
        return QString();
    }
    const QString password = QString::fromUtf8(plain);
    plain.fill('\0');
    return password;
}

void GdtfShare::saveAccount(const QString &user, const QString &password, bool remember)
{
    QSettings settings;
    settings.setValue(KSettingUser, user);
    const QByteArray blob = remember && canRemember() ? protectSecret(password.toUtf8()) : QByteArray();
    if (blob.isEmpty())
        settings.remove(KSettingPassword);
    else
        settings.setValue(KSettingPassword, QString::fromLatin1(blob.toBase64()));
    emit changed();
}

void GdtfShare::forgetPassword()
{
    QSettings().remove(KSettingPassword);
    emit changed();
}

/*********************************************************************
 * Requests
 *********************************************************************/

QNetworkAccessManager *GdtfShare::network()
{
    // (its cookie jar keeps the session, in memory only)
    if (m_network == nullptr)
        m_network = new QNetworkAccessManager(this);
    return m_network;
}

QNetworkRequest GdtfShare::request(const QString &endpoint, const QUrlQuery &query, bool download) const
{
    QUrl url(m_baseUrl + endpoint);
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("DMXTotalControl/%1").arg(QString(APPVERSION).section(' ', 0, 0)));
    request.setRawHeader("Accept", download ? "application/octet-stream" : "application/json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    // given up after this long without data (a file can be large)
    request.setTransferTimeout(download ? 60000 : 30000);
    return request;
}

void GdtfShare::fetchList(bool manual)
{
    call(Fetching, LIST_ENDPOINT, QUrlQuery(), manual, [this](const QByteArray &body) {
        QJsonObject list;
        if (readObject(body, &list) == false)
            return tr("GDTF-Share sent an answer that can't be read");
        emit listReceived(body);
        return QString();
    });
}

void GdtfShare::call(State kind, const QString &endpoint, const QUrlQuery &query, bool manual, const Handler &done,
                     const Failure &failed)
{
    Call call;
    call.kind = kind;
    call.endpoint = endpoint;
    call.query = query;
    call.manual = manual;
    call.done = done;
    call.failed = failed;
    enqueue(call);
}

void GdtfShare::enqueue(const Call &call)
{
    m_queue.append(call);
    runNext();
}

void GdtfShare::signIn(const QString &user, const QString &password, bool remember)
{
    Call call;
    call.kind = SigningIn;
    call.manual = true;
    call.user = user.trimmed();
    call.password = password;
    call.remember = remember;
    // before the requests waiting their turn: they may need it
    m_queue.prepend(call);
    runNext();
}

void GdtfShare::runNext()
{
    if (m_state != Idle || m_queue.isEmpty())
        return;

    const Call call = m_queue.takeFirst();
    m_manual = call.manual;
    m_failed = call.failed;
    if (call.manual)
    {
        // (a download that failed in a batch is still told at its end)
        if (call.follows == false)
            m_error.clear();
        m_notice.clear();
    }
    if (call.kind == SigningIn)
        login(call.user, call.password, call);
    else
        start(call);
}

void GdtfShare::start(const Call &call)
{
    if (m_signedIn)
    {
        send(call);
        return;
    }
    // not signed in yet
    signInAgain(call);
}

void GdtfShare::signInAgain(const Call &call)
{
    bool unreadable = false;
    const QString password = passwordForSession(&unreadable);
    if (password.isEmpty() == false)
    {
        login(user(), password, call);
        return;
    }
    waitForLogin(call, unreadable ? m_notice : tr("Sign in to GDTF-Share to continue"));
}

void GdtfShare::send(const Call &call)
{
    if (allowedUrl(m_baseUrl, m_testUrl) == false)
    {
        fail(tr("GDTF-Share is only used over a secure connection (https)"));
        return;
    }

    const bool download = call.kind == Downloading;
    const bool toFile = call.file.isEmpty() == false;
    if (toFile && openPart(call.file) == false)
    {
        fail(tr("Unable to write %1").arg(QDir::toNativeSeparators(call.file)));
        return;
    }
    if (call.manual && call.notice.isEmpty() == false)
        m_notice = call.notice;
    setState(call.kind);
    QNetworkReply *reply = network()->get(request(call.endpoint, call.query, download));
    m_reply = reply;
    if (download)
    {
        connect(reply, &QNetworkReply::downloadProgress, this,
                [this, toFile](qint64 received, qint64 total) { downloadProgress(received, total, toFile); });
    }
    // (written as it comes: a file can be large. One that can't be written stops)
    if (toFile)
    {
        connect(reply, &QNetworkReply::readyRead, this, [this, reply]() {
            const QByteArray data = reply->readAll();
            if (m_part != nullptr && m_part->write(data) != data.size())
                reply->abort();
        });
    }
    connect(reply, &QNetworkReply::finished, this, [this, reply, call]() { finishCall(reply, call); });
    // (last: what it tells may cancel it)
    if (download)
        downloadProgress(0, 0, toFile);
}

/* The first bytes of a file: all of it when it is a short answer */
static QByteArray headOf(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.read(SHORT_ANSWER + 1) : QByteArray();
}

static QString partOf(const QString &file)
{
    return file + QStringLiteral(".part");
}

void GdtfShare::finishCall(QNetworkReply *reply, Call call)
{
    reply->deleteLater();
    m_reply = nullptr;
    QByteArray body = reply->readAll();
    // (the notice of the request goes with it)
    if (call.notice.isEmpty() == false && m_notice == call.notice)
        m_notice.clear();

    // a download written to its file: its first bytes tell what it is
    if (m_part != nullptr)
    {
        const bool written = m_part->error() == QFileDevice::NoError && m_part->write(body) == body.size()
                             && m_part->flush();
        closePart(written);
        if (written == false)
        {
            fail(tr("Unable to write %1").arg(QDir::toNativeSeparators(call.file)));
            return;
        }
        body = headOf(partOf(call.file));
    }

    if (unauthorized(reply, body))
    {
        // the session expired (2 h without use): signed in again once, and
        // the same request again. Refused right after a sign-in: no loop
        if (call.file.isEmpty() == false)
            QFile::remove(partOf(call.file));
        m_signedIn = false;
        emit changed();
        if (call.retried)
        {
            fail(tr("GDTF-Share refused the session: sign in again"));
            return;
        }
        signInAgain(call);
        return;
    }
    if (reply->error() != QNetworkReply::NoError || httpStatus(reply) != 200 || refused(shortAnswer(body)))
    {
        if (call.file.isEmpty() == false)
            QFile::remove(partOf(call.file));
        answerError(reply, body);
        return;
    }

    // (its answer is used: nothing more to tell if it fails)
    m_failed = nullptr;
    const QString error = call.done ? call.done(body) : QString();
    if (error.isEmpty())
        finish();
    else
        fail(error);
}

/*********************************************************************
 * The session
 *********************************************************************/

void GdtfShare::login(const QString &user, const QString &password, const Call &call)
{
    // the password only goes to GDTF-Share, encrypted
    if (allowedUrl(m_baseUrl, m_testUrl) == false)
    {
        fail(tr("GDTF-Share is only used over a secure connection (https)"));
        return;
    }

    setState(SigningIn);
    QNetworkRequest request = this->request(QStringLiteral("login.php"), QUrlQuery(), false);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArray("application/json"));
    const QJsonObject account({ { "user", user }, { "password", password } });
    QNetworkReply *reply = network()->post(request, QJsonDocument(account).toJson(QJsonDocument::Compact));
    m_reply = reply;
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, user, password, call]() { finishLogin(reply, user, password, call); });
}

void GdtfShare::finishLogin(QNetworkReply *reply, const QString &user, const QString &password, Call call)
{
    reply->deleteLater();
    m_reply = nullptr;
    const QByteArray body = reply->readAll();
    const int status = httpStatus(reply);
    QJsonObject answer;
    const bool json = readObject(body, &answer);
    const bool answered = status == 200 && reply->error() == QNetworkReply::NoError;

    if (answered && json && answer.value("result").toVariant().toBool())
    {
        m_signedIn = true;
        m_user = user;
        m_password = password;
        emit changed();
        if (call.kind != SigningIn)
        {
            call.retried = true;
            send(call);
            return;
        }
        // the account, for the next times (and the saved password can be
        // used again after a sign-out)
        m_signedOut = false;
        saveAccount(user, password, call.remember);
        // what waited for the sign-in, before what waits its turn
        for (int i = m_pending.count() - 1; i >= 0; i--)
        {
            Call waiting = m_pending.at(i);
            waiting.retried = true;
            m_queue.prepend(waiting);
        }
        m_pending.clear();
        finish();
        return;
    }
    if (status == 401 || (answered && refused(answer)))
    {
        // (the request waits for the right password)
        m_signedIn = false;
        m_password.clear();
        // a sign-in without asking used the saved password, when there is
        // one (that of the session is the same): it was changed on the
        // website, and is forgotten
        if (call.kind != SigningIn && remembered())
            forgetPassword();
        waitForLogin(call, tr("Wrong user name or password"));
        return;
    }
    if (answered && json == false)
    {
        fail(tr("GDTF-Share sent an answer that can't be read"));
        return;
    }
    answerError(reply, body);
}

void GdtfShare::waitForLogin(const Call &call, const QString &message)
{
    // $call and the requests waiting their turn need the sign-in too (a
    // sign-in waiting its turn stays there). What the program does on its
    // own doesn't ask: it is dropped
    QList<Call> calls;
    if (call.kind != SigningIn)
        calls.append(call);
    // ($call is in calls: it waits there, or is dropped)
    m_failed = nullptr;
    QList<Call> signIns;
    for (const Call &queued : std::as_const(m_queue))
        (queued.kind == SigningIn ? signIns : calls).append(queued);
    m_queue = signIns;
    bool ask = m_manual;
    QList<Failure> dropped;
    for (const Call &waiting : std::as_const(calls))
    {
        if (waiting.manual)
        {
            m_pending.append(waiting);
            ask = true;
        }
        else if (waiting.failed)
        {
            dropped.append(waiting.failed);
        }
    }

    if (ask)
    {
        m_error = message;
        emit loginNeeded();
    }
    for (const Failure &failed : std::as_const(dropped))
        failed(false);
    finish();
}

QList<GdtfShare::Failure> GdtfShare::dropCalls()
{
    QList<Failure> failures;
    if (m_failed)
        failures.append(m_failed);
    for (const Call &call : std::as_const(m_queue))
        if (call.failed)
            failures.append(call.failed);
    for (const Call &call : std::as_const(m_pending))
        if (call.failed)
            failures.append(call.failed);
    m_failed = nullptr;
    m_queue.clear();
    m_pending.clear();
    dropReply();
    return failures;
}

void GdtfShare::signOut(bool forget)
{
    const QList<Failure> dropped = dropCalls();
    // a new cookie jar: the session goes with the old one
    if (m_network != nullptr)
        m_network->setCookieJar(new QNetworkCookieJar);
    m_password.clear();
    m_signedIn = false;
    // (no sign-in without asking until the user signs in: the saved
    // password stays for the next runs, unless $forget)
    m_signedOut = true;
    if (forget && remembered())
        forgetPassword();
    m_error.clear();
    m_notice.clear();
    setState(Idle);
    for (const Failure &failed : dropped)
        failed(false);
}

void GdtfShare::cancel()
{
    bool download = m_state == Downloading;
    for (const Call &call : std::as_const(m_queue))
        download = download || call.kind == Downloading;
    for (const Call &call : std::as_const(m_pending))
        download = download || call.kind == Downloading;

    // (a sign-in dismissed: what it told goes with what waited for it. A
    // transfer stopped keeps the error of a file before it in its batch)
    const bool transferring = m_state == Downloading || m_state == Fetching;
    const QList<Failure> dropped = dropCalls();
    if (transferring == false)
        m_error.clear();
    if (download)
        m_notice = tr("Download cancelled");
    setState(Idle);
    for (const Failure &failed : dropped)
        failed(false);
}

/*********************************************************************
 * The end of a request
 *********************************************************************/

void GdtfShare::answerError(QNetworkReply *reply, const QByteArray &body)
{
    const int status = httpStatus(reply);
    if (status == 0)
    {
        fail(tr("GDTF-Share could not be reached: %1").arg(reply->errorString()), true);
        return;
    }
    const QString error = shortAnswer(body).value("error").toVariant().toString().trimmed();
    if (error.isEmpty() == false)
    {
        fail(tr("GDTF-Share answered with an error: %1").arg(error));
        return;
    }
    if (status != 200)
    {
        fail(tr("GDTF-Share answered with an error: %1").arg(QStringLiteral("HTTP %1").arg(status)));
        return;
    }
    // (an answer cut short)
    if (reply->error() != QNetworkReply::NoError)
    {
        fail(tr("GDTF-Share could not be reached: %1").arg(reply->errorString()), true);
        return;
    }
    fail(tr("GDTF-Share sent an answer that can't be read"));
}

void GdtfShare::fail(const QString &message, bool unreachable)
{
    // what the program does on its own never shows an error
    if (m_manual)
        m_error = message;
    const Failure failed = m_failed;
    m_failed = nullptr;
    if (failed)
        failed(unreachable);
    finish();
}

void GdtfShare::finish()
{
    setState(Idle);
    runNext();
}

void GdtfShare::setState(State state)
{
    m_state = state;
    emit changed();
}

void GdtfShare::dropReply()
{
    // (a download cut short leaves nothing)
    closePart(false);
    if (m_reply == nullptr)
        return;
    // (aborted without its answer: nothing is done with it)
    m_reply->disconnect(this);
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = nullptr;
}

/*********************************************************************
 * The list
 *********************************************************************/

/* A text GDTF-Share gave (also a number) */
static QString textOf(const QJsonValue &value)
{
    return value.toVariant().toString().trimmed();
}

/* A number GDTF-Share gave: as a number or a text, 0 when it isn't one */
static qint64 numberOf(const QJsonValue &value)
{
    const QVariant variant = value.toVariant();
    bool ok = false;
    const qint64 number = variant.toLongLong(&ok);
    if (ok)
        return number;
    const double real = variant.toDouble(&ok);
    return ok ? qint64(real) : 0;
}

/* A time GDTF-Share gave, in seconds since 1970: a number or a text of
 * seconds (or milliseconds), or a date. 0 when there is none */
static qint64 secondsOf(const QJsonValue &value)
{
    const QString text = textOf(value);
    if (text.isEmpty())
        return 0;
    qint64 seconds = numberOf(value);
    if (seconds == 0)
    {
        // a date, in UTC when it says nothing
        QDateTime time = QDateTime::fromString(QString(text).replace(' ', 'T'), Qt::ISODate);
        if (time.isValid() && time.timeSpec() == Qt::LocalTime)
            time = QDateTime(time.date(), time.time(), QTimeZone::UTC);
        return time.isValid() ? qMax<qint64>(time.toSecsSinceEpoch(), 0) : 0;
    }
    if (seconds > 100000000000LL)
        seconds /= 1000;
    return qMax<qint64>(seconds, 0);
}

/* The uuid of a fixture type, as the list keeps it: without braces, in
 * capitals, and empty when it is all zeros (none) */
static QString uuidKey(const QString &text)
{
    QString uuid = text;
    uuid = uuid.remove('{').remove('}').trimmed().toUpper();
    for (const QChar c : std::as_const(uuid))
    {
        if (c != '0' && c != '-')
            return uuid;
    }
    return QString();
}

/* The fixture type of an upload: its uuid, or its rid when it has none */
static QString typeKey(const GdtfShare::Entry &entry)
{
    return entry.uuid.isEmpty() ? QStringLiteral("rid:%1").arg(entry.rid) : entry.uuid;
}

/* Uploaded by the manufacturer ("Manuf."), not by a user */
static bool byManufacturer(const GdtfShare::Entry &entry)
{
    return entry.uploader.startsWith(QLatin1String("manuf"), Qt::CaseInsensitive);
}

static QDateTime timeOf(qint64 seconds)
{
    return seconds > 0 ? QDateTime::fromSecsSinceEpoch(seconds, QTimeZone::UTC) : QDateTime();
}

/* An upload, for QML */
static QVariantMap entryMap(const GdtfShare::Entry &entry)
{
    QVariantList modes;
    for (const QPair<QString, int> &mode : entry.modes)
        modes.append(QVariantMap({ { "name", mode.first }, { "channels", mode.second } }));

    QVariantMap map;
    map.insert("uuid", typeKey(entry));
    map.insert("rid", entry.rid);
    map.insert("fixture", entry.fixture);
    map.insert("manufacturer", entry.manufacturer);
    map.insert("revision", entry.revision);
    map.insert("uploader", byManufacturer(entry) ? QStringLiteral("manufacturer") : QStringLiteral("user"));
    map.insert("creator", entry.creator);
    map.insert("rating", entry.rating);
    map.insert("version", entry.version);
    map.insert("created", timeOf(entry.created));
    map.insert("modified", timeOf(entry.modified));
    map.insert("filesize", entry.filesize);
    map.insert("modes", modes);
    return map;
}

/* GDTF-Share's list, kept as it came */
static bool saveList(const QString &path, const QByteArray &body)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (file.open(QIODevice::WriteOnly) == false || file.write(body) != body.size())
        return false;
    return file.commit();
}

static QString downloadingText()
{
    return GdtfShare::tr("Downloading the list of GDTF-Share…");
}

QString GdtfShare::fold(const QString &text)
{
    // (the accents are marks of their own once decomposed)
    const QString decomposed = text.toCaseFolded().normalized(QString::NormalizationForm_D);
    QString folded;
    folded.reserve(decomposed.size());
    for (const QChar c : decomposed)
    {
        if (c.isMark() == false)
            folded.append(c);
    }
    return folded;
}

QList<GdtfShare::Entry> GdtfShare::parseList(const QByteArray &json, QString *error, qint64 *timestamp)
{
    error->clear();
    *timestamp = 0;
    // (its documentation and its answers don't agree on the types: every
    // value is read as a number or a text)
    QJsonObject answer;
    if (readObject(json, &answer) == false || answer.value("result").toVariant().toBool() == false
        || answer.value("list").isArray() == false)
    {
        *error = tr("GDTF-Share sent an answer that can't be read");
        return QList<Entry>();
    }
    *timestamp = secondsOf(answer.value("timestamp"));

    const QJsonArray items = answer.value("list").toArray();
    QList<Entry> entries;
    entries.reserve(items.count());
    for (const QJsonValue &value : items)
    {
        const QJsonObject item = value.toObject();
        Entry entry;
        entry.rid = int(numberOf(item.value("rid")));
        entry.fixture = textOf(item.value("fixture"));
        if (entry.rid <= 0 || entry.fixture.isEmpty())
            continue;
        entry.manufacturer = textOf(item.value("manufacturer"));
        entry.revision = textOf(item.value("revision"));
        entry.uploader = textOf(item.value("uploader"));
        entry.version = textOf(item.value("version"));
        entry.creator = textOf(item.value("creator"));
        entry.uuid = uuidKey(textOf(item.value("uuid")));
        entry.created = secondsOf(item.value("creationDate"));
        entry.modified = secondsOf(item.value("lastModified"));
        entry.filesize = numberOf(item.value("filesize"));
        entry.rating = item.value("rating").toVariant().toDouble();
        for (const QJsonValue &mode : item.value("modes").toArray())
        {
            const QJsonObject object = mode.toObject();
            const QString name = textOf(object.value("name"));
            if (name.isEmpty() == false)
                entry.modes.append({ name, int(numberOf(object.value("dmxfootprint"))) });
        }
        entry.key = fold(entry.manufacturer + ' ' + entry.fixture);
        entry.makerKey = fold(entry.manufacturer);
        entries.append(entry);
    }
    return entries;
}

GdtfShare::List GdtfShare::readList(const QByteArray &json, const QLocale &locale)
{
    List list;
    list.entries = parseList(json, &list.error, &list.timestamp);
    const QList<Entry> &entries = list.entries;

    // the uploads of each fixture type, newest first
    QHash<QString, int> typeOf;
    for (int i = 0; i < entries.count(); i++)
    {
        const QString key = typeKey(entries.at(i));
        const auto found = typeOf.constFind(key);
        if (found == typeOf.constEnd())
        {
            typeOf.insert(key, int(list.types.count()));
            list.types.append(QList<int>({ i }));
        }
        else
        {
            list.types[found.value()].append(i);
        }
    }
    for (QList<int> &type : list.types)
    {
        std::sort(type.begin(), type.end(), [&entries](int a, int b) {
            const Entry &x = entries.at(a);
            const Entry &y = entries.at(b);
            return x.modified != y.modified ? x.modified > y.modified : x.rid > y.rid;
        });
    }

    // the rows: by manufacturer, then fixture, the upload of the
    // manufacturer first (the keys of the language are made once)
    struct Order
    {
        QCollatorSortKey maker;
        QCollatorSortKey fixture;
        bool fromMaker;
        int rid;
        int type;
    };
    QCollator collator(locale);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    collator.setNumericMode(true);
    std::vector<Order> order;
    order.reserve(size_t(list.types.count()));
    for (int i = 0; i < list.types.count(); i++)
    {
        const Entry &entry = entries.at(list.types.at(i).first());
        order.push_back({ collator.sortKey(entry.manufacturer), collator.sortKey(entry.fixture),
                          byManufacturer(entry), entry.rid, i });
    }
    std::sort(order.begin(), order.end(), [](const Order &a, const Order &b) {
        int compared = a.maker.compare(b.maker);
        if (compared == 0)
            compared = a.fixture.compare(b.fixture);
        if (compared != 0)
            return compared < 0;
        if (a.fromMaker != b.fromMaker)
            return a.fromMaker;
        return a.rid < b.rid;
    });

    // and each manufacturer with its fixture types (case and accents ignored)
    QList<QList<int>> types;
    types.reserve(list.types.count());
    QHash<QString, int> makers;
    for (const Order &item : order)
    {
        const QList<int> &type = list.types.at(item.type);
        const Entry &entry = entries.at(type.first());
        list.typeOf.insert(typeKey(entry), int(types.count()));
        types.append(type);
        const auto maker = makers.constFind(entry.makerKey);
        if (maker != makers.constEnd())
        {
            list.manufacturers[maker.value()].second++;
            continue;
        }
        makers.insert(entry.makerKey, int(list.manufacturers.count()));
        list.manufacturers.append({ entry.manufacturer, 1 });
    }
    list.types = types;
    return list;
}

QDir GdtfShare::dataDirectory()
{
    // (asked each time: the user folder can change, as in the tests)
    return QDir(FixtureLibrary::gdtfDirectory().filePath(QStringLiteral("gdtf-share")));
}

QString GdtfShare::listFile()
{
    return dataDirectory().filePath(QStringLiteral("list.json"));
}

int GdtfShare::listCount() const
{
    return int(m_list.types.count());
}

QDateTime GdtfShare::listDate() const
{
    return m_listDate;
}

bool GdtfShare::listLoading() const
{
    return m_listFetching || m_listThread != nullptr;
}

int GdtfShare::listRevision() const
{
    return m_listRevision;
}

QString GdtfShare::listDateText() const
{
    return QLocale().toString(m_listDate.toLocalTime().date(), QLocale::ShortFormat);
}

bool GdtfShare::listFresh() const
{
    // the last answer of this run, else when the saved list was written
    QDateTime last = m_listFetched;
    const QFileInfo saved(listFile());
    if (saved.exists() && (last.isValid() == false || saved.lastModified() > last))
        last = saved.lastModified();
    return last.isValid() && last.secsTo(QDateTime::currentDateTimeUtc()) < LIST_MAX_AGE;
}

void GdtfShare::load()
{
    const QString file = listFile();
    if (m_list.types.isEmpty() && m_listThread == nullptr && QFileInfo::exists(file))
        readListLater({ file, QByteArray(), false, false });
    if (listFresh() == false && sessionPossible())
        refresh(true);
}

void GdtfShare::refresh(bool manual)
{
    // one at a time. The program alone asks only with a session that needs
    // nobody, and not within 20 h of the last list (a courtesy to GDTF-Share)
    if (m_listFetching)
        return;
    if (manual == false && (sessionPossible() == false || listFresh()))
        return;

    Call call;
    call.kind = Fetching;
    call.endpoint = LIST_ENDPOINT;
    call.manual = manual;
    call.notice = downloadingText();
    call.done = [this, manual](const QByteArray &body) {
        m_listFetching = false;
        m_listFetched = QDateTime::currentDateTimeUtc();
        readListLater({ listFile(), body, true, manual });
        return QString();
    };
    call.failed = [this](bool unreachable) { listFailed(unreachable); };
    m_listFetching = true;
    emit listChanged();
    enqueue(call);
}

void GdtfShare::listFailed(bool unreachable)
{
    m_listFetching = false;
    // (the check of the start is tried again at the next start)
    if (m_check == Check::FetchedList)
        m_check = Check::None;
    if (m_notice == downloadingText())
        m_notice.clear();
    // GDTF-Share can't be reached: the saved list is shown, and said to be
    // the saved one (no error: the list is there)
    if (unreachable && (m_list.types.isEmpty() == false || m_listThread != nullptr))
    {
        m_listOffline = true;
        m_error.clear();
        if (m_list.types.isEmpty() == false)
            m_notice = tr("Offline: showing the list from %1").arg(listDateText());
    }
    emit changed();
    emit listChanged();
}

void GdtfShare::readListLater(const ListJob &job)
{
    // one at a time: the newest waits for the one being read
    if (m_listThread != nullptr)
    {
        m_listJobs = { job };
        return;
    }

    const QLocale locale;
    QThread *thread = QThread::create([this, job, locale]() {
        QByteArray body = job.body;
        if (job.fetched == false)
        {
            QFile file(job.file);
            if (file.open(QIODevice::ReadOnly))
                body = file.readAll();
        }
        auto list = std::make_shared<List>(readList(body, locale));
        // GDTF-Share's list is saved once it is known to have fixtures
        if (job.fetched && list->error.isEmpty() && list->entries.isEmpty() == false
            && saveList(job.file, body) == false)
            qWarning() << "GDTF-Share: the list could not be saved in" << job.file;
        QMetaObject::invokeMethod(this, [this, job, list]() { useList(job, *list); }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    m_listThread = thread;
    emit listChanged();
    thread->start();
}

void GdtfShare::useList(const ListJob &job, const List &list)
{
    m_listThread = nullptr;
    const bool used = list.error.isEmpty() && list.entries.isEmpty() == false;
    if (list.error.isEmpty() == false)
    {
        // (the list shown stays)
        if (job.fetched && job.manual)
            m_error = list.error;
    }
    else if (list.entries.isEmpty())
    {
        // GDTF-Share sent none (it happened): the saved one stays
        if (job.fetched)
            m_notice = m_list.types.isEmpty()
                ? tr("GDTF-Share sent an empty list: try again later")
                : tr("GDTF-Share sent an empty list: showing the one from %1").arg(listDateText());
    }
    else
    {
        m_list = list;
        // (the fixtures imported from files are matched with this one)
        m_library.read = false;
        if (list.timestamp > 0)
            m_listDate = timeOf(list.timestamp);
        else
            m_listDate = job.fetched ? QDateTime::currentDateTimeUtc() : QFileInfo(job.file).lastModified().toUTC();
        m_listRevision++;
        if (job.fetched)
        {
            m_listOffline = false;
            if (job.manual)
                m_notice = tr("GDTF-Share: %n fixture type(s)", "", listCount());
        }
        else if (m_listOffline)
        {
            m_notice = tr("Offline: showing the list from %1").arg(listDateText());
        }
    }
    emit changed();
    emit listChanged();
    if (used)
        findUpdates();

    // the check of the start waited for this list
    if (m_check == Check::AnyList || (m_check == Check::FetchedList && job.fetched))
    {
        m_check = Check::None;
        if (used)
            checkDone();
    }

    if (m_listJobs.isEmpty() == false)
        readListLater(m_listJobs.takeFirst());
}

int GdtfShare::recordOf(const QList<int> &type, const Library &library) const
{
    // by its uuid; a fixture type without one by its upload
    const Entry &newest = m_list.entries.at(type.first());
    if (newest.uuid.isEmpty())
        return library.ofRid.value(newest.rid, -1);

    // the newest of its records: one of GDTF-Share before one of a file
    auto better = [](const Record &a, const Record &b) {
        if ((a.rid > 0) != (b.rid > 0))
            return a.rid > 0;
        return a.modified != b.modified ? a.modified > b.modified : a.rid > b.rid;
    };
    int best = -1;
    for (int index : library.ofUuid.value(newest.uuid))
    {
        if (best < 0 || better(library.records.at(index), library.records.at(best)))
            best = index;
    }
    return best;
}

QString GdtfShare::libraryStatus(const QList<int> &type, const Library &library) const
{
    const int index = recordOf(type, library);
    if (index < 0)
        return QString();
    const Record &record = library.records.at(index);
    if (record.rid <= 0)
        return QStringLiteral("file");
    // older when GDTF-Share has a newer revision (a later one, or another
    // upload of the same time)
    const Entry &newest = m_list.entries.at(type.first());
    const bool newer = newest.rid != record.rid
        && (newest.modified > record.modified || (newest.modified == record.modified && newest.rid > record.rid));
    return newer ? QStringLiteral("older") : QStringLiteral("current");
}

QString GdtfShare::libraryModel(const QList<int> &type, const Library &library, bool *replaces) const
{
    *replaces = false;
    const int index = recordOf(type, library);
    if (index >= 0)
        return library.records.at(index).model;

    // another upload of the same fixture (by its name): it is converted in
    // its place, as when its file is imported again
    const Entry &entry = m_list.entries.at(type.first());
    for (const QString &model : { entry.fixture, entry.fixture + QStringLiteral(" (GDTF)") })
    {
        const QString file = GdtfImporter::qxfName(entry.manufacturer, model).toLower();
        if (library.files.contains(file) == false)
            continue;
        *replaces = true;
        const int other = library.ofFile.value(file, -1);
        return other >= 0 && library.records.at(other).model.isEmpty() == false ? library.records.at(other).model : model;
    }
    return QString();
}

QVariantMap GdtfShare::typeRow(const QList<int> &type, const Library &library) const
{
    QVariantMap row = entryMap(m_list.entries.at(type.first()));
    row.insert("revisions", int(type.count()));
    row.insert("status", libraryStatus(type, library));
    // the name of its fixture in the library, and the upload it is (0 from a file)
    bool replaces = false;
    row.insert("libraryModel", libraryModel(type, library, &replaces));
    row.insert("replaces", replaces);
    const int record = recordOf(type, library);
    row.insert("libraryRid", record >= 0 ? library.records.at(record).rid : 0);
    row.insert("libraryManufacturer", record >= 0 ? library.records.at(record).manufacturer : QString());
    return row;
}

/* The words of a search, as fold() leaves them */
static QStringList searchWords(const QString &text)
{
    return GdtfShare::fold(text).simplified().split(' ', Qt::SkipEmptyParts);
}

/* $entry's manufacturer and name have every one of $words */
static bool hasWords(const GdtfShare::Entry &entry, const QStringList &words)
{
    for (const QString &word : words)
    {
        if (entry.key.contains(word) == false)
            return false;
    }
    return true;
}

bool GdtfShare::inFilter(const QList<int> &type, const QString &filter, const Library &library) const
{
    if (filter == QLatin1String("manufacturer"))
        return byManufacturer(m_list.entries.at(type.first()));
    if (filter == QLatin1String("library"))
        return libraryStatus(type, library).isEmpty() == false;
    if (filter == QLatin1String("updates"))
        return libraryStatus(type, library) == QLatin1String("older");
    return true;
}

QVariantList GdtfShare::manufacturers(const QString &text, const QString &filter) const
{
    QList<QPair<QString, int>> makers = m_list.manufacturers;
    const QStringList words = searchWords(text);
    if (words.isEmpty() == false || filter.isEmpty() == false)
    {
        // those of the fixture types found, in the same order
        makers.clear();
        QHash<QString, int> index;
        const Library &fixtures = this->library();
        for (const QList<int> &type : m_list.types)
        {
            const Entry &entry = m_list.entries.at(type.first());
            if (hasWords(entry, words) == false || inFilter(type, filter, fixtures) == false)
                continue;
            const auto found = index.constFind(entry.makerKey);
            if (found != index.constEnd())
            {
                makers[found.value()].second++;
                continue;
            }
            index.insert(entry.makerKey, int(makers.count()));
            makers.append({ entry.manufacturer, 1 });
        }
    }

    QVariantList list;
    for (const QPair<QString, int> &maker : std::as_const(makers))
        list.append(QVariantMap({ { "name", maker.first }, { "count", maker.second } }));
    return list;
}

QVariantMap GdtfShare::search(const QString &text, const QString &manufacturer, const QString &filter, int limit) const
{
    const QStringList words = searchWords(text);
    const QString maker = fold(manufacturer.trimmed());
    const Library &fixtures = this->library();

    QVariantList rows;
    int total = 0;
    for (const QList<int> &type : m_list.types)
    {
        const Entry &entry = m_list.entries.at(type.first());
        if (maker.isEmpty() == false && entry.makerKey != maker)
            continue;
        if (hasWords(entry, words) == false || inFilter(type, filter, fixtures) == false)
            continue;
        if (rows.count() < limit)
            rows.append(typeRow(type, fixtures));
        total++;
    }
    return QVariantMap({ { "rows", rows }, { "total", total } });
}

QVariantList GdtfShare::revisions(const QString &uuid) const
{
    const QString key = uuid.startsWith(QLatin1String("rid:")) ? uuid : uuidKey(uuid);
    QVariantList list;
    for (int index : m_list.types.value(m_list.typeOf.value(key, -1)))
        list.append(entryMap(m_list.entries.at(index)));
    return list;
}

/*********************************************************************
 * What was downloaded
 *********************************************************************/

QString GdtfShare::indexFile()
{
    return dataDirectory().filePath(QStringLiteral("fixtures.json"));
}

/* The fixtures of the record, as written (also those deleted since) */
static QJsonObject recordedFixtures()
{
    QFile file(GdtfShare::indexFile());
    QJsonObject index;
    if (file.open(QIODevice::ReadOnly) == false || readObject(file.readAll(), &index) == false)
        return QJsonObject();
    return index.value("fixtures").toObject();
}

/* The fixture of the record $qxf is still in the GDTF folder */
static bool inGdtfFolder(const QString &qxf)
{
    if (qxf.contains('/') || qxf.contains('\\'))
        return false;
    return QFileInfo(FixtureLibrary::gdtfDirectory().filePath(qxf)).isFile();
}

/* The .gdtf kept of a fixture of the record: only in the folder of
 * GDTF-Share (empty for none, or one elsewhere) */
static QString originalOf(const QJsonValue &fixture)
{
    const QString original = fixture.toObject().value("original").toString();
    if (original.isEmpty())
        return QString();
    const QDir data = GdtfShare::dataDirectory();
    const QString path = QDir::cleanPath(data.absoluteFilePath(original));
    return path.startsWith(data.absolutePath() + '/') ? path : QString();
}

/* The paths of $fixtures' .gdtf kept, to tell them from other files */
static QSet<QString> originalsOf(const QJsonObject &fixtures)
{
    QSet<QString> paths;
    for (const QJsonValue &fixture : fixtures)
    {
        const QString path = originalOf(fixture);
        if (path.isEmpty() == false)
            paths.insert(path.toLower());
    }
    return paths;
}

/* Delete the .gdtf $paths that $kept doesn't have (see originalsOf()):
 * only those in the folder of the originals */
static void removeOriginals(const QStringList &paths, const QSet<QString> &kept)
{
    const QString originals =
        QDir::cleanPath(GdtfShare::dataDirectory().absoluteFilePath(QStringLiteral("originals"))) + '/';
    for (const QString &path : paths)
    {
        if (path.isEmpty())
            continue;
        const QString clean = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
        if (clean.startsWith(originals) && kept.contains(clean.toLower()) == false)
            QFile::remove(clean);
    }
}

/* The downloads (.gdtf, and .part) the record doesn't keep, left by a
 * program that stopped in the middle of one (a crash), deleted. Only those
 * a day old: a program running beside (a screenshot) may be downloading.
 * Not with a record that can't be read: it may keep them */
static void removeLeftBehind()
{
    const QDir originals(GdtfShare::dataDirectory().filePath(QStringLiteral("originals")));
    const QDateTime limit = QDateTime::currentDateTimeUtc().addSecs(-LEFT_BEHIND_AGE);
    QStringList old;
    for (const QFileInfo &file : originals.entryInfoList({ QStringLiteral("*.gdtf"), QStringLiteral("*.gdtf.part") }, QDir::Files))
    {
        if (file.lastModified().toUTC() < limit)
            old.append(file.absoluteFilePath());
    }
    if (old.isEmpty())
        return;
    QFile file(GdtfShare::indexFile());
    QJsonObject index;
    if (file.exists() && (file.open(QIODevice::ReadOnly) == false || readObject(file.readAll(), &index) == false))
        return;
    removeOriginals(old, originalsOf(index.value("fixtures").toObject()));
}

QStringList GdtfShare::originals() const
{
    // (a fixture deleted from the GDTF folder is left out)
    const QJsonObject fixtures = recordedFixtures();
    QStringList list;
    for (auto it = fixtures.constBegin(); it != fixtures.constEnd(); ++it)
    {
        const QString path = originalOf(it.value());
        if (path.isEmpty() == false && inGdtfFolder(it.key()) && QFileInfo(path).isFile())
            list.append(path);
    }
    list.removeDuplicates();
    return list;
}

int GdtfShare::originalsCount() const
{
    return int(originals().count());
}

void GdtfShare::recordImports(const QList<Imported> &imported)
{
    if (imported.isEmpty())
        return;

    // what the record had, without the fixtures deleted since
    const QJsonObject before = recordedFixtures();
    QJsonObject fixtures;
    for (auto it = before.constBegin(); it != before.constEnd(); ++it)
    {
        if (inGdtfFolder(it.key()))
            fixtures.insert(it.key(), it.value());
    }
    // each one in the place of what its .qxf had
    const QDir data = dataDirectory();
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    for (const Imported &item : imported)
    {
        QJsonObject fixture({ { "uuid", item.uuid }, { "rid", item.rid }, { "revision", item.revision },
                              { "revisionText", item.revisionText }, { "modified", item.modified },
                              { "manufacturer", item.manufacturer }, { "model", item.model },
                              { "original", item.original.isEmpty() ? QString() : data.relativeFilePath(item.original) },
                              { "imported", now } });
        fixtures.insert(QFileInfo(item.file).fileName(), fixture);
    }

    data.mkpath(".");
    QSaveFile file(indexFile());
    const QByteArray json = QJsonDocument(QJsonObject({ { "version", 1 }, { "fixtures", fixtures } })).toJson();
    if (file.open(QIODevice::WriteOnly) == false || file.write(json) != json.size() || file.commit() == false)
    {
        qWarning() << "GDTF-Share: the record could not be saved in" << indexFile();
        return;
    }
    // the .gdtf kept that nothing has any more (also one of two uploads of
    // the same name in one go: the last one stays)
    QStringList dropped;
    for (const QJsonValue &fixture : before)
        dropped.append(originalOf(fixture));
    for (const Imported &item : imported)
        dropped.append(item.original);
    removeOriginals(dropped, originalsOf(fixtures));
    emit indexChanged();
}

void GdtfShare::discardDownloads(const QStringList &paths)
{
    if (paths.isEmpty() == false)
        removeOriginals(paths, originalsOf(recordedFixtures()));
}

const GdtfShare::Library &GdtfShare::library() const
{
    // read again once it changed, or in another user folder (the tests)
    const QDir gdtf = FixtureLibrary::gdtfDirectory();
    if (m_library.read && m_library.folder == gdtf.absolutePath())
        return m_library;

    Library library;
    library.folder = gdtf.absolutePath();
    library.read = true;
    for (const QString &name : gdtf.entryList({ QStringLiteral("*.qxf") }, QDir::Files))
        library.files.insert(name.toLower());
    const QJsonObject fixtures = recordedFixtures();
    for (auto it = fixtures.constBegin(); it != fixtures.constEnd(); ++it)
    {
        if (library.files.contains(it.key().toLower()) == false)
            continue;
        const QJsonObject fixture = it.value().toObject();
        Record record;
        record.file = it.key();
        record.uuid = uuidKey(textOf(fixture.value("uuid")));
        record.rid = int(numberOf(fixture.value("rid")));
        record.modified = numberOf(fixture.value("modified"));
        record.manufacturer = textOf(fixture.value("manufacturer"));
        record.model = textOf(fixture.value("model"));
        record.revision = textOf(fixture.value("revision"));
        record.revisionText = textOf(fixture.value("revisionText"));
        if (record.rid <= 0)
            matchUpload(&record);

        const int index = int(library.records.count());
        library.ofFile.insert(it.key().toLower(), index);
        if (record.uuid.isEmpty() == false)
            library.ofUuid[record.uuid].append(index);
        if (record.rid > 0)
            library.ofRid.insert(record.rid, index);
        library.records.append(record);
    }
    m_library = library;
    return m_library;
}

void GdtfShare::matchUpload(Record *record) const
{
    if (record->uuid.isEmpty() || record->revisionText.isEmpty())
        return;
    const Entry *match = nullptr;
    for (int index : m_list.types.value(m_list.typeOf.value(record->uuid, -1)))
    {
        const Entry &entry = m_list.entries.at(index);
        if (entry.revision.compare(record->revisionText, Qt::CaseInsensitive) != 0)
            continue;
        // (two uploads with that text: it can be either, so it is none)
        if (match != nullptr)
            return;
        match = &entry;
    }
    if (match == nullptr)
        return;
    record->rid = match->rid;
    record->modified = match->modified;
    record->revision = match->revision;
    record->matched = true;
}

/*********************************************************************
 * Newer revisions
 *********************************************************************/

QVariantList GdtfShare::updates() const
{
    QVariantList list;
    for (const Update &update : m_updates)
    {
        list.append(QVariantMap({ { "file", update.file }, { "manufacturer", update.manufacturer },
                                  { "model", update.model }, { "fixture", update.fixture }, { "from", update.from },
                                  { "to", update.to }, { "rid", update.rid }, { "modified", timeOf(update.modified) } }));
    }
    return list;
}

int GdtfShare::updatesCount() const
{
    return int(m_updates.count());
}

int GdtfShare::updateAll()
{
    QVariantList rids;
    for (const Update &update : std::as_const(m_updates))
        rids.append(update.rid);
    return download(rids);
}

void GdtfShare::findUpdates()
{
    QList<Update> updates;
    if (m_list.types.isEmpty() == false)
    {
        // the fixture types the library has from GDTF-Share, in the order of the rows
        const Library &library = this->library();
        QList<int> types;
        for (const Record &record : library.records)
        {
            const int type = record.rid > 0 ? m_list.typeOf.value(record.uuid, -1) : -1;
            if (record.uuid.isEmpty() == false && type >= 0 && types.contains(type) == false)
                types.append(type);
        }
        std::sort(types.begin(), types.end());
        for (int type : std::as_const(types))
        {
            const QList<int> &uploads = m_list.types.at(type);
            if (libraryStatus(uploads, library) != QLatin1String("older"))
                continue;
            const Record &record = library.records.at(recordOf(uploads, library));
            const Entry &newest = m_list.entries.at(uploads.first());
            Update update;
            update.file = record.file;
            update.manufacturer = record.manufacturer.isEmpty() ? newest.manufacturer : record.manufacturer;
            update.model = record.model;
            update.fixture = newest.fixture;
            update.from = record.revision;
            update.to = newest.revision;
            update.rid = newest.rid;
            update.modified = newest.modified;
            updates.append(update);
        }
    }

    // told only when they change
    bool same = updates.count() == m_updates.count();
    for (int i = 0; same && i < updates.count(); i++)
    {
        const Update &a = updates.at(i);
        const Update &b = m_updates.at(i);
        same = a.file == b.file && a.rid == b.rid && a.from == b.from && a.model == b.model;
    }
    if (same)
        return;
    m_updates = updates;
    emit updatesChanged();
}

void GdtfShare::setStartupDelay(int ms)
{
    m_startupDelay = ms;
}

bool GdtfShare::checkedRecently() const
{
    const QDateTime last = QDateTime::fromString(QSettings().value(KSettingLastCheck).toString(), Qt::ISODate);
    return last.isValid() && last.secsTo(QDateTime::currentDateTimeUtc()) < LIST_MAX_AGE;
}

void GdtfShare::checkOnStartup(bool autoCheck)
{
    // (nothing is downloaded yet: what a crash left behind can go)
    removeLeftBehind();
    // nothing of GDTF-Share in the library: nothing to look for
    const Library &library = this->library();
    if (library.records.isEmpty())
        return;
    // the updates the saved list knows, also offline (no request)
    if (m_list.types.isEmpty() && m_listThread == nullptr && QFileInfo::exists(listFile()))
        readListLater({ listFile(), QByteArray(), false, false });

    // once a day, with the saved account, for the fixtures downloaded from it
    bool downloads = false;
    for (const Record &record : library.records)
        downloads = downloads || (record.rid > 0 && record.matched == false);
    if (autoCheck == false || downloads == false || remembered() == false || checkedRecently())
        return;
    QTimer::singleShot(m_startupDelay, this, [this]() { checkDaily(); });
}

void GdtfShare::checkDaily()
{
    // (signed out since, or the password forgotten: not without the user)
    if (sessionPossible() == false)
        return;
    // a list new enough (the browser asked for it) is the check: nothing is
    // asked of GDTF-Share
    if (listFresh())
    {
        if (m_listThread != nullptr || m_listFetching)
            m_check = Check::AnyList;
        else if (m_list.types.isEmpty() == false)
            checkDone();
        return;
    }
    // signed in without asking: no error is shown, nobody is asked
    m_check = Check::FetchedList;
    refresh(false);
}

void GdtfShare::checkDone()
{
    QSettings settings;
    settings.setValue(KSettingLastCheck, QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    // each newer revision is told once (the rid of each one, as it was told)
    const QStringList told = settings.value(KSettingAnnounced).toStringList();
    QStringList rids;
    bool news = false;
    for (const Update &update : std::as_const(m_updates))
    {
        const QString rid = QString::number(update.rid);
        rids.append(rid);
        news = news || told.contains(rid) == false;
    }
    if (rids.isEmpty())
        settings.remove(KSettingAnnounced);
    else
        settings.setValue(KSettingAnnounced, rids);
    if (news)
        emit updatesFound(updatesCount());
}

/*********************************************************************
 * Downloads
 *********************************************************************/

/* "Manufacturer Fixture", as the user knows it */
static QString nameOf(const GdtfShare::Entry &entry)
{
    return QStringLiteral("%1 %2").arg(entry.manufacturer, entry.fixture).trimmed();
}

/* The file an upload is kept in: its manufacturer, its name and its rid,
 * with only plain characters (as the .qxf) */
static QString originalName(const GdtfShare::Entry &entry)
{
    QString name = GdtfImporter::qxfName(entry.manufacturer, entry.fixture);
    name.chop(4);
    // (a short path: Windows limits them)
    return QStringLiteral("%1-%2.gdtf").arg(name.left(80)).arg(entry.rid);
}

const GdtfShare::Entry *GdtfShare::entryOf(int rid) const
{
    for (const Entry &entry : m_list.entries)
    {
        if (entry.rid == rid)
            return &entry;
    }
    return nullptr;
}

int GdtfShare::download(const QVariantList &rids)
{
    // the uploads of the list, each once
    QList<Entry> entries;
    QSet<int> added;
    for (const QVariant &value : rids)
    {
        const int rid = value.toInt();
        const Entry *entry = entryOf(rid);
        if (entry == nullptr || m_downloadRids.contains(rid) || added.contains(rid))
            continue;
        entries.append(*entry);
        added.insert(rid);
    }
    if (entries.isEmpty())
        return 0;

    // they join those on their way: all are converted together at the end
    const QDir originals(dataDirectory().filePath(QStringLiteral("originals")));
    const int first = m_downloadCount;
    m_downloadCount += int(entries.count());
    for (int i = 0; i < entries.count(); i++)
    {
        const Entry &entry = entries.at(i);
        m_downloadRids.insert(entry.rid);
        m_downloadBytes += qMax<qint64>(entry.filesize, 0);
        m_downloadSizes = m_downloadSizes && entry.filesize > 0;
        const QString file = QDir::cleanPath(originals.absoluteFilePath(originalName(entry)));
        Call call = downloadCall(entry, file, first + i + 1, m_downloadCount);
        call.follows = first + i > 0;
        m_queue.append(call);
    }
    // (all of them queued first: a sign-in is asked for once)
    runNext();
    return int(entries.count());
}

GdtfShare::Call GdtfShare::downloadCall(const Entry &entry, const QString &file, int number, int count)
{
    Call call;
    call.kind = Downloading;
    call.endpoint = FILE_ENDPOINT;
    call.query = QUrlQuery({ { QStringLiteral("rid"), QString::number(entry.rid) } });
    call.manual = true;
    call.file = file;
    call.notice = count > 1
        ? tr("Downloading %1 (%2 of %3)…").arg(nameOf(entry), QString::number(number), QString::number(count))
        : tr("Downloading %1…").arg(nameOf(entry));
    const int rid = entry.rid;
    const qint64 bytes = qMax<qint64>(entry.filesize, 0);
    call.done = [this, entry, file, bytes](const QByteArray &head) {
        const QString error = keepDownload(entry, file, head);
        downloadEnded(entry.rid, bytes);
        return error;
    };
    call.failed = [this, rid, bytes](bool) { downloadEnded(rid, bytes); };
    return call;
}

QString GdtfShare::keepDownload(const Entry &entry, const QString &file, const QByteArray &head)
{
    const QString part = partOf(file);
    // a .gdtf is a zip. Else GDTF-Share's own words where the file was
    // expected, or anything else (a page, a file cut short): not kept
    if (head.startsWith(QByteArrayLiteral("PK\x03\x04")) == false)
    {
        QFile::remove(part);
        const QString error = shortAnswer(head).value("error").toVariant().toString().trimmed();
        if (error.isEmpty() == false)
            return tr("GDTF-Share answered with an error: %1").arg(error);
        return tr("%1 is not a GDTF file: it was not imported").arg(nameOf(entry));
    }
    // (the same upload, downloaded again: the new one)
    QFile::remove(file);
    if (QFile::rename(part, file) == false)
    {
        QFile::remove(part);
        return tr("Unable to write %1").arg(QDir::toNativeSeparators(file));
    }
    Download download;
    download.path = file;
    download.uuid = entry.uuid;
    download.rid = entry.rid;
    download.revision = entry.revision;
    download.modified = entry.modified;
    download.manufacturer = entry.manufacturer;
    download.fixture = entry.fixture;
    m_downloads.append(download);
    return QString();
}

void GdtfShare::downloadEnded(int rid, qint64 bytes)
{
    if (m_downloadRids.remove(rid) == false)
        return;
    m_downloadsOver++;
    m_downloadedBytes += bytes;
    if (m_downloadRids.isEmpty() == false)
    {
        downloadProgress(0, 0, true);
        return;
    }

    // the last one: those that arrived whole are converted, all together
    const QList<Download> downloads = m_downloads;
    m_downloads.clear();
    m_downloadCount = 0;
    m_downloadsOver = 0;
    m_downloadBytes = 0;
    m_downloadedBytes = 0;
    m_downloadSizes = true;
    m_progress = 1;
    emit progressChanged();
    if (downloads.isEmpty() == false)
        emit downloaded(downloads);
}

void GdtfShare::downloadProgress(qint64 received, qint64 total, bool batch)
{
    // one file: its bytes. Those of download(): the bytes of all of them, as
    // the list says, else the files
    const qreal file = total > 0 ? qreal(received) / qreal(total) : 0;
    if (batch == false)
        m_progress = file;
    else if (m_downloadSizes && m_downloadBytes > 0)
        m_progress = qMin<qreal>(1, qreal(m_downloadedBytes + received) / qreal(m_downloadBytes));
    else
        m_progress = (qreal(m_downloadsOver) + file) / qreal(qMax(1, m_downloadCount));
    emit progressChanged();
}

bool GdtfShare::openPart(const QString &file)
{
    closePart(false);
    QDir().mkpath(QFileInfo(file).absolutePath());
    m_part = std::make_unique<QFile>(partOf(file));
    if (m_part->open(QIODevice::WriteOnly | QIODevice::Truncate))
        return true;
    m_part.reset();
    return false;
}

void GdtfShare::closePart(bool keep)
{
    if (m_part == nullptr)
        return;
    const QString name = m_part->fileName();
    m_part->close();
    m_part.reset();
    if (keep == false)
        QFile::remove(name);
}

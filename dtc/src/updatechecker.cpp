/*
  DMX Total Control
  updatechecker.cpp

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
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QSettings>
#include <QProcess>
#include <QTimer>
#include <QFileInfo>
#include <QLocale>
#include <QDir>
#include <QUrl>

#include "updatechecker.h"
#include "qlcconfig.h"

// the repository of the releases ("owner/name"); CMake sets it (DTC_GITHUB_REPO)
#ifndef DTC_UPDATE_REPO
#define DTC_UPDATE_REPO "joseramos1999/DMXTotalControl"
#endif

#define KSettingAutoCheck   QStringLiteral("updates/autoCheck")
#define KSettingPrereleases QStringLiteral("updates/prereleases")
#define KSettingLastCheck   QStringLiteral("updates/lastCheck")
#define KSettingDismissed   QStringLiteral("updates/dismissed")

/** The installers of the releases: DMXTotalControl-Setup-0.13.0.exe */
static const QLatin1String KAssetPrefix("DMXTotalControl-Setup-");

UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
    , m_state(Idle)
    , m_manual(false)
    , m_progress(0)
    , m_installOnExit(false)
    , m_hash(QCryptographicHash::Sha256)
    , m_received(0)
{
}

UpdateChecker::~UpdateChecker()
{
    if (m_reply)
        m_reply->abort();
}

/*********************************************************************
 * Properties
 *********************************************************************/

int UpdateChecker::state() const
{
    return m_state;
}

QString UpdateChecker::currentVersion() const
{
    return QString(APPVERSION);
}

QString UpdateChecker::latestVersion() const
{
    return m_release ? m_release->version : QString();
}

QString UpdateChecker::notes() const
{
    return m_release ? m_release->notes : QString();
}

QString UpdateChecker::pageUrl() const
{
    return m_release ? m_release->pageUrl : QString();
}

bool UpdateChecker::prerelease() const
{
    return m_release && m_release->prerelease;
}

bool UpdateChecker::installable() const
{
    return m_release && m_release->assetUrl.isEmpty() == false && m_release->sha256.isEmpty() == false;
}

qreal UpdateChecker::progress() const
{
    return m_progress;
}

QString UpdateChecker::error() const
{
    return m_error;
}

QString UpdateChecker::lastCheck() const
{
    const QDateTime when = QDateTime::fromString(QSettings().value(KSettingLastCheck).toString(), Qt::ISODate);
    return when.isValid() ? QLocale().toString(when.toLocalTime(), QLocale::ShortFormat) : QString();
}

bool UpdateChecker::installOnExit() const
{
    return m_installOnExit;
}

bool UpdateChecker::dismissed() const
{
    return m_release && QSettings().value(KSettingDismissed).toString() == m_release->version;
}

bool UpdateChecker::autoCheck() const
{
    return QSettings().value(KSettingAutoCheck, true).toBool();
}

void UpdateChecker::setAutoCheck(bool on)
{
    if (on == autoCheck())
        return;
    QSettings().setValue(KSettingAutoCheck, on);
    emit settingsChanged();
}

bool UpdateChecker::prereleases() const
{
    return QSettings().value(KSettingPrereleases, false).toBool();
}

void UpdateChecker::setPrereleases(bool on)
{
    if (on == prereleases())
        return;
    QSettings().setValue(KSettingPrereleases, on);
    emit settingsChanged();
}

QString UpdateChecker::downloadedFile() const
{
    return m_downloaded;
}

QString UpdateChecker::repository()
{
    return QStringLiteral(DTC_UPDATE_REPO);
}

/*********************************************************************
 * Versions and releases
 *********************************************************************/

QList<int> UpdateChecker::parseVersion(const QString &text)
{
    // "v0.13.2", "0.13.0 GIT", "0.14.0-beta.1": the numbers at the start
    static const QRegularExpression re(QStringLiteral("^\\s*[vV]?(\\d+)(?:\\.(\\d+))?(?:\\.(\\d+))?"));
    const QRegularExpressionMatch match = re.match(text);
    QList<int> numbers;
    if (match.hasMatch() == false)
        return numbers;
    for (int i = 1; i <= 3; i++)
        numbers.append(match.captured(i).isEmpty() ? 0 : match.captured(i).toInt());
    return numbers;
}

int UpdateChecker::compareVersions(const QString &a, const QString &b)
{
    const QList<int> x = parseVersion(a), y = parseVersion(b);
    for (int i = 0; i < 3; i++)
    {
        const int p = x.value(i, 0), q = y.value(i, 0);
        if (p != q)
            return p < q ? -1 : 1;
    }
    return 0;
}

std::optional<UpdateChecker::Release> UpdateChecker::pickRelease(const QJsonDocument &doc, const QString &current,
                                                                 bool prereleases, const QString &repo)
{
    QJsonArray releases;
    if (doc.isArray())
        releases = doc.array();
    else if (doc.isObject())
        releases.append(doc.object());

    const QString downloads = QStringLiteral("https://github.com/%1/releases/download/").arg(repo);
    const QString pages = QStringLiteral("https://github.com/%1/releases/").arg(repo);
    std::optional<Release> best;

    for (const QJsonValue &value : std::as_const(releases))
    {
        const QJsonObject release = value.toObject();
        if (release.value("draft").toBool())
            continue;
        const bool pre = release.value("prerelease").toBool();
        if (pre && prereleases == false)
            continue;
        const QString tag = release.value("tag_name").toString();
        if (parseVersion(tag).isEmpty() || compareVersions(tag, current) <= 0)
            continue;
        if (best && compareVersions(tag, best->version) <= 0)
            continue;

        Release r;
        const QList<int> numbers = parseVersion(tag);
        r.version = QStringLiteral("%1.%2.%3").arg(numbers.at(0)).arg(numbers.at(1)).arg(numbers.at(2));
        r.tag = tag;
        r.notes = release.value("body").toString();
        r.prerelease = pre;
        const QString page = release.value("html_url").toString();
        if (repo.isEmpty() || page.startsWith(pages, Qt::CaseInsensitive))
            r.pageUrl = page;

        // its installer, from the releases of this repository only
        bool found = false;
        for (const QJsonValue &a : release.value("assets").toArray())
        {
            const QJsonObject asset = a.toObject();
            const QString name = asset.value("name").toString();
            if (name.startsWith(KAssetPrefix, Qt::CaseInsensitive) == false ||
                name.endsWith(QLatin1String(".exe"), Qt::CaseInsensitive) == false)
                continue;
            const QString url = asset.value("browser_download_url").toString();
            if (repo.isEmpty() == false && url.startsWith(downloads, Qt::CaseInsensitive) == false)
                continue;
            // without a well-formed digest it is announced but never installed from here
            static const QRegularExpression sha(QStringLiteral("^sha256:([0-9a-fA-F]{64})$"));
            const QRegularExpressionMatch digest = sha.match(asset.value("digest").toString());
            r.assetName = QFileInfo(name).fileName();
            r.assetUrl = url;
            r.assetSize = qint64(asset.value("size").toDouble());
            r.sha256 = digest.hasMatch() ? digest.captured(1).toLower() : QString();
            found = true;
            break;
        }
        // a release without its installer (still being published) is not an update
        if (found)
            best = r;
    }
    return best;
}

/*********************************************************************
 * Checking
 *********************************************************************/

QString UpdateChecker::apiUrl() const
{
    if (m_apiUrl.isEmpty() == false)
        return m_apiUrl;
    return prereleases()
               ? QStringLiteral("https://api.github.com/repos/%1/releases?per_page=20").arg(repository())
               : QStringLiteral("https://api.github.com/repos/%1/releases/latest").arg(repository());
}

void UpdateChecker::setApiUrl(const QString &url)
{
    m_apiUrl = url;
}

void UpdateChecker::setDownloadDir(const QString &dir)
{
    m_downloadDir = dir;
}

static QNetworkRequest githubRequest(const QUrl &url, const QByteArray &accept)
{
    QNetworkRequest request(url);
    // GitHub refuses requests without a User-Agent
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("DMXTotalControl/%1").arg(QString(APPVERSION).section(' ', 0, 0)));
    request.setRawHeader("Accept", accept);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    // given up after this long without data
    request.setTransferTimeout(30000);
    return request;
}

void UpdateChecker::check(bool manual)
{
    if (m_state == Checking || m_state == Downloading)
        return;
    // a downloaded installer stays: the check would only find it again
    if (m_state == Ready && manual == false)
        return;

    m_manual = manual;
    m_error.clear();
    if (manual)
        QSettings().remove(KSettingDismissed);
    setState(Checking);

    QNetworkReply *reply = m_network->get(githubRequest(QUrl(apiUrl()), "application/vnd.github+json"));
    m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { finishCheck(reply); });
}

void UpdateChecker::finishCheck(QNetworkReply *reply)
{
    reply->deleteLater();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // no release yet: up to date
    if (status == 404)
    {
        QSettings().setValue(KSettingLastCheck, QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        m_release.reset();
        setState(UpToDate);
        return;
    }
    if (reply->error() != QNetworkReply::NoError)
    {
        // without a network at startup it says nothing
        if (m_manual)
            fail(tr("GitHub could not be reached: %1").arg(reply->errorString()));
        else
            setState(Idle);
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        if (m_manual)
            fail(tr("GitHub sent an answer that can't be read"));
        else
            setState(Idle);
        return;
    }

    QSettings().setValue(KSettingLastCheck, QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    m_release = pickRelease(doc, currentVersion(), prereleases(), m_apiUrl.isEmpty() ? repository() : QString());
    setState(m_release ? Available : UpToDate);
}

void UpdateChecker::checkOnStartup()
{
    if (autoCheck() == false)
        return;
    const QDateTime last = QDateTime::fromString(QSettings().value(KSettingLastCheck).toString(), Qt::ISODate);
    if (last.isValid() && last.secsTo(QDateTime::currentDateTimeUtc()) < 20 * 3600)
        return;
    // after the start, when the show is open
    QTimer::singleShot(5000, this, [this]() { check(false); });
}

void UpdateChecker::dismiss()
{
    if (m_release == std::nullopt)
        return;
    QSettings().setValue(KSettingDismissed, m_release->version);
    emit changed();
}

void UpdateChecker::openPage()
{
    if (pageUrl().isEmpty() == false)
        QDesktopServices::openUrl(QUrl(pageUrl()));
}

/*********************************************************************
 * Downloading
 *********************************************************************/

QString UpdateChecker::targetPath() const
{
    const QString dir = m_downloadDir.isEmpty() ? QDir::temp().filePath(QStringLiteral("DMXTotalControl-update")) : m_downloadDir;
    return QDir(dir).filePath(m_release ? m_release->assetName : QString());
}

QString UpdateChecker::partialPath() const
{
    return targetPath() + QStringLiteral(".part");
}

void UpdateChecker::download()
{
    if (installable() == false || m_state == Downloading || m_state == Checking)
        return;
    if (m_state == Ready && QFileInfo::exists(m_downloaded))
        return;

    QDir().mkpath(QFileInfo(targetPath()).absolutePath());
    if (m_file.isOpen())
        m_file.close();
    m_file.setFileName(partialPath());
    if (m_file.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
    {
        fail(tr("The installer can't be saved in %1").arg(QDir::toNativeSeparators(QFileInfo(m_file).absolutePath())));
        return;
    }
    m_hash.reset();
    m_received = 0;
    m_progress = 0;
    m_error.clear();
    emit progressChanged();
    setState(Downloading);

    QNetworkReply *reply = m_network->get(githubRequest(QUrl(m_release->assetUrl), "application/octet-stream"));
    m_reply = reply;
    connect(reply, &QIODevice::readyRead, this, [this, reply]()
    {
        const QByteArray data = reply->readAll();
        m_file.write(data);
        m_hash.addData(data);
        m_received += data.size();
    });
    connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total)
    {
        const qint64 size = m_release && m_release->assetSize > 0 ? m_release->assetSize : total;
        m_progress = size > 0 ? qBound(0.0, qreal(received) / qreal(size), 1.0) : 0.0;
        emit progressChanged();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { finishDownload(reply); });
}

void UpdateChecker::finishDownload(QNetworkReply *reply)
{
    reply->deleteLater();
    // what is still in the buffer
    const QByteArray rest = reply->readAll();
    m_file.write(rest);
    m_hash.addData(rest);
    m_received += rest.size();
    m_file.close();

    QString problem;
    if (reply->error() != QNetworkReply::NoError)
        problem = tr("The download failed: %1").arg(reply->errorString());
    else if (m_release->assetSize > 0 && m_received != m_release->assetSize)
        problem = tr("The download is incomplete (%1 of %2 bytes)").arg(m_received).arg(m_release->assetSize);
    else if (QString::fromLatin1(m_hash.result().toHex()) != m_release->sha256)
        problem = tr("The installer doesn't match the SHA-256 digest GitHub publishes: it was not kept");
    if (problem.isEmpty() == false)
    {
        QFile::remove(partialPath());
        fail(problem);
        return;
    }

    QFile::remove(targetPath());
    if (QFile::rename(partialPath(), targetPath()) == false)
    {
        QFile::remove(partialPath());
        fail(tr("The installer can't be saved in %1").arg(QDir::toNativeSeparators(QFileInfo(targetPath()).absolutePath())));
        return;
    }
    m_downloaded = targetPath();
    m_progress = 1;
    emit progressChanged();
    setState(Ready);
}

/*********************************************************************
 * Installing
 *********************************************************************/

QStringList UpdateChecker::installerArguments(bool relaunch) const
{
    // over the installation of this program: the installer finds it and
    // keeps its mode, for the user or for everyone, wherever its folder is
    // (UsePreviousPrivileges); without one, for the user. Never /ALLUSERS or
    // /CURRENTUSER: they choose the mode and it would install a second copy
    QStringList args = { QStringLiteral("/SILENT"), QStringLiteral("/SUPPRESSMSGBOXES"), QStringLiteral("/NORESTART"),
                         QStringLiteral("/CLOSEAPPLICATIONS"), QStringLiteral("/NORESTARTAPPLICATIONS") };
    if (relaunch)
        args.append(QStringLiteral("/RELAUNCH=1"));
    return args;
}

bool UpdateChecker::installNow()
{
    if (m_state != Ready || QFileInfo::exists(m_downloaded) == false)
        return false;
    if (QProcess::startDetached(m_downloaded, installerArguments(true)) == false)
    {
        fail(tr("The installer could not be started"));
        return false;
    }
    m_installOnExit = false;
    emit changed();
    return true;
}

void UpdateChecker::setInstallOnExit(bool on)
{
    on = on && m_state == Ready;
    if (on == m_installOnExit)
        return;
    m_installOnExit = on;
    emit changed();
}

void UpdateChecker::runPendingInstall()
{
    if (m_installOnExit && QFileInfo::exists(m_downloaded))
        QProcess::startDetached(m_downloaded, installerArguments(false));
    m_installOnExit = false;
}

/*********************************************************************
 * State
 *********************************************************************/

void UpdateChecker::setState(State state)
{
    m_state = state;
    emit changed();
}

void UpdateChecker::fail(const QString &message)
{
    m_error = message;
    setState(Failed);
}

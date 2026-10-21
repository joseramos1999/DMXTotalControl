/*
  DMX Total Control
  fixturelibrary.cpp

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
#include <QDesktopServices>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QHash>
#include <QSet>
#include <QSettings>
#include <QLocale>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>

#include <memory>

// QZipReader moved from QtGui to QtCore in recent Qt versions
#if __has_include(<QtCore/private/qzipreader_p.h>)
#include <QtCore/private/qzipreader_p.h>
#else
#include <QtGui/private/qzipreader_p.h>
#endif

#include "fixturelibrary.h"
#include "patchmanager.h"
#include "gdtfimporter.h"

#include "qlcfixturedefcache.h"
#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcphysical.h"
#include "qlcchannel.h"
#include "qlcconfig.h"
#include "qlcfile.h"
#include "universe.h"
#include "fixture.h"
#include "doc.h"

#define OFL_DOWNLOAD_URL QStringLiteral("https://open-fixture-library.org/download.qlcplus_4.12.2")
// the last change of its fixtures, on GitHub (where the site is published from)
#define OFL_COMMITS_URL  QStringLiteral("https://api.github.com/repos/OpenLightingProject/open-fixture-library/commits?path=fixtures&per_page=1")
#define KSettingOflDownloaded QStringLiteral("library/oflDownloaded")
#define KSettingOflLatest     QStringLiteral("library/oflLatest")
#define KSettingOflLastCheck  QStringLiteral("library/oflLastCheck")
#define GENERIC_SUBDIR   QStringLiteral("DMXTotalControl")
#define OFL_SUBDIR       QStringLiteral("OpenFixtureLibrary")
#define GDTF_SUBDIR      QStringLiteral("GDTF")

/* A time saved in the settings (UTC, ISO 8601): invalid when there is none */
static QDateTime settingTime(const QString &key)
{
    const QDateTime time = QDateTime::fromString(QSettings().value(key).toString(), Qt::ISODate);
    return time.isValid() ? time.toUTC() : time;
}

/* The status of an update when some patched fixtures keep the old version */
static QString keptMessage(int kept)
{
    return FixtureLibrary::tr("%n patched fixture(s) keep the previous version until patched again: "
                              "their new mode doesn't fit where they are", "", kept);
}

FixtureLibrary::FixtureLibrary(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_network(nullptr)
    , m_reply(nullptr)
    , m_progress(0)
    , m_cancelImport(false)
    , m_importProgress(0)
    , m_oflDownloadUrl(OFL_DOWNLOAD_URL)
    , m_oflCommitsUrl(OFL_COMMITS_URL)
{
    // (an update found before, and not downloaded yet; from the settings
    // alone: the user folder is only made when something needs it)
    const QDateTime downloaded = settingTime(KSettingOflDownloaded);
    m_oflUpdate = downloaded.isValid() && settingTime(KSettingOflLatest) > downloaded;
}

FixtureLibrary::~FixtureLibrary()
{
    // a GDTF import still running stops between two files
    if (m_importThread != nullptr)
    {
        m_cancelImport = true;
        m_importThread->wait();
    }
}

QDir FixtureLibrary::genericDefinitionDirectory()
{
    QDir dir = QLCFixtureDefCache::systemDefinitionDirectory();
    dir.setPath(dir.filePath(GENERIC_SUBDIR));
    return dir;
}

QDir FixtureLibrary::openFixtureLibraryDirectory()
{
    QDir dir = QLCFixtureDefCache::userDefinitionDirectory();
    dir.setPath(dir.filePath(OFL_SUBDIR));
    return dir;
}

QDir FixtureLibrary::gdtfDirectory()
{
    QDir dir = QLCFixtureDefCache::userDefinitionDirectory();
    dir.setPath(dir.filePath(GDTF_SUBDIR));
    return dir;
}

void FixtureLibrary::loadExtraDefinitions()
{
    // Our generics, then the Open Fixture Library: the definitions QLC+
    // ships stay the reference when both libraries have the same fixture
    QLCFixtureDefCache *cache = m_doc->fixtureDefCache();
    if (genericDefinitionDirectory().exists())
        cache->load(genericDefinitionDirectory());
    // the GDTF imports before the Open Fixture Library: an import keeps its fixture
    QDir gdtf = gdtfDirectory();
    for (const QString &file : gdtf.entryList(QStringList() << "*.qxf", QDir::Files))
        cache->loadQXF(gdtf.filePath(file), true);
    if (openFixtureLibraryDirectory().exists())
        cache->load(openFixtureLibraryDirectory());
}

QVariantList FixtureLibrary::userFixtures() const
{
    // only what the user made or imported: not our generics, nor the OFL download,
    // which the engine also flags as "user" definitions
    const QString userDir = QLCFixtureDefCache::userDefinitionDirectory().absolutePath();

    QVariantList list;
    for (QLCFixtureDef *def : m_doc->fixtureDefCache()->defs())
    {
        if (def->isUser() == false)
            continue;
        if (QFileInfo(def->definitionSourceFile()).absolutePath() != userDir)
            continue;

        QVariantMap info;
        info.insert("manufacturer", def->manufacturer());
        info.insert("model", def->model());
        info.insert("file", def->definitionSourceFile());
        list.append(info);
    }

    std::sort(list.begin(), list.end(), [](const QVariant &a, const QVariant &b) {
        QString ka = a.toMap().value("manufacturer").toString() + a.toMap().value("model").toString();
        QString kb = b.toMap().value("manufacturer").toString() + b.toMap().value("model").toString();
        return ka.compare(kb, Qt::CaseInsensitive) < 0;
    });
    return list;
}

int FixtureLibrary::definitionsCount() const
{
    return m_doc->fixtureDefCache()->defs().count();
}

bool FixtureLibrary::downloading() const
{
    return m_reply != nullptr;
}

qreal FixtureLibrary::downloadProgress() const
{
    return m_progress;
}

QString FixtureLibrary::status() const
{
    return m_status;
}

void FixtureLibrary::setStatus(const QString &status)
{
    m_status = status;
    emit statusChanged();
}

bool FixtureLibrary::openFixtureLibraryInstalled() const
{
    QDir dir = openFixtureLibraryDirectory();
    return dir.exists() && dir.entryList(QStringList() << "*.qxf", QDir::Files).isEmpty() == false;
}

int FixtureLibrary::oflCount() const
{
    QDir dir = openFixtureLibraryDirectory();
    return dir.exists() ? int(dir.entryList(QStringList() << "*.qxf", QDir::Files).count()) : 0;
}

QDateTime FixtureLibrary::oflDownloaded() const
{
    if (openFixtureLibraryInstalled() == false)
        return QDateTime();
    // (downloaded before the date was kept: the date of its folder)
    const QDateTime time = settingTime(KSettingOflDownloaded);
    return time.isValid() ? time : QFileInfo(openFixtureLibraryDirectory().absolutePath()).lastModified().toUTC();
}

bool FixtureLibrary::oflUpdateAvailable() const
{
    return m_oflUpdate;
}

bool FixtureLibrary::oflChecking() const
{
    return m_checkReply != nullptr;
}

QStringList FixtureLibrary::oflNewFixtures() const
{
    return m_oflNew;
}

void FixtureLibrary::setOpenFixtureLibraryUrls(const QString &download, const QString &commits)
{
    m_oflDownloadUrl = download;
    m_oflCommitsUrl = commits;
}

void FixtureLibrary::checkOnStartup(bool autoCheck)
{
    if (autoCheck == false || openFixtureLibraryInstalled() == false)
        return;
    const QDateTime last = settingTime(KSettingOflLastCheck);
    if (last.isValid() && last.secsTo(QDateTime::currentDateTimeUtc()) < 20 * 3600)
        return;
    // after the start and the check of the program
    QTimer::singleShot(8000, this, [this]() { checkOpenFixtureLibrary(false); });
}

void FixtureLibrary::checkOpenFixtureLibrary(bool manual)
{
    if (m_checkReply != nullptr)
        return;
    if (m_network == nullptr)
        m_network = new QNetworkAccessManager(this);

    QNetworkRequest request{QUrl(m_oflCommitsUrl)};
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("%1/%2").arg(APPNAME, APPVERSION));
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setTransferTimeout(20000);
    m_checkManual = manual;
    m_checkReply = m_network->get(request);
    connect(m_checkReply, &QNetworkReply::finished, this, &FixtureLibrary::slotCheckFinished);
    if (manual)
        setStatus(tr("Looking for new fixtures in the Open Fixture Library…"));
    emit oflCheckChanged();
}

void FixtureLibrary::slotCheckFinished()
{
    QNetworkReply *reply = m_checkReply;
    m_checkReply = nullptr;
    reply->deleteLater();

    // [{ "commit": { "committer": { "date": "2026-10-20T08:00:00Z" } } }]
    const QJsonArray commits = QJsonDocument::fromJson(reply->readAll()).array();
    const QDateTime latest = commits.isEmpty() ? QDateTime()
        : QDateTime::fromString(commits.first().toObject().value("commit").toObject()
                                    .value("committer").toObject().value("date").toString(), Qt::ISODate).toUTC();
    if (reply->error() != QNetworkReply::NoError || latest.isValid() == false)
    {
        if (m_checkManual)
            setStatus(tr("Couldn't look for new fixtures: %1")
                      .arg(reply->error() != QNetworkReply::NoError ? reply->errorString() : tr("unexpected answer")));
        emit oflCheckChanged();
        return;
    }

    QSettings settings;
    settings.setValue(KSettingOflLastCheck, QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    settings.setValue(KSettingOflLatest, latest.toString(Qt::ISODate));
    const bool update = openFixtureLibraryInstalled() && latest > oflDownloaded();
    const bool found = update && m_oflUpdate == false;
    m_oflUpdate = update;
    if (m_checkManual)
        setStatus(update ? tr("The Open Fixture Library has new or improved fixtures since your download (%1)")
                               .arg(QLocale().toString(latest.toLocalTime().date(), QLocale::ShortFormat))
                         : tr("The Open Fixture Library is up to date"));
    emit oflCheckChanged();
    if (found && m_checkManual == false)
        emit oflUpdateFound();
}

QString FixtureLibrary::mergeOpenFixtureLibrary(const QDir &staging, int count)
{
    QDir target = openFixtureLibraryDirectory();
    target.mkpath(".");
    QLCFixtureDefCache *cache = m_doc->fixtureDefCache();
    // (the first download is all new: only an update tells its new ones)
    const bool update = target.entryList(QStringList() << "*.qxf", QDir::Files).isEmpty() == false;

    QStringList added;
    int updated = 0;
    int kept = 0;
    for (const QString &file : staging.entryList(QStringList() << "*.qxf", QDir::Files))
    {
        const QString from = staging.filePath(file);
        const QString to = target.filePath(file);
        // the same as the one there: nothing to do
        QFile old(to);
        QFile fresh(from);
        const bool existed = old.exists();
        if (existed && old.open(QIODevice::ReadOnly) && fresh.open(QIODevice::ReadOnly) &&
            old.size() == fresh.size() && old.readAll() == fresh.readAll())
            continue;
        old.close();
        fresh.close();

        QLCFixtureDef *def = new QLCFixtureDef();
        if (def->loadXML(from) != QFile::NoError || def->modes().isEmpty())
        {
            delete def;
            continue;
        }
        QFile::remove(to);
        if (QFile::copy(from, to) == false)
        {
            delete def;
            continue;
        }
        def->setDefinitionSourceFile(to);

        // in the library: new, or the one from this file changed. The same
        // fixture from another library (the built-in one, the user's) stays
        QLCFixtureDef *current = cache->fixtureDef(def->manufacturer(), def->model());
        if (current == nullptr)
        {
            const QString name = def->manufacturer() + " " + def->model();
            def->setIsUser(true);
            def->setLoaded(true);
            if (cache->addFixtureDef(def))
                added.append(name);
            else
                delete def;
        }
        else if (QFileInfo(current->definitionSourceFile()) == QFileInfo(to))
        {
            kept += installDefinition(def);
            updated++;
        }
        else
        {
            delete def;
        }
    }

    QSettings().setValue(KSettingOflDownloaded, QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    m_oflUpdate = false;
    std::sort(added.begin(), added.end(), [](const QString &a, const QString &b) {
        return QString::localeAwareCompare(a, b) < 0;
    });
    m_oflNew = update ? added : QStringList();
    emit oflCheckChanged();
    emit libraryChanged();

    QString status = tr("Open Fixture Library: %n fixture(s) downloaded", "", count);
    if (added.isEmpty() && updated == 0)
        status += " · " + tr("nothing new");
    if (added.isEmpty() == false)
        status += " · " + tr("%n new in the library", "", int(added.count()));
    if (updated > 0)
        status += " · " + tr("%n improved", "", updated);
    if (kept > 0)
        status += " · " + keptMessage(kept);
    return status;
}

/*********************************************************************
 * Import
 *********************************************************************/

int FixtureLibrary::loadNewDefinitions(const QDir &dir)
{
    int before = m_doc->fixtureDefCache()->defs().count();
    m_doc->fixtureDefCache()->load(dir);
    int added = m_doc->fixtureDefCache()->defs().count() - before;
    emit libraryChanged();
    return added;
}

int FixtureLibrary::importQxf(const QString &path, const QDir &target, const QString &prefix)
{
    QLCFixtureDef def;
    if (def.loadXML(path) != QFile::NoError || def.modes().isEmpty())
    {
        qWarning() << Q_FUNC_INFO << "Not a valid fixture definition:" << path;
        return 0;
    }

    QString fileName = prefix + QFileInfo(path).fileName();
    QString destination = target.filePath(fileName);
    if (QFileInfo(path).absoluteFilePath() == QFileInfo(destination).absoluteFilePath())
        return 0;

    QFile::remove(destination);
    return QFile::copy(path, destination) ? 1 : 0;
}

int FixtureLibrary::importZip(const QString &path, const QDir &target)
{
    QZipReader zip(path);
    if (zip.isReadable() == false)
        return 0;

    int count = 0;
    for (const QZipReader::FileInfo &info : zip.fileInfoList())
    {
        if (info.isFile == false || info.filePath.endsWith(".qxf", Qt::CaseInsensitive) == false)
            continue;

        // flatten "Manufacturer/model.qxf" to "Manufacturer-model.qxf"
        QString name = info.filePath;
        name.replace('/', '-').replace('\\', '-');

        // an entry that can't be read leaves no empty file behind (it was
        // counted, and failed to load at every start)
        const QByteArray data = GdtfImporter::zipFileData(zip, info.filePath);
        if (data.isEmpty())
            continue;
        QFile out(target.filePath(name));
        if (out.open(QIODevice::WriteOnly) == false)
            continue;
        out.write(data);
        out.close();
        count++;
    }
    return count;
}

int FixtureLibrary::importFiles(const QList<QUrl> &urls)
{
    QDir target = QLCFixtureDefCache::userDefinitionDirectory();
    target.mkpath(".");

    int copied = 0;
    QList<GdtfJob> gdtf;
    auto isGdtf = [](const QString &name) { return name.endsWith(".gdtf", Qt::CaseInsensitive); };
    for (const QUrl &url : urls)
    {
        QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
        QFileInfo fi(path);
        if (fi.isDir())
        {
            QDir dir(path);
            for (const QString &file : dir.entryList(QStringList() << "*.qxf", QDir::Files))
                copied += importQxf(dir.filePath(file), target);
            for (const QString &file : dir.entryList(QStringList() << "*.gdtf", QDir::Files))
                gdtf.append({ dir.filePath(file), QString() });
        }
        else if (fi.suffix().compare("zip", Qt::CaseInsensitive) == 0)
        {
            copied += importZip(path, target);
            // a download of GDTF-Share: many .gdtf in one .zip
            QZipReader zip(path);
            for (const QZipReader::FileInfo &info : zip.fileInfoList())
                if (info.isFile && isGdtf(info.filePath))
                    gdtf.append({ path, info.filePath });
        }
        else if (isGdtf(path))
        {
            gdtf.append({ path, QString() });
        }
        else
        {
            copied += importQxf(path, target);
        }
    }

    if (gdtf.isEmpty())
    {
        int added = loadNewDefinitions(target);
        setStatus(tr("%1 file(s) imported, %2 new fixture(s) in the library").arg(copied).arg(added));
        return copied;
    }
    if (copied > 0)
        loadNewDefinitions(target);
    startGdtf(gdtf, copied);
    return copied + int(gdtf.count());
}

void FixtureLibrary::startGdtf(const QList<GdtfJob> &jobs, int copied)
{
    if (m_importThread != nullptr)
    {
        setStatus(tr("A GDTF import is already running"));
        return;
    }

    // the fixtures of the other libraries keep their name: the GDTF one gets " (GDTF)"
    const QString gdtfPath = gdtfDirectory().absolutePath();
    QSet<QString> taken;
    for (QLCFixtureDef *def : m_doc->fixtureDefCache()->defs())
        if (QFileInfo(def->definitionSourceFile()).absolutePath() != gdtfPath)
            taken.insert((def->manufacturer() + '|' + def->model()).toLower());

    m_cancelImport = false;
    m_importProgress = 0;
    m_importErrors.clear();
    setStatus(tr("Converting %n GDTF file(s)…", "", int(jobs.count())));

    const QDir target = gdtfDirectory();
    QThread *thread = QThread::create([this, jobs, target, taken, copied]() {
        QStringList written, errors;
        int failed = 0, warnings = 0;
        std::unique_ptr<QZipReader> zip;
        QString zipPath;
        auto isTaken = [&taken](const QString &manufacturer, const QString &model) {
            return taken.contains((manufacturer + '|' + model).toLower());
        };

        for (int i = 0; i < jobs.count() && m_cancelImport == false; i++)
        {
            const GdtfJob &job = jobs.at(i);
            QByteArray data;
            if (job.entry.isEmpty())
            {
                QFile file(job.path);
                if (file.open(QIODevice::ReadOnly))
                    data = file.readAll();
            }
            else
            {
                if (zipPath != job.path)
                {
                    zip = std::make_unique<QZipReader>(job.path);
                    zipPath = job.path;
                }
                data = GdtfImporter::zipFileData(*zip, job.entry);
            }

            const QString name = QFileInfo(job.entry.isEmpty() ? job.path : job.entry).fileName();
            GdtfImporter::Result result = GdtfImporter::convert(data, target, isTaken);
            if (result.ok)
            {
                written.append(result.file);
                warnings += int(result.warnings.count());
            }
            else
            {
                failed++;
                errors.append(QString("%1: %2").arg(name, result.error));
            }

            if (i % 10 == 9 || i + 1 == jobs.count())
            {
                const qreal progress = qreal(i + 1) / qreal(jobs.count());
                QMetaObject::invokeMethod(this, [this, progress]() {
                    m_importProgress = progress;
                    emit importChanged();
                }, Qt::QueuedConnection);
            }
        }

        QMetaObject::invokeMethod(this, [this, written, failed, warnings, errors, copied]() {
            finishGdtf(written, failed, warnings, errors, copied);
        }, Qt::QueuedConnection);
    });
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    m_importThread = thread;
    emit importChanged();
    thread->start();
}

void FixtureLibrary::finishGdtf(const QStringList &written, int failed, int warnings, const QStringList &errors, int copied)
{
    // the new ones join the library, the ones imported again are updated (the
    // fixtures patched with them too)
    QLCFixtureDefCache *cache = m_doc->fixtureDefCache();
    int added = 0;
    int kept = 0;
    for (const QString &path : written)
    {
        QLCFixtureDef *def = new QLCFixtureDef();
        if (def->loadXML(path) != QFile::NoError)
        {
            delete def;
            continue;
        }
        def->setDefinitionSourceFile(path);
        def->setIsUser(true);
        if (cache->fixtureDef(def->manufacturer(), def->model()) != nullptr)
        {
            kept += installDefinition(def);
        }
        else
        {
            def->setLoaded(true);
            if (cache->addFixtureDef(def))
                added++;
            else
                delete def;
        }
    }

    m_importErrors = errors;
    QString status = m_cancelImport ? tr("GDTF import stopped: %n fixture(s) converted", "", int(written.count()))
                                    : tr("GDTF: %n fixture(s) converted", "", int(written.count()));
    status += " · " + tr("%n new in the library", "", added);
    if (copied > 0)
        status += " · " + tr("%n .qxf file(s) imported", "", copied);
    if (failed > 0)
        status += " · " + tr("%n couldn't be read", "", failed);
    if (warnings > 0)
        status += " · " + tr("%n warning(s)", "", warnings);
    if (kept > 0)
        status += " · " + keptMessage(kept);
    setStatus(status);

    m_importProgress = 1;
    m_importThread = nullptr;
    emit importChanged();
    emit libraryChanged();
}

int FixtureLibrary::installDefinition(QLCFixtureDef *def)
{
    def->setIsUser(true);
    def->setLoaded(true);
    // the old one isn't changed nor deleted: patched fixtures point into it,
    // and what plays may be reading it in the timer thread right now
    QLCFixtureDef *old = m_doc->fixtureDefCache()->replaceFixtureDef(def);
    if (old == nullptr)
        return 0;

    QList<Fixture *> users;
    for (Fixture *fxi : m_doc->fixtures())
    {
        if (fxi->fixtureDef() == old)
            users.append(fxi);
    }
    if (users.isEmpty())
        return 0;

    // each one gets the mode of the same name (or the first one)
    QHash<Fixture *, QLCFixtureMode *> modes;
    for (Fixture *fxi : std::as_const(users))
    {
        QLCFixtureMode *mode = fxi->fixtureMode() != nullptr ? def->mode(fxi->fixtureMode()->name()) : nullptr;
        if (mode == nullptr && def->modes().isEmpty() == false)
            mode = def->modes().first();
        if (mode != nullptr)
            modes.insert(fxi, mode);
    }
    // the channels of a fixture once they all have it
    auto range = [&modes](Fixture *fxi) {
        int channels = modes.contains(fxi) ? int(modes.value(fxi)->channels().count()) : int(fxi->channels());
        return qMakePair(int(fxi->universeAddress()), channels);
    };

    int kept = 0;
    for (Fixture *fxi : std::as_const(users))
    {
        // a mode with more channels than before must fit where the fixture is
        bool fits = modes.contains(fxi);
        const int channels = fits ? int(modes.value(fxi)->channels().count()) : 0;
        if (fits && channels != int(fxi->channels()))
        {
            fits = fxi->crossUniverse() == false && int(fxi->address()) + channels <= UNIVERSE_SIZE;
            const int start = int(fxi->universeAddress());
            for (Fixture *other : m_doc->fixtures())
            {
                if (fits == false)
                    break;
                if (other == fxi)
                    continue;
                QPair<int, int> r = range(other);
                fits = start + channels <= r.first || r.first + r.second <= start;
            }
        }
        if (fits == false)
        {
            kept++;
            continue;
        }
        PatchManager::applyPatch(m_doc, fxi, def, modes.value(fxi), fxi->universe(), fxi->address());
    }

    // the universes learn the new channels (the timer stays on: the show plays on)
    m_doc->refreshChannelCapabilities();
    return kept;
}

void FixtureLibrary::cancelImport()
{
    m_cancelImport = true;
}

bool FixtureLibrary::importing() const
{
    return m_importThread != nullptr;
}

qreal FixtureLibrary::importProgress() const
{
    return m_importProgress;
}

QStringList FixtureLibrary::importErrors() const
{
    return m_importErrors;
}

int FixtureLibrary::gdtfCount() const
{
    const QString gdtfPath = gdtfDirectory().absolutePath();
    int count = 0;
    for (QLCFixtureDef *def : m_doc->fixtureDefCache()->defs())
        if (QFileInfo(def->definitionSourceFile()).absolutePath() == gdtfPath)
            count++;
    return count;
}

int FixtureLibrary::importFromQlcPlus()
{
    // QLC+ keeps the user fixtures in its own user data folder
    QStringList candidates;
    QString home = QDir::homePath();
#if defined(Q_OS_WIN)
    candidates << home + "/QLC+/Fixtures";
#elif defined(Q_OS_MAC)
    candidates << home + "/Library/Application Support/QLC+/Fixtures";
#else
    candidates << home + "/.qlcplus/fixtures";
#endif

    for (const QString &path : candidates)
    {
        if (QDir(path).exists())
            return importFiles(QList<QUrl>() << QUrl::fromLocalFile(path));
    }

    setStatus(tr("No QLC+ user fixtures found"));
    return 0;
}

void FixtureLibrary::downloadOpenFixtureLibrary()
{
    if (m_reply != nullptr)
        return;

    if (m_network == nullptr)
        m_network = new QNetworkAccessManager(this);

    QNetworkRequest request{QUrl(m_oflDownloadUrl)};
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader, QString("%1/%2").arg(APPNAME, APPVERSION));

    m_reply = m_network->get(request);
    m_progress = 0;
    connect(m_reply, &QNetworkReply::downloadProgress, this, &FixtureLibrary::slotDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &FixtureLibrary::slotDownloadFinished);

    setStatus(tr("Downloading the Open Fixture Library…"));
    emit downloadChanged();
}

void FixtureLibrary::cancelDownload()
{
    if (m_reply != nullptr)
        m_reply->abort();
}

void FixtureLibrary::slotDownloadProgress(qint64 received, qint64 total)
{
    m_progress = total > 0 ? qreal(received) / qreal(total) : 0;
    emit downloadChanged();
}

void FixtureLibrary::slotDownloadFinished()
{
    QNetworkReply *reply = m_reply;
    m_reply = nullptr;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        setStatus(reply->error() == QNetworkReply::OperationCanceledError
                  ? tr("Download cancelled")
                  : tr("Download failed: %1").arg(reply->errorString()));
        emit downloadChanged();
        return;
    }

    QString zipPath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                          .filePath("dtc-open-fixture-library.zip");
    QFile zipFile(zipPath);
    if (zipFile.open(QIODevice::WriteOnly) == false)
    {
        setStatus(tr("Unable to write %1").arg(zipPath));
        emit downloadChanged();
        return;
    }
    zipFile.write(reply->readAll());
    zipFile.close();

    // unpacked apart first: a download that can't be read leaves the
    // library as it was
    QTemporaryDir staging;
    const int count = staging.isValid() ? importZip(zipPath, QDir(staging.path())) : 0;
    QFile::remove(zipPath);
    if (count == 0)
        setStatus(tr("The download has no fixtures: the Open Fixture Library stays as it was"));
    else
        setStatus(mergeOpenFixtureLibrary(QDir(staging.path()), count));
    m_progress = 1;
    emit downloadChanged();
}

void FixtureLibrary::openUserFolder() const
{
    QDir dir = QLCFixtureDefCache::userDefinitionDirectory();
    dir.mkpath(".");
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absolutePath()));
}

/*********************************************************************
 * Fixture editor
 *********************************************************************/

QVariantList FixtureLibrary::channelPresets() const
{
    QVariantList list;
    for (int p = QLCChannel::Custom + 1; p < QLCChannel::LastPreset; p++)
    {
        QLCChannel ch;
        ch.setPreset(QLCChannel::Preset(p));

        QVariantMap info;
        info.insert("preset", QLCChannel::presetToString(QLCChannel::Preset(p)));
        info.insert("name", ch.name());
        info.insert("group", QLCChannel::groupToString(ch.group()));
        info.insert("colour", ch.colour() == QLCChannel::NoColour ? QString()
                                                                  : QColor(QRgb(ch.colour())).name());
        info.insert("fine", ch.controlByte() == QLCChannel::LSB);
        list.append(info);
    }
    return list;
}

QStringList FixtureLibrary::channelGroups() const
{
    return QLCChannel::groupList();
}

QStringList FixtureLibrary::fixtureTypes() const
{
    QStringList list;
    for (int t = QLCFixtureDef::ColorChanger; t <= QLCFixtureDef::Strobe; t++)
        list.append(QLCFixtureDef::typeToString(QLCFixtureDef::FixtureType(t)));
    return list;
}

/* What the editor doesn't show travels as the XML of the definition file:
 * saved as it was */
template <typename T> static QString toXml(const T &item)
{
    QString xml;
    QXmlStreamWriter writer(&xml);
    item.saveXML(&writer);
    return xml;
}

static QLCCapability *capabilityFromXml(const QString &xml)
{
    QXmlStreamReader reader(xml);
    if (xml.isEmpty() || reader.readNextStartElement() == false)
        return nullptr;
    QLCCapability *cap = new QLCCapability();
    if (cap->loadXML(reader) == false)
    {
        delete cap;
        return nullptr;
    }
    return cap;
}

static bool physicalFromXml(const QString &xml, QLCPhysical *physical)
{
    QXmlStreamReader reader(xml);
    return xml.isEmpty() == false && reader.readNextStartElement() && physical->loadXML(reader);
}

static QVariantMap channelToMap(const QLCChannel *ch)
{
    QVariantList caps;
    for (QLCCapability *cap : ch->capabilities())
    {
        QVariantMap c;
        c.insert("min", cap->min());
        c.insert("max", cap->max());
        c.insert("name", cap->name());
        // its preset, colours or picture, aliases
        c.insert("xml", toXml(*cap));
        caps.append(c);
    }

    QVariantMap map;
    map.insert("name", ch->name());
    // the heads of the modes know the channels by this one (it may be renamed)
    map.insert("sourceName", ch->name());
    map.insert("preset", QLCChannel::presetToString(ch->preset()));
    map.insert("group", QLCChannel::groupToString(ch->group()));
    map.insert("colour", QLCChannel::colourToString(ch->colour()));
    map.insert("fine", ch->controlByte() == QLCChannel::LSB);
    map.insert("defaultValue", ch->defaultValue());
    map.insert("capabilities", caps);
    return map;
}

QVariantMap FixtureLibrary::newDefinition() const
{
    QVariantList channels;
    QVariantList modeChannels;
    const QLCChannel::Preset presets[] = { QLCChannel::IntensityMasterDimmer, QLCChannel::IntensityRed,
                                           QLCChannel::IntensityGreen, QLCChannel::IntensityBlue };
    for (QLCChannel::Preset p : presets)
    {
        QLCChannel ch;
        ch.setPreset(p);
        modeChannels.append(channels.count());
        channels.append(channelToMap(&ch));
    }

    QVariantMap mode;
    mode.insert("name", QString("%1ch").arg(channels.count()));
    mode.insert("channels", modeChannels);

    QVariantMap def;
    def.insert("manufacturer", QString());
    def.insert("model", QString());
    def.insert("type", QLCFixtureDef::typeToString(QLCFixtureDef::ColorChanger));
    def.insert("channels", channels);
    def.insert("modes", QVariantList() << mode);
    def.insert("isUser", false);
    return def;
}

QVariantMap FixtureLibrary::loadDefinition(const QString &manufacturer, const QString &model) const
{
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef(manufacturer, model);
    if (def == nullptr)
        return QVariantMap();

    QList<QLCChannel *> channelList = def->channels();
    QVariantList channels;
    for (QLCChannel *ch : channelList)
        channels.append(channelToMap(ch));

    QVariantList modes;
    for (QLCFixtureMode *mode : def->modes())
    {
        QVariantList indexes;
        for (QLCChannel *ch : mode->channels())
            indexes.append(channelList.indexOf(ch));

        QVariantMap m;
        m.insert("name", mode->name());
        m.insert("channels", indexes);

        // the heads (cells of a bar, pixels): the channels by name. One head
        // with all the channels is what a fixture makes of a mode without any
        const QVector<QLCFixtureHead> &modeHeads = mode->heads();
        if (modeHeads.count() > 1 ||
            (modeHeads.count() == 1 && modeHeads.first().channels().count() != mode->channels().count()))
        {
            QVariantList heads;
            for (const QLCFixtureHead &head : modeHeads)
            {
                QVariantList names;
                for (quint32 c : head.channels())
                {
                    QLCChannel *ch = mode->channel(c);
                    if (ch != nullptr)
                        names.append(ch->name());
                }
                heads.append(QVariant(names));
            }
            m.insert("heads", heads);
        }
        if (mode->useGlobalPhysical() == false)
            m.insert("physical", toXml(mode->physical()));
        modes.append(m);
    }

    QVariantMap map;
    map.insert("manufacturer", def->manufacturer());
    map.insert("model", def->model());
    map.insert("type", QLCFixtureDef::typeToString(def->type()));
    map.insert("author", def->author());
    map.insert("physical", toXml(def->physical()));
    map.insert("channels", channels);
    map.insert("modes", modes);
    map.insert("isUser", def->isUser());
    return map;
}

QString FixtureLibrary::userFileName(const QString &manufacturer, const QString &model) const
{
    QString name = QString("%1-%2").arg(manufacturer, model);
    name.replace(QRegularExpression("[^A-Za-z0-9_\\-\\.]+"), "-");
    return QLCFixtureDefCache::userDefinitionDirectory().filePath(name + KExtFixture);
}

QString FixtureLibrary::saveDefinition(const QVariantMap &definition)
{
    QString manufacturer = definition.value("manufacturer").toString().trimmed();
    QString model = definition.value("model").toString().trimmed();
    QVariantList channels = definition.value("channels").toList();
    QVariantList modes = definition.value("modes").toList();

    if (manufacturer.isEmpty() || model.isEmpty())
        return tr("Manufacturer and model are required");
    if (channels.isEmpty())
        return tr("The fixture needs at least one channel");
    if (modes.isEmpty())
        return tr("The fixture needs at least one mode");

    QLCFixtureDef def;
    def.setManufacturer(manufacturer);
    def.setModel(model);
    def.setType(QLCFixtureDef::stringToType(definition.value("type").toString()));
    const QString author = definition.value("author").toString().trimmed();
    def.setAuthor(author.isEmpty() ? QLCFile::currentUserName() : author);
    QLCPhysical physical;
    if (physicalFromXml(definition.value("physical").toString(), &physical))
        def.setPhysical(physical);

    QStringList names;
    QList<QLCChannel *> created;
    // the channels by their name when they were loaded (the heads use it)
    QHash<QString, int> bySourceName;
    for (const QVariant &v : channels)
    {
        QVariantMap c = v.toMap();
        QString name = c.value("name").toString().trimmed();
        if (name.isEmpty())
            return tr("Every channel needs a name");
        if (names.contains(name, Qt::CaseInsensitive))
            return tr("There are two channels called %1").arg(name);
        names.append(name);
        const QString sourceName = c.value("sourceName").toString();
        if (sourceName.isEmpty() == false && bySourceName.contains(sourceName) == false)
            bySourceName.insert(sourceName, int(created.count()));

        QLCChannel *ch = new QLCChannel();
        QLCChannel::Preset preset = QLCChannel::stringToPreset(c.value("preset").toString());
        if (preset != QLCChannel::Custom)
        {
            ch->setPreset(preset);
            ch->setName(name);
            ch->addPresetCapability();
        }
        else
        {
            ch->setName(name);
            ch->setGroup(QLCChannel::stringToGroup(c.value("group").toString()));
            if (ch->group() == QLCChannel::Intensity)
                ch->setColour(QLCChannel::stringToColour(c.value("colour").toString()));
            ch->setControlByte(c.value("fine").toBool() ? QLCChannel::LSB : QLCChannel::MSB);

            QVariantList caps = c.value("capabilities").toList();
            if (caps.isEmpty())
                ch->addCapability(new QLCCapability(0, 255, name));
            for (const QVariant &cv : caps)
            {
                QVariantMap cm = cv.toMap();
                int min = qBound(0, cm.value("min").toInt(), 255);
                int max = qBound(min, cm.value("max").toInt(), 255);
                // what it was (preset, colours, picture), with the range and name edited
                QLCCapability *cap = capabilityFromXml(cm.value("xml").toString());
                if (cap == nullptr)
                {
                    cap = new QLCCapability(uchar(min), uchar(max), cm.value("name").toString());
                }
                else
                {
                    cap->setMin(uchar(min));
                    cap->setMax(uchar(max));
                    cap->setName(cm.value("name").toString());
                }
                if (ch->addCapability(cap) == false)
                {
                    delete cap;
                    delete ch;
                    return tr("Overlapping ranges in channel %1").arg(name);
                }
            }
        }
        ch->setDefaultValue(uchar(qBound(0, c.value("defaultValue").toInt(), 255)));

        def.addChannel(ch);
        created.append(ch);
    }

    QStringList modeNames;
    for (const QVariant &v : modes)
    {
        QVariantMap m = v.toMap();
        QString name = m.value("name").toString().trimmed();
        if (name.isEmpty())
            return tr("Every mode needs a name");
        if (modeNames.contains(name, Qt::CaseInsensitive))
            return tr("There are two modes called %1").arg(name);
        modeNames.append(name);

        QVariantList indexes = m.value("channels").toList();
        if (indexes.isEmpty())
            return tr("Mode %1 has no channels").arg(name);

        QLCFixtureMode *mode = new QLCFixtureMode(&def);
        mode->setName(name);
        quint32 pos = 0;
        QList<int> modeChannels;
        for (const QVariant &iv : indexes)
        {
            int idx = iv.toInt();
            if (idx < 0 || idx >= created.count() || mode->insertChannel(created.at(idx), pos) == false)
            {
                delete mode;
                return tr("Mode %1 uses an invalid or repeated channel").arg(name);
            }
            modeChannels.append(idx);
            pos++;
        }

        // its heads, with the channels it still has
        for (const QVariant &hv : m.value("heads").toList())
        {
            QLCFixtureHead head;
            for (const QVariant &channelName : hv.toList())
            {
                const int idx = bySourceName.value(channelName.toString(), -1);
                const int at = idx < 0 ? -1 : int(modeChannels.indexOf(idx));
                if (at < 0)
                    continue;
                head.addChannel(quint32(at));
            }
            if (head.channels().isEmpty() == false)
                mode->insertHead(-1, head);
        }
        QLCPhysical modePhysical;
        if (physicalFromXml(m.value("physical").toString(), &modePhysical))
            mode->setPhysical(modePhysical);
        def.addMode(mode);
    }

    // a user definition is saved in its own file (it may have been imported
    // with another name: a second file would be loaded instead of it, or
    // come back after it is deleted)
    QString path = userFileName(manufacturer, model);
    QLCFixtureDef *existing = m_doc->fixtureDefCache()->fixtureDef(manufacturer, model);
    if (existing != nullptr && existing->isUser() &&
        QFileInfo(existing->definitionSourceFile()).absolutePath() ==
            QLCFixtureDefCache::userDefinitionDirectory().absolutePath())
        path = existing->definitionSourceFile();
    QLCFixtureDefCache::userDefinitionDirectory().mkpath(".");
    if (def.saveXML(path) != QFile::NoError)
        return tr("Unable to write %1").arg(path);

    // reload it from its file, so the cache owns a clean copy
    QLCFixtureDef *saved = new QLCFixtureDef();
    if (saved->loadXML(path) != QFile::NoError)
    {
        delete saved;
        return tr("The saved definition can't be read back");
    }
    saved->setDefinitionSourceFile(path);
    // the fixtures patched with the previous version get this one
    const int kept = installDefinition(saved);

    QString status = tr("%1 %2 saved").arg(manufacturer, model);
    if (kept > 0)
        status += " · " + keptMessage(kept);
    setStatus(status);
    emit libraryChanged();
    return QString();
}

bool FixtureLibrary::deleteUserDefinition(const QString &manufacturer, const QString &model)
{
    QLCFixtureDef *def = m_doc->fixtureDefCache()->fixtureDef(manufacturer, model);
    if (def == nullptr || def->isUser() == false)
        return false;

    // the definition stays in the library (and with the fixtures of the show
    // patched with it) until the next start
    if (QFile::remove(def->definitionSourceFile()) == false)
        return false;

    setStatus(tr("%1 %2 deleted. It will disappear from the library on the next start").arg(manufacturer, model));
    emit libraryChanged();
    return true;
}

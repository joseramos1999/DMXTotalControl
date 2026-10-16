/*
  DMX Total Control
  dtccore.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QStandardPaths>
#include <QCollator>
#include <QGuiApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QTimer>
#include <QTime>
#include <QSaveFile>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "dtccore.h"
#include "zonemanager.h"
#include "patchmanager.h"
#include "programmer.h"
#include "effectexchange.h"
#include "timelineeditor.h"
#include "zone.h"
#include "iomanager.h"

#include "qlcfixturedefcache.h"
#include "grandmaster.h"
#include "qlcmodifierscache.h"
#include "audioplugincache.h"
#include "rgbscriptscache.h"
#include "inputoutputmap.h"
#include "ioplugincache.h"
#include "mastertimer.h"
#include "qlcconfig.h"
#include "function.h"
#include "fixturegroup.h"
#include "rgbmatrix.h"
#include "sequence.h"
#include "showfunction.h"
#include "track.h"
#include "show.h"
#include "qlcfile.h"
#include "doc.h"

DtcCore::DtcCore(QObject *parent)
    : QObject(parent)
    , m_doc(nullptr)
    , m_settings(new AppSettings(this))
    , m_library(nullptr)
    , m_hasRecovery(false)
    , m_backupDone(false)
    , m_clearing(false)
{
    initDoc();

    m_zones = new ZoneManager(m_doc, this);
    m_patch = new PatchManager(m_doc, this);
    m_programmer = new Programmer(m_doc, m_zones, this);
    m_palettes = new PaletteManager(m_doc, m_programmer, m_patch, this);
    m_programmer->setPalettes(m_palettes);
    // cuelists record from the programmer, with its palettes
    m_cuelists = new CuelistEditor(m_doc, m_programmer, m_zones, m_palettes, this);
    m_commands = new CommandLine(m_doc, m_programmer, m_patch, m_palettes, this);
    // GO, RECORD, CUE 3... on the command line work on the selected cuelist
    m_commands->setCuelists(m_cuelists);
    m_io = new IOManager(m_doc, this);
    m_effects = new EffectEditor(m_doc, m_programmer, m_zones, this);
    // (the default priority of the scenes it makes)
    m_effects->setSettings(m_settings);
    m_timeline = new TimelineEditor(m_doc, m_programmer, this);
    m_fxMacros = new FxMacros(m_programmer, this);
    // pages of playbacks: cuelists and scenes with GO, flash and a fader;
    // Record on a playback records into its cuelist
    m_playbacks = new PlaybackManager(m_doc, m_programmer, m_cuelists, m_settings, this);
    // (PB 3 selects a playback; GO and BACK go through the playbacks)
    m_commands->setPlaybacks(m_playbacks);
    m_surface = new ShowSurface(m_doc, m_zones, this);
    m_drive = new DriveControl(m_doc, this);
    m_mixer = new LiveMixer(m_doc, this);
    m_surface->setMixer(m_mixer);
    m_mapping = new ControlMapper(this, this);
    m_surface->setCore(this);
    m_remote = new RemoteServer(this, this);
    // updates from the releases on GitHub (main() starts the check)
    m_updates = new UpdateChecker(this);
    // the LEDs of the known controllers (APC note offs, APC40 mkII Ableton
    // mode), once the event loop runs
    QTimer::singleShot(0, m_mapping, &ControlMapper::initControllers);

    connect(m_settings, SIGNAL(liveEditChanged()), this, SLOT(slotLiveEditChanged()));
    slotLiveEditChanged();

    connect(m_settings, SIGNAL(grandMasterModeChanged()), this, SLOT(slotGrandMasterModeChanged()));
    slotGrandMasterModeChanged();

    // A session that didn't end cleanly may have left an autosave of an
    // untitled show behind
    if (m_settings->sessionRunning() && QFile::exists(recoveryFilePath()))
        m_hasRecovery = true;
    else
        QFile::remove(recoveryFilePath());
    m_settings->setSessionRunning(true);

    m_autosaveTimer = new QTimer(this);
    connect(m_autosaveTimer, SIGNAL(timeout()), this, SLOT(slotAutosave()));
    connect(m_settings, SIGNAL(autosaveChanged()), this, SLOT(slotAutosaveSettingsChanged()));
    slotAutosaveSettingsChanged();
}

DtcCore::~DtcCore()
{
    // the remote control first: it reads everything
    delete m_remote;
    m_remote = nullptr;

    // the controllers don't stay lit: their LEDs go off while the MIDI
    // outputs are still open
    m_mapping->lightsOff();

    // the palettes, the command line, the playbacks and the cuelist editor
    // work on the programmer
    delete m_commands;
    m_commands = nullptr;
    delete m_playbacks;
    m_playbacks = nullptr;
    delete m_cuelists;
    m_cuelists = nullptr;
    m_programmer->setPalettes(nullptr);
    delete m_palettes;
    m_palettes = nullptr;

    // stop the output before tearing down what feeds it
    delete m_programmer;
    m_programmer = nullptr;
    // the identify output of the patch, too: Doc is deleted before the other children
    delete m_patch;
    m_patch = nullptr;
    // and the audio input of the pulses
    delete m_drive;
    m_drive = nullptr;
    // and the movement speed faders of the SHOW
    delete m_surface;
    m_surface = nullptr;
    m_zones->clear();

    m_doc->masterTimer()->stop();
    m_doc->inputOutputMap()->saveDefaults();

    // a clean exit: the recovery file is not needed any more
    if (m_hasRecovery == false)
        QFile::remove(recoveryFilePath());
    m_settings->setSessionRunning(false);
}

void DtcCore::initDoc()
{
    Q_ASSERT(m_doc == nullptr);
    m_doc = new Doc(this);

    connect(m_doc, SIGNAL(modified(bool)), this, SIGNAL(modifiedChanged()));
    connect(m_doc, SIGNAL(functionAdded(quint32)), this, SIGNAL(functionsChanged()));
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SIGNAL(functionsChanged()));
    connect(m_doc, SIGNAL(functionNameChanged(quint32)), this, SIGNAL(functionsChanged()));
    connect(m_doc, SIGNAL(functionReplaced(quint32)), this, SIGNAL(functionsChanged()));
    connect(m_doc->masterTimer(), SIGNAL(functionListChanged()),
            this, SIGNAL(runningFunctionsChanged()));
    connect(m_doc->inputOutputMap(), SIGNAL(blackoutChanged(bool)),
            this, SIGNAL(blackoutChanged()));
    connect(m_doc->inputOutputMap(), SIGNAL(grandMasterValueChanged(uchar)),
            this, SIGNAL(grandMasterChanged()));
    connect(m_doc->inputOutputMap(), SIGNAL(bpmNumberChanged(int)), this, SIGNAL(bpmChanged()));
    connect(m_doc->inputOutputMap(), SIGNAL(beat()), this, SIGNAL(beat()));

    /* User fixtures first, so that they override system fixtures */
    m_doc->fixtureDefCache()->load(QLCFixtureDefCache::userDefinitionDirectory());
    m_doc->fixtureDefCache()->loadMap(QLCFixtureDefCache::systemDefinitionDirectory());
    m_library = new FixtureLibrary(m_doc, this);
    m_library->loadExtraDefinitions();

    m_doc->modifiersCache()->load(QLCModifiersCache::systemTemplateDirectory(), true);
    m_doc->modifiersCache()->load(QLCModifiersCache::userTemplateDirectory());

    m_doc->rgbScriptsCache()->load(RGBScriptsCache::systemScriptsDirectory());
    m_doc->rgbScriptsCache()->load(RGBScriptsCache::userScriptsDirectory());

    /* I/O plugins: DMX USB, Art-Net, E1.31, MIDI, OSC... */
    m_doc->ioPluginCache()->load(IOPluginCache::systemPluginDirectory());
    m_doc->audioPluginCache()->load(QLCFile::systemDirectory(AUDIOPLUGINDIR, KExtPlugin));

    m_doc->inputOutputMap()->loadProfiles(InputOutputMap::userProfileDirectory());
    m_doc->inputOutputMap()->loadProfiles(InputOutputMap::systemProfileDirectory());
    m_doc->inputOutputMap()->loadDefaults();

    m_doc->inputOutputMap()->setBeatGeneratorType(InputOutputMap::Internal);
    m_doc->inputOutputMap()->startUniverses();
    m_doc->masterTimer()->start();

    qInfo() << "Fixture definitions:" << m_doc->fixtureDefCache()->manufacturers().count() << "manufacturers;"
            << "output plugins:" << m_doc->inputOutputMap()->outputPluginNames();
}

Doc *DtcCore::doc() const
{
    return m_doc;
}

ZoneManager *DtcCore::zones() const
{
    return m_zones;
}

PatchManager *DtcCore::patch() const
{
    return m_patch;
}

Programmer *DtcCore::programmer() const
{
    return m_programmer;
}

IOManager *DtcCore::io() const
{
    return m_io;
}

AppSettings *DtcCore::settings() const
{
    return m_settings;
}

FixtureLibrary *DtcCore::library() const
{
    return m_library;
}

EffectEditor *DtcCore::effects() const
{
    return m_effects;
}

TimelineEditor *DtcCore::timeline() const
{
    return m_timeline;
}

FxMacros *DtcCore::fxMacros() const
{
    return m_fxMacros;
}

RemoteServer *DtcCore::remote() const
{
    return m_remote;
}

UpdateChecker *DtcCore::updates() const
{
    return m_updates;
}

ShowSurface *DtcCore::surface() const
{
    return m_surface;
}

DriveControl *DtcCore::drive() const
{
    return m_drive;
}

LiveMixer *DtcCore::mixer() const
{
    return m_mixer;
}

ControlMapper *DtcCore::mapping() const
{
    return m_mapping;
}

PaletteManager *DtcCore::palettes() const
{
    return m_palettes;
}

CommandLine *DtcCore::commands() const
{
    return m_commands;
}

CuelistEditor *DtcCore::cuelists() const
{
    return m_cuelists;
}

PlaybackManager *DtcCore::playbacks() const
{
    return m_playbacks;
}

void DtcCore::slotLiveEditChanged()
{
    m_programmer->setLivePriority(m_settings->liveEditPriority());
    m_programmer->setReleaseMode(m_settings->liveEditRelease());
}

QString DtcCore::appName() const
{
    return QString(APPNAME);
}

QString DtcCore::appVersion() const
{
    return QString(APPVERSION);
}

QString DtcCore::fileName() const
{
    return m_fileName;
}

QString DtcCore::showTitle() const
{
    if (m_fileName.isEmpty())
        return tr("Untitled show");
    return QFileInfo(m_fileName).completeBaseName();
}

void DtcCore::setFileName(const QString &fileName)
{
    if (fileName == m_fileName)
        return;
    m_fileName = fileName;
    emit fileNameChanged();
}

bool DtcCore::modified() const
{
    return m_doc->isModified();
}

QString DtcCore::lastError() const
{
    return m_lastError;
}

void DtcCore::setLastError(const QString &error)
{
    m_lastError = error;
    emit lastErrorChanged();
}

bool DtcCore::blackout() const
{
    return m_doc->inputOutputMap()->blackout();
}

void DtcCore::setBlackout(bool enable)
{
    m_doc->inputOutputMap()->setBlackout(enable);
}

int DtcCore::grandMaster() const
{
    return m_doc->inputOutputMap()->grandMasterValue();
}

void DtcCore::setGrandMaster(int value)
{
    m_doc->inputOutputMap()->setGrandMasterValue(uchar(qBound(0, value, 255)));
}

int DtcCore::bpm() const
{
    return m_doc->inputOutputMap()->bpmNumber();
}

void DtcCore::setBpm(int bpm)
{
    bpm = qBound(20, bpm, 300);
    if (bpm == this->bpm())
        return;
    m_doc->inputOutputMap()->setBpmNumber(bpm);
    m_doc->setModified();
}

void DtcCore::tapTempo()
{
    // a pause longer than 2 s starts a new tap sequence
    qint64 elapsed = m_tapTimer.isValid() ? m_tapTimer.restart() : -1;
    if (m_tapTimer.isValid() == false)
        m_tapTimer.start();
    if (elapsed < 0 || elapsed > 2000)
    {
        m_tapIntervals.clear();
        return;
    }

    m_tapIntervals.append(elapsed);
    while (m_tapIntervals.count() > 4)
        m_tapIntervals.removeFirst();

    qint64 sum = 0;
    for (qint64 interval : std::as_const(m_tapIntervals))
        sum += interval;
    setBpm(qRound(60000.0 / (qreal(sum) / m_tapIntervals.count())));
}

int DtcCore::runningFunctions() const
{
    // the FX of the programmer are live edits, not a scene
    int count = m_doc->masterTimer()->runningFunctions();
    return m_programmer->isFxRunning() ? qMax(0, count - 1) : count;
}

QVariantList DtcCore::functions() const
{
    QVariantList list;
    for (Function *f : m_doc->functions())
    {
        // hide the internal functions (e.g. sequences' bound scenes)
        if (f->isVisible() == false)
            continue;

        QVariantMap info;
        info.insert("id", f->id());
        info.insert("name", f->name());
        info.insert("type", Function::typeToString(f->type()));
        info.insert("priority", f->priority());
        info.insert("folder", f->path(true));
        list.append(info);
    }
    return list;
}

QVariantMap DtcCore::functionInfo(quint32 id) const
{
    QVariantMap info;
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return info;

    info.insert("id", f->id());
    info.insert("name", f->name());
    info.insert("type", Function::typeToString(f->type()));
    info.insert("fadeIn", f->fadeInSpeed());
    info.insert("fadeOut", f->fadeOutSpeed());
    info.insert("priority", f->priority());
    info.insert("running", f->isRunning());
    if (Cuelist *cuelist = qobject_cast<Cuelist *>(f))
        info.insert("cueCount", cuelist->cueCount());
    return info;
}

void DtcCore::renameFunction(quint32 id, const QString &name)
{
    Function *f = m_doc->function(id);
    if (f == nullptr || name.trimmed().isEmpty())
        return;
    f->setName(name.trimmed());
    m_doc->setModified();
}

void DtcCore::deleteFunction(quint32 id)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return;
    f->stopAndWait();

    // what was made only for this function goes with it
    quint32 boundScene = Function::invalidId();
    quint32 fxGroup = FixtureGroup::invalidId();
    if (Sequence *sequence = qobject_cast<Sequence *>(f))
        boundScene = sequence->boundSceneID();
    if (RGBMatrix *matrix = qobject_cast<RGBMatrix *>(f))
    {
        FixtureGroup *group = m_doc->fixtureGroup(matrix->fixtureGroup());
        if (group != nullptr && (group->name().endsWith(" (FX)") || group->name().endsWith(" (FX 2D)")))
            fxGroup = group->id();
    }

    // its blocks in the super scenes go (a playing super scene forgets them
    // and plays on): a function made later may get its ID. (The timeline
    // editor follows functionRemoved)
    for (Function *other : m_doc->functionsByType(Function::ShowType))
    {
        Show *show = qobject_cast<Show *>(other);
        if (show == nullptr || other == f)
            continue;
        for (Track *track : show->tracks())
        {
            for (ShowFunction *sf : track->showFunctions())
            {
                if (sf->functionID() == id)
                    track->removeShowFunction(sf, true);
            }
        }
    }

    m_doc->deleteFunction(id);

    // only when nothing else uses it (QLC+ steps can be bound to a user scene)
    if (boundScene != Function::invalidId())
        EffectEditor::deleteBoundScene(m_doc, boundScene);
    if (fxGroup != FixtureGroup::invalidId())
    {
        bool shared = false;
        for (Function *other : m_doc->functionsByType(Function::RGBMatrixType))
        {
            RGBMatrix *m = qobject_cast<RGBMatrix *>(other);
            if (m != nullptr && m->fixtureGroup() == fxGroup)
                shared = true;
        }
        if (shared == false)
            m_doc->deleteFixtureGroup(fxGroup);
    }
}

void DtcCore::setFunctionFades(quint32 id, int fadeIn, int fadeOut)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return;
    f->setFadeInSpeed(uint(qMax(0, fadeIn)));
    f->setFadeOutSpeed(uint(qMax(0, fadeOut)));
    m_doc->setModified();
}

void DtcCore::setFunctionPriority(quint32 id, int priority)
{
    Function *f = m_doc->function(id);
    if (f == nullptr)
        return;
    f->setPriority(priority);
    m_doc->setModified();
    emit functionsChanged();
}

/*********************************************************************
 * Scene folders
 *********************************************************************/

QStringList DtcCore::sceneFolders() const
{
    QStringList folders = m_folders;
    for (Function *f : m_doc->functions())
    {
        QString folder = f->path(true);
        if (f->isVisible() && folder.isEmpty() == false && folders.contains(folder) == false)
            folders.append(folder);
    }

    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(folders.begin(), folders.end(), collator);
    return folders;
}

bool DtcCore::addSceneFolder(const QString &name)
{
    QString folder = name.trimmed();
    if (folder.isEmpty() || sceneFolders().contains(folder))
        return false;
    m_folders.append(folder);
    m_doc->setModified();
    emit functionsChanged();
    return true;
}

bool DtcCore::renameSceneFolder(const QString &name, const QString &newName)
{
    QString folder = newName.trimmed();
    if (folder.isEmpty() || folder == name)
        return false;

    m_folders.removeAll(name);
    if (m_folders.contains(folder) == false)
        m_folders.append(folder);
    for (Function *f : m_doc->functions())
    {
        if (f->path(true) == name)
            f->setPath(folder);
    }
    m_doc->setModified();
    emit functionsChanged();
    return true;
}

void DtcCore::removeSceneFolder(const QString &name)
{
    m_folders.removeAll(name);
    for (Function *f : m_doc->functions())
    {
        if (f->path(true) == name)
            f->setPath(QString());
    }
    m_doc->setModified();
    emit functionsChanged();
}

void DtcCore::setFunctionFolder(const QVariantList &ids, const QString &folder)
{
    QString name = folder.trimmed();
    if (name.isEmpty() == false && m_folders.contains(name) == false)
        m_folders.append(name);
    for (const QVariant &id : ids)
    {
        Function *f = m_doc->function(id.toUInt());
        if (f != nullptr)
            f->setPath(name);
    }
    m_doc->setModified();
    emit functionsChanged();
}

void DtcCore::stopAll()
{
    // the flashes held on playbacks are let go (their levels stay)
    m_playbacks->resetLive();
    m_zones->stopAll();
    m_doc->masterTimer()->stopAllFunctions();
    // the FX of the programmer are live edits, not a playback: they go on
    m_programmer->restartFx();
}

/*********************************************************************
 * Show files
 *********************************************************************/

void DtcCore::clearShow()
{
    m_zones->clear();
    m_playbacks->clear();
    m_cuelists->clear();
    m_surface->clear();
    m_mapping->clear();
    m_patch->clearSelections();
    m_patch->clearPlan();
    m_palettes->clear();
    m_folders.clear();
    m_foreignSections.clear();
    m_doc->masterTimer()->stop();
    m_clearing = true;
    m_doc->clearContents();
    m_clearing = false;
    m_doc->inputOutputMap()->resetUniverses();
    // (resetUniverses sets the grand master back to intensity only)
    slotGrandMasterModeChanged();
    // the files of a new show aren't relative to the folder of the last one
    m_doc->setWorkspacePath(QString());
    m_doc->resetModified();
    m_doc->inputOutputMap()->startUniverses();
    m_doc->masterTimer()->start();
    setFileName(QString());
    // this show was never autosaved
    if (m_lastAutosave.isEmpty() == false)
    {
        m_lastAutosave.clear();
        emit lastAutosaveChanged();
    }
}

void DtcCore::newShow()
{
    clearShow();
    emit functionsChanged();
    emit modifiedChanged();
}

/** The highest "paletteId" in $value (maps and lists of maps), -1 for none */
static int highestPaletteId(const QVariant &value)
{
    int highest = -1;
    if (value.typeId() == QMetaType::QVariantMap)
    {
        const QVariantMap map = value.toMap();
        for (auto it = map.cbegin(); it != map.cend(); ++it)
            highest = qMax(highest, it.key() == QLatin1String("paletteId") ? it.value().toInt()
                                                                            : highestPaletteId(it.value()));
    }
    else if (value.typeId() == QMetaType::QVariantList)
    {
        for (const QVariant &v : value.toList())
            highest = qMax(highest, highestPaletteId(v));
    }
    return highest;
}

bool DtcCore::openShow(const QUrl &url)
{
    QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
    if (path.isEmpty())
        return false;

    QXmlStreamReader *reader = QLCFile::getXMLReader(path);
    if (reader == nullptr || reader->device() == nullptr || reader->hasError())
    {
        setLastError(tr("Unable to read %1").arg(path));
        return false;
    }

    while (!reader->atEnd())
    {
        if (reader->readNext() == QXmlStreamReader::DTD)
            break;
    }

    // (a file cut right after its header has no workspace: the show that is
    // open stays, like for any file that isn't a show)
    if (reader->hasError() || reader->dtdName() != KXMLDTCWorkspace ||
        reader->readNextStartElement() == false || reader->name() != KXMLDTCWorkspace)
    {
        QLCFile::releaseXMLReader(reader);
        setLastError(tr("%1 is not a show file").arg(path));
        return false;
    }

    clearShow();
    // local resources (audio, video...) are relative to the show file
    m_doc->setWorkspacePath(QFileInfo(path).absolutePath());

    // loading replaces the universes with the ones of the show: the new ones
    // must be started, or nothing reaches the outputs
    m_doc->masterTimer()->stop();
    bool ok = loadWorkspace(*reader);
    // a damaged file (an XML error halfway, a truncated file) stops the
    // loading there without a word: what came after it is missing
    const bool damaged = ok && reader->hasError();
    const QString damage = damaged ? tr("line %1: %2").arg(reader->lineNumber()).arg(reader->errorString())
                                   : QString();
    QLCFile::releaseXMLReader(reader);
    m_doc->inputOutputMap()->startUniverses();
    m_doc->masterTimer()->start();
    // the controllers were patched again: their LEDs get ready again
    m_mapping->initControllers();

    if (ok == false)
    {
        setLastError(tr("Error while loading %1").arg(path));
        return false;
    }

    // what names something that isn't there (a damaged or edited file) lets
    // it go, as when it is deleted: a function made later gets its ID. A
    // jump to a missing function jumps nowhere; the cues that name a missing
    // palette keep plain values (the palette IDs the SHOW and the mappings
    // name stay taken)
    for (Function *f : m_doc->functions())
    {
        if (f->jumpTo() >= 0 && m_doc->function(quint32(f->jumpTo())) == nullptr)
            f->setJumpTo(Function::jumpNone());
    }
    m_palettes->afterLoad(qMax(highestPaletteId(m_surface->pages()), highestPaletteId(m_mapping->mappings())));

    // give every fixture a spot in the 2D plan (shows from QLC+ may have none)
    m_patch->autoLayout(false);
    // BPM, tap tempo and beat-following scenes need a beat generator:
    // shows coming from QLC+ often have it disabled
    if (m_doc->inputOutputMap()->beatGeneratorType() == InputOutputMap::Disabled)
        m_doc->inputOutputMap()->setBeatGeneratorType(InputOutputMap::Internal);
    if (m_doc->inputOutputMap()->bpmNumber() < 20)
        m_doc->inputOutputMap()->setBpmNumber(120);

    if (damaged)
    {
        // what could be read stays, as an untitled show: nothing (autosave,
        // Save) writes the part over the whole file
        setFileName(QString());
        m_doc->setModified();
        setLastError(tr("%1 is damaged (%2): only part of it was loaded. Save it under another name to keep this part.")
                     .arg(QFileInfo(path).fileName(), damage));
        emit functionsChanged();
        emit modifiedChanged();
        return false;
    }

    setFileName(path);
    m_doc->resetModified();
    m_backupDone = false;
    if (path != recoveryFilePath())
        m_settings->addRecentShow(path);
    emit functionsChanged();
    emit modifiedChanged();
    return true;
}

QString DtcCore::importFromShow(const QUrl &url, bool patch, bool scenes)
{
    QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
    QList<EffectExchange::ShowZone> zones;
    QString error;
    Doc *other = EffectExchange::loadShow(m_doc, path, &zones, &error);
    if (other == nullptr)
    {
        setLastError(error);
        return QString();
    }

    QStringList done;
    // the patch first: the scenes then go on the copies of their fixtures
    // (not on fixtures of this show at the same address)
    QHash<quint32, quint32> copies;
    if (patch)
        done.append(tr("%n fixture(s)", "", m_patch->importFixtures(other, &copies)));

    if (scenes)
    {
        // what can be played: the hidden channels of steps come with their steps
        QList<quint32> roots;
        for (Function *f : other->functions())
        {
            if (f->isVisible() == false)
                continue;
            switch (f->type())
            {
            case Function::SceneType: case Function::ChaserType: case Function::SequenceType:
            case Function::EFXType: case Function::RGBMatrixType: case Function::CollectionType:
            case Function::CurveFXType:
            case Function::FXSceneType:
            case Function::CuelistType:
                roots.append(f->id());
                break;
            default:
                break;
            }
        }

        int missing = 0;
        QHash<quint32, quint32> functionMap = EffectExchange::importFunctions(m_doc, other, roots, &missing, copies);
        int imported = 0;
        for (quint32 fid : std::as_const(roots))
            imported += functionMap.contains(fid) ? 1 : 0;

        // their groups: into the group of the same name, or a new one
        for (const EffectExchange::ShowZone &source : std::as_const(zones))
        {
            QList<quint32> ids;
            for (quint32 fid : source.functions)
                if (functionMap.contains(fid))
                    ids.append(functionMap.value(fid));
            if (ids.isEmpty())
                continue;

            Zone *zone = nullptr;
            for (int i = 0; i < m_zones->count() && zone == nullptr; i++)
                if (m_zones->zone(i)->name() == source.name)
                    zone = m_zones->zone(i);
            if (zone == nullptr)
            {
                zone = m_zones->addZone(source.name);
                if (QColor(source.color).isValid())
                    zone->setColor(QColor(source.color));
                zone->setExclusive(source.exclusive);
            }
            for (quint32 id : std::as_const(ids))
                zone->addFunction(id);
        }

        done.append(tr("%n scene(s)", "", imported));
        if (missing > 0)
            done.append(tr("%n fixture(s) not found in this show", "", missing));
    }
    delete other;

    m_doc->setModified();
    emit functionsChanged();
    emit modifiedChanged();
    return tr("Imported: %1").arg(done.join(", "));
}

bool DtcCore::loadWorkspace(QXmlStreamReader &reader)
{
    // (openShow found the Workspace node)
    if (reader.isStartElement() == false || reader.name() != KXMLDTCWorkspace)
    {
        qWarning() << Q_FUNC_INFO << "Workspace node not found";
        return false;
    }

    while (reader.readNextStartElement())
    {
        if (reader.name() == KXMLQLCEngine)
        {
            m_doc->loadXML(reader);
        }
        else if (reader.name() == KXMLDTCSection)
        {
            loadSection(reader);
        }
        else if (reader.name() == KXMLQLCCreator)
        {
            // (each save writes its own)
            reader.skipCurrentElement();
        }
        else
        {
            // the QLC+ UI sections (Virtual Console, Simple Desk...): not used
            // here, kept as they are so that saving gives them back to QLC+
            qDebug() << "Keeping workspace section" << reader.name().toString();
            QByteArray section = copyElement(reader);
            if (reader.hasError() == false)
                m_foreignSections.append(section);
        }
    }

    return true;
}

QByteArray DtcCore::copyElement(QXmlStreamReader &reader)
{
    // from its start to its end element, which stays the current token
    // (like skipCurrentElement)
    QByteArray xml;
    QXmlStreamWriter writer(&xml);
    int depth = 0;
    while (reader.hasError() == false)
    {
        if (reader.isStartElement())
            depth++;
        else if (reader.isEndElement())
            depth--;
        copyToken(reader, writer);
        if (depth <= 0 || reader.atEnd())
            break;
        reader.readNext();
    }
    return xml;
}

void DtcCore::copyToken(QXmlStreamReader &reader, QXmlStreamWriter &writer)
{
    // by qualified names: the elements stay in the namespace of the workspace
    switch (reader.tokenType())
    {
        case QXmlStreamReader::StartElement:
            writer.writeStartElement(reader.qualifiedName().toString());
            for (const QXmlStreamAttribute &attr : reader.attributes())
                writer.writeAttribute(attr.qualifiedName().toString(), attr.value().toString());
        break;
        case QXmlStreamReader::EndElement:
            writer.writeEndElement();
        break;
        case QXmlStreamReader::Characters:
            if (reader.isCDATA())
                writer.writeCDATA(reader.text().toString());
            else if (reader.isWhitespace() == false)
                writer.writeCharacters(reader.text().toString());
        break;
        case QXmlStreamReader::Comment:
            writer.writeComment(reader.text().toString());
        break;
        case QXmlStreamReader::ProcessingInstruction:
            writer.writeProcessingInstruction(reader.processingInstructionTarget().toString(),
                                              reader.processingInstructionData().toString());
        break;
        case QXmlStreamReader::EntityReference:
            writer.writeEntityReference(reader.name().toString());
        break;
        default:
        break;
    }
}

void DtcCore::loadSection(QXmlStreamReader &reader)
{
    while (reader.readNextStartElement())
    {
        if (reader.name() == KXMLDTCZones)
            m_zones->loadXML(reader);
        else if (reader.name() == KXMLDTCPlaybacks)
            m_playbacks->loadXML(reader);
        else if (reader.name() == KXMLDTCSurface)
            m_surface->loadXML(reader);
        else if (reader.name() == KXMLDTCMappings)
            m_mapping->loadXML(reader);
        else if (reader.name() == KXMLDTCPalettes)
            m_palettes->loadXML(reader);
        else if (reader.name() == KXMLDTCSelections || reader.name() == KXMLDTCPlan ||
                 reader.name() == KXMLDTCNumbers)
            m_patch->loadXML(reader);
        else if (reader.name() == KXMLDTCFolders)
        {
            while (reader.readNextStartElement())
            {
                QString name = reader.attributes().value("Name").toString();
                if (reader.name() == KXMLDTCFolder && name.isEmpty() == false && m_folders.contains(name) == false)
                    m_folders.append(name);
                reader.skipCurrentElement();
            }
        }
        else
            reader.skipCurrentElement();
    }
}

bool DtcCore::writeWorkspace(const QString &path)
{
    // written aside, and put in place of the show only when all of it was
    // written (a full disk, a drive gone: the show stays as it was)
    QSaveFile file(path);
    if (file.open(QIODevice::WriteOnly) == false)
    {
        setLastError(tr("Unable to write %1 (%2)").arg(path, file.errorString()));
        return false;
    }

    QXmlStreamWriter doc(&file);
    doc.setAutoFormatting(true);
    doc.setAutoFormattingIndent(1);

    doc.writeStartDocument();
    doc.writeDTD(QString("<!DOCTYPE %1>").arg(KXMLDTCWorkspace));

    doc.writeStartElement(KXMLDTCWorkspace);
    doc.writeAttribute("xmlns", QString("%1%2").arg(KXMLQLCplusNamespace, KXMLDTCWorkspace));

    doc.writeStartElement(KXMLQLCCreator);
    doc.writeTextElement(KXMLQLCCreatorName, APPNAME);
    doc.writeTextElement(KXMLQLCCreatorVersion, APPVERSION);
    doc.writeTextElement(KXMLQLCCreatorAuthor, QLCFile::currentUserName());
    doc.writeEndElement();

    m_doc->saveXML(&doc);

    // the sections of QLC+ (Virtual Console, Simple Desk...) of the show opened
    for (const QByteArray &section : std::as_const(m_foreignSections))
    {
        QXmlStreamReader reader(section);
        while (reader.atEnd() == false && reader.hasError() == false)
        {
            reader.readNext();
            copyToken(reader, doc);
        }
    }

    doc.writeStartElement(KXMLDTCSection);
    m_zones->saveXML(&doc);
    m_playbacks->saveXML(&doc);
    m_surface->saveXML(&doc);
    m_mapping->saveXML(&doc);
    m_patch->saveXML(&doc);
    m_palettes->saveXML(&doc);
    // the scenes keep their folder (the function path); this list keeps the empty ones
    const QStringList folders = sceneFolders();
    if (folders.isEmpty() == false)
    {
        doc.writeStartElement(KXMLDTCFolders);
        for (const QString &folder : folders)
        {
            doc.writeStartElement(KXMLDTCFolder);
            doc.writeAttribute("Name", folder);
            doc.writeEndElement();
        }
        doc.writeEndElement();
    }
    doc.writeEndElement();

    doc.writeEndElement(); // Workspace
    doc.writeEndDocument();

    if (doc.hasError() || file.error() != QFileDevice::NoError)
    {
        QString error = file.errorString();
        file.cancelWriting();
        setLastError(tr("Unable to write %1 (%2)").arg(path, error));
        return false;
    }
    // (in place of the show at once: never a moment without it)
    if (file.commit() == false)
    {
        setLastError(tr("Unable to overwrite %1 (%2)").arg(path, file.errorString()));
        return false;
    }
    return true;
}

bool DtcCore::saveShow(const QUrl &url)
{
    QString path = m_fileName;
    if (url.isEmpty() == false)
        path = url.isLocalFile() ? url.toLocalFile() : url.toString();

    if (path.isEmpty())
    {
        setLastError(tr("No file name given"));
        return false;
    }
    if (path.endsWith(KExtWorkspace, Qt::CaseInsensitive) == false)
        path += KExtWorkspace;

    m_doc->setWorkspacePath(QFileInfo(path).absolutePath());
    if (writeWorkspace(path) == false)
        return false;

    if (path != m_fileName)
        m_backupDone = false;
    setFileName(path);
    m_doc->resetModified();
    m_settings->addRecentShow(path);

    // the show is safe on disk now
    QFile::remove(recoveryFilePath());
    return true;
}

void DtcCore::openStartupShow()
{
    if (m_hasRecovery || m_fileName.isEmpty() == false || m_settings->openLastShow() == false)
        return;

    QStringList recent = m_settings->recentShows();
    if (recent.isEmpty() == false)
        openShow(QUrl::fromLocalFile(recent.first()));
}

/*********************************************************************
 * Autosave & recovery
 *********************************************************************/

QString DtcCore::recoveryFilePath() const
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    dir.mkpath(".");
    return dir.filePath("autosave" + KExtWorkspace);
}

bool DtcCore::hasRecovery() const
{
    return m_hasRecovery;
}

bool DtcCore::recoverShow()
{
    if (m_hasRecovery == false)
        return false;

    bool ok = openShow(QUrl::fromLocalFile(recoveryFilePath()));
    m_hasRecovery = false;
    emit hasRecoveryChanged();

    if (ok)
    {
        // it is still an untitled show, that needs to be saved
        setFileName(QString());
        m_doc->setModified();
    }
    return ok;
}

void DtcCore::discardRecovery()
{
    QFile::remove(recoveryFilePath());
    m_hasRecovery = false;
    emit hasRecoveryChanged();
}

QString DtcCore::lastAutosave() const
{
    return m_lastAutosave;
}

void DtcCore::slotAutosaveSettingsChanged()
{
    if (m_settings->autosaveEnabled())
        m_autosaveTimer->start(m_settings->autosaveMinutes() * 60 * 1000);
    else
        m_autosaveTimer->stop();
}

void DtcCore::slotAutosave()
{
    if (m_doc->isModified() == false)
        return;

    bool ok;
    if (m_fileName.isEmpty())
    {
        // untitled shows go to a recovery file, offered back after a crash.
        // Its files (images, audio) are saved relative to the recovery file,
        // where they are found again (in memory they are absolute)
        const QString workspace = m_doc->workspacePath();
        m_doc->setWorkspacePath(QFileInfo(recoveryFilePath()).absolutePath());
        ok = writeWorkspace(recoveryFilePath());
        m_doc->setWorkspacePath(workspace);
    }
    else
    {
        // keep the version the user saved last, once per session (the old
        // backup goes only once the new one is there)
        if (m_backupDone == false)
        {
            const QString backup = m_fileName + ".bak";
            const QString fresh = backup + ".new";
            QFile::remove(fresh);
            if (QFile::copy(m_fileName, fresh))
            {
                QFile::remove(backup);
                m_backupDone = QFile::rename(fresh, backup);
            }
        }
        ok = writeWorkspace(m_fileName);
        if (ok)
            m_doc->resetModified();
    }

    if (ok)
    {
        m_lastAutosave = QTime::currentTime().toString("HH:mm");
        emit lastAutosaveChanged();
    }
    else
    {
        // nothing protects the show any more: the user must know
        emit autosaveFailed(m_lastError);
    }
}

void DtcCore::slotFunctionRemoved(quint32 id)
{
    if (m_clearing)
        return;
    // the functions that jumped to it jump nowhere (a function made later
    // may get its ID)
    for (Function *f : m_doc->functions())
    {
        if (f->jumpTo() >= 0 && quint32(f->jumpTo()) == id)
            f->setJumpTo(Function::jumpNone());
    }
}

QUrl DtcCore::localFileUrl(const QString &path) const
{
    return QUrl::fromLocalFile(path);
}

void DtcCore::slotGrandMasterModeChanged()
{
    m_doc->inputOutputMap()->setGrandMasterChannelMode(m_settings->grandMasterIntensityOnly()
                                                       ? GrandMaster::Intensity
                                                       : GrandMaster::AllChannels);
}

void DtcCore::setControlValue(QObject *control, qreal value)
{
    if (control)
        control->setProperty("value", value);
}

void DtcCore::copyToClipboard(const QString &text)
{
    if (QGuiApplication::clipboard() != nullptr)
        QGuiApplication::clipboard()->setText(text);
}

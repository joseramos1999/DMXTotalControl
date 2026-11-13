/*
  DMX Total Control
  playbackmanager.cpp

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
#include <QColor>
#include <QTimer>
#include <QDebug>

#include "playbackmanager.h"
#include "cuelisteditor.h"
#include "appsettings.h"
#include "programmer.h"

#include "chaseraction.h"
#include "mastertimer.h"
#include "cuelist.h"
#include "chaser.h"
#include "show.h"
#include "doc.h"

#define KXMLDTCPlaybackSlotsPerPage QStringLiteral("SlotsPerPage")
#define KXMLDTCPlaybackCurrentPage  QStringLiteral("CurrentPage")
#define KXMLDTCPlaybackPage         QStringLiteral("Page")
#define KXMLDTCPlaybackPageName     QStringLiteral("Name")
#define KXMLDTCPlaybackSlot         QStringLiteral("Slot")
#define KXMLDTCPlaybackIndex        QStringLiteral("Index")
#define KXMLDTCPlaybackFunction     QStringLiteral("Function")
#define KXMLDTCPlaybackLevel        QStringLiteral("Level")
#define KXMLDTCPlaybackLabel        QStringLiteral("Label")
#define KXMLDTCPlaybackColor        QStringLiteral("Color")
#define KXMLDTCPlaybackButton1      QStringLiteral("Button1")
#define KXMLDTCPlaybackButton2      QStringLiteral("Button2")
#define KXMLDTCPlaybackButton3      QStringLiteral("Button3")
#define KXMLDTCPlaybackFaderStarts  QStringLiteral("FaderStarts")
#define KXMLDTCPlaybackFaderRelease QStringLiteral("FaderReleases")
#define KXMLDTCPlaybackFlashAttack  QStringLiteral("FlashAttack")
#define KXMLDTCPlaybackFlashDecay   QStringLiteral("FlashDecay")
#define KXMLDTCPlaybackFaderMode    QStringLiteral("FaderMode")

namespace
{
/** Below this, a fader is down: moving it up starts the playback, and GO plays at full */
const qreal kFaderDown = 0.05;
/** An A/B fader this near an end is at that end (a controller fader jitters there) */
const qreal kCrossfadeEdge = 0.01;
const int kDefaultSlots = 10;
const int kMinSlots = 4;
/** How often a flash attack or decay moves on, ms */
const int kRampTick = 20;
}

PlaybackManager::PlaybackManager(Doc *doc, Programmer *programmer, CuelistEditor *cuelists,
                                 AppSettings *settings, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_programmer(programmer)
    , m_cuelists(cuelists)
    , m_settings(settings)
    , m_currentPage(0)
    , m_slotsPerPage(kDefaultSlots)
    , m_selectedPage(-1)
    , m_selectedSlot(-1)
    , m_lastGoPage(-1)
    , m_lastGoSlot(-1)
    , m_revision(0)
    , m_progressRevision(0)
{
    Q_ASSERT(m_doc != nullptr);
    m_pages.append(defaultPage(1));

    // what the playbacks show is read while something plays
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(100);
    connect(m_pollTimer, &QTimer::timeout, this, &PlaybackManager::slotPoll);

    // the flash attacks and decays, only while one runs
    m_rampClock.start();
    m_rampTimer = new QTimer(this);
    m_rampTimer->setInterval(kRampTick);
    m_rampTimer->setTimerType(Qt::PreciseTimer);
    connect(m_rampTimer, &QTimer::timeout, this, &PlaybackManager::slotFlashRamps);

    // engine signals go by name (MinGW)
    connect(m_doc, SIGNAL(functionRemoved(quint32)), this, SLOT(slotFunctionRemoved(quint32)));
    connect(m_doc, SIGNAL(functionReplaced(quint32)), this, SLOT(slotFunctionReplaced(quint32)));
    connect(m_doc, SIGNAL(functionChanged(quint32)), this, SLOT(slotFunctionChanged(quint32)));
    connect(m_doc, SIGNAL(cleared()), this, SLOT(slotDocCleared()));
    connect(m_doc->masterTimer(), SIGNAL(functionListChanged()), this, SLOT(slotFunctionListChanged()));
    // a GO inside a running cuelist (or a Follow) changes no running state: tell it at once
    connect(m_doc->masterTimer(), SIGNAL(cueStarted(quint32,quint32)),
            this, SLOT(slotCueStarted(quint32,quint32)));
    // the cue speed lives in the timer: whoever sets it (the SHOW, a mapping)
    connect(m_doc->masterTimer(), SIGNAL(cueSpeedChanged()), this, SIGNAL(cueSpeedChanged()));
}

PlaybackManager::~PlaybackManager()
{
    m_pollTimer->stop();
    m_rampTimer->stop();
}

int PlaybackManager::revision() const
{
    return m_revision;
}

int PlaybackManager::progressRevision() const
{
    return m_progressRevision;
}

void PlaybackManager::bump()
{
    m_revision++;
    emit stateChanged();
}

PlaybackManager::Page PlaybackManager::defaultPage(int number) const
{
    Page page;
    page.name = tr("Page %1").arg(number);
    return page;
}

/*********************************************************************
 * Pages
 *********************************************************************/

int PlaybackManager::currentPage() const
{
    return m_currentPage;
}

void PlaybackManager::setCurrentPage(int page)
{
    page = qBound(0, page, int(m_pages.count()) - 1);
    if (page == m_currentPage)
        return;
    // the slots of page -1 are other ones now
    m_currentPage = page;
    emit pageChanged();
    bump();
}

int PlaybackManager::pageCount() const
{
    return int(m_pages.count());
}

QStringList PlaybackManager::pageNames() const
{
    QStringList names;
    for (const Page &page : m_pages)
        names.append(page.name);
    return names;
}

int PlaybackManager::slotsPerPage() const
{
    return m_slotsPerPage;
}

void PlaybackManager::setSlotsPerPage(int count)
{
    // fewer slots hide the last ones: they come back with more
    count = qBound(kMinSlots, count, maxSlots());
    if (count == m_slotsPerPage)
        return;
    m_slotsPerPage = count;
    m_doc->setModified();
    emit pagesChanged();
    bump();
}

int PlaybackManager::addPage(const QString &name)
{
    Page page = defaultPage(int(m_pages.count()) + 1);
    if (name.trimmed().isEmpty() == false)
        page.name = name.trimmed();
    m_pages.append(page);
    m_doc->setModified();
    emit pagesChanged();
    return int(m_pages.count()) - 1;
}

void PlaybackManager::renamePage(int page, const QString &name)
{
    const int p = resolvePage(page);
    if (p < 0 || name.trimmed().isEmpty() || m_pages.at(p).name == name.trimmed())
        return;
    m_pages[p].name = name.trimmed();
    m_doc->setModified();
    emit pagesChanged();
}

void PlaybackManager::removePage(int page)
{
    const int p = resolvePage(page);
    if (p < 0 || m_pages.count() <= 1)
        return;

    // what its slots held (it keeps playing): the controls bound to them
    // drive it by itself now
    QHash<int, quint32> removedSlots;
    for (auto it = m_pages.at(p).slotMap.cbegin(); it != m_pages.at(p).slotMap.cend(); ++it)
    {
        removedSlots.insert(it.key(), it.value().functionId);
        if (it.value().crossfade)
            dropCrossfade(it.value().functionId);
    }
    QList<int> newIndexes;
    for (int i = 0; i < m_pages.count(); i++)
        newIndexes.append(i < p ? i : i == p ? -1 : i - 1);

    m_pages.removeAt(p);
    // the pages after it move down one place
    auto shift = [p](int &index) {
        if (index > p)
            index--;
        else if (index == p)
            index = -1;
    };
    if (m_currentPage > p || m_currentPage >= m_pages.count())
        m_currentPage--;
    shift(m_selectedPage);
    shift(m_lastGoPage);
    fixSelection();
    renumberPages(newIndexes, removedSlots);

    m_doc->setModified();
    emit pagesChanged();
    emit pageChanged();
    emit assignmentsChanged();
    bump();
}

int PlaybackManager::movedIndex(int index, int from, int to)
{
    if (index == from)
        return to;
    if (from < to && index > from && index <= to)
        return index - 1;
    if (from > to && index >= to && index < from)
        return index + 1;
    return index;
}

void PlaybackManager::movePage(int from, int to)
{
    const int count = int(m_pages.count());
    if (from < 0 || from >= count || to < 0 || to >= count || from == to)
        return;

    m_pages.move(from, to);
    // the current and the selected pages are the same pages, at other places
    m_currentPage = movedIndex(m_currentPage, from, to);
    if (m_selectedPage >= 0)
        m_selectedPage = movedIndex(m_selectedPage, from, to);
    if (m_lastGoPage >= 0)
        m_lastGoPage = movedIndex(m_lastGoPage, from, to);
    QList<int> newIndexes;
    for (int i = 0; i < count; i++)
        newIndexes.append(movedIndex(i, from, to));
    renumberPages(newIndexes, QHash<int, quint32>());

    m_doc->setModified();
    emit pagesChanged();
    emit pageChanged();
    bump();
}

void PlaybackManager::renumberPages(const QList<int> &newIndexes, const QHash<int, quint32> &removedSlots)
{
    // the flash presses held on a page: their releases come with its new
    // index; the ones of a deleted page let go now (its controls are bound
    // to something else from now on)
    QMap<std::tuple<int, int, int>, QList<quint32>> presses;
    QList<quint32> letGo;
    for (auto it = m_flashPresses.cbegin(); it != m_flashPresses.cend(); ++it)
    {
        const int page = std::get<0>(it.key());
        const int now = page >= 0 && page < newIndexes.count() ? newIndexes.at(page) : page;
        if (page >= 0 && now < 0)
            letGo.append(it.value());
        else
            presses.insert(std::make_tuple(now, std::get<1>(it.key()), std::get<2>(it.key())), it.value());
    }
    m_flashPresses = presses;
    for (quint32 fid : std::as_const(letGo))
        flashFunction(int(fid), false);

    emit pagesRenumbered(newIndexes, removedSlots);
}

void PlaybackManager::nextPage()
{
    setCurrentPage(m_currentPage + 1);
}

void PlaybackManager::previousPage()
{
    setCurrentPage(m_currentPage - 1);
}

int PlaybackManager::resolvePage(int page) const
{
    if (page < 0)
        return m_currentPage;
    return page < m_pages.count() ? page : -1;
}

/*********************************************************************
 * Slots
 *********************************************************************/

const PlaybackManager::Slot *PlaybackManager::slotAt(int page, int slot) const
{
    const int p = resolvePage(page);
    if (p < 0)
        return nullptr;
    const QMap<int, Slot> &slotMap = m_pages.at(p).slotMap;
    auto it = slotMap.constFind(slot);
    return it == slotMap.cend() ? nullptr : &it.value();
}

Function *PlaybackManager::function(quint32 functionId) const
{
    return m_doc->function(functionId);
}

bool PlaybackManager::isAction(const QString &action)
{
    static const QStringList actions = {
        "none", "go", "back", "pauseBack", "pause", "release", "flash", "flashGo", "toggle", "select"
    };
    return actions.contains(action);
}

bool PlaybackManager::faderReleases(const Slot &slot, const Function *f)
{
    // by default a cuelist keeps its cue when its fader goes down, a
    // submaster and a scene stop, like the scene faders of the SHOW
    if (slot.faderReleases < 0)
    {
        const Cuelist *cl = qobject_cast<const Cuelist *>(f);
        return cl == nullptr || cl->mode() == Cuelist::ModeSubmaster;
    }
    return slot.faderReleases > 0;
}

bool PlaybackManager::crossfades(const Slot &slot, const Function *f)
{
    // (a list made another type keeps the option: its fader is the intensity)
    const Cuelist *cl = qobject_cast<const Cuelist *>(f);
    return slot.crossfade && cl != nullptr && cl->mode() == Cuelist::ModeNormal;
}

bool PlaybackManager::ruledByFader(const Function *f) const
{
    const Cuelist *cl = qobject_cast<const Cuelist *>(f);
    if (cl == nullptr || cl->mode() != Cuelist::ModeSubmaster)
        return false;
    for (const Page &page : m_pages)
    {
        for (const Slot &entry : page.slotMap)
        {
            if (entry.functionId == cl->id() && entry.faderStarts)
                return true;
        }
    }
    return false;
}

bool PlaybackManager::isPlaying(const Function *f)
{
    return f != nullptr && f->stopped() == false;
}

bool PlaybackManager::isFlashing(quint32 functionId) const
{
    return m_flashHolds.value(functionId, 0) > 0;
}

bool PlaybackManager::isFlashLit(quint32 functionId) const
{
    return isFlashing(functionId) || m_flashRamps.contains(functionId);
}

bool PlaybackManager::hasAssigned() const
{
    for (const Page &page : m_pages)
        if (page.slotMap.isEmpty() == false)
            return true;
    return false;
}

void PlaybackManager::assign(int page, int slot, int functionId)
{
    if (functionId < 0)
    {
        clearSlot(page, slot);
        return;
    }

    const int p = resolvePage(page);
    if (p < 0 || slot < 0 || slot >= maxSlots() || function(quint32(functionId)) == nullptr)
        return;

    QMap<int, Slot> &slotMap = m_pages[p].slotMap;
    auto it = slotMap.constFind(slot);
    if (it != slotMap.cend() && it.value().functionId == quint32(functionId))
        return;

    // another function: the options of the slot were for the one before
    if (it != slotMap.cend() && it.value().crossfade)
        dropCrossfade(it.value().functionId);
    Slot entry;
    entry.functionId = quint32(functionId);
    slotMap.insert(slot, entry);

    m_doc->setModified();
    emit assignmentsChanged();
    bump();
}

void PlaybackManager::clearSlot(int page, int slot)
{
    const int p = resolvePage(page);
    if (p < 0 || m_pages.at(p).slotMap.contains(slot) == false)
        return;
    const Slot entry = m_pages[p].slotMap.take(slot);
    if (entry.crossfade)
        dropCrossfade(entry.functionId);

    // what it plays keeps playing
    fixSelection();
    m_doc->setModified();
    emit assignmentsChanged();
    bump();
}

void PlaybackManager::moveSlot(int fromPage, int fromSlot, int toPage, int toSlot)
{
    const int from = resolvePage(fromPage);
    const int to = resolvePage(toPage);
    if (from < 0 || to < 0 || fromSlot < 0 || fromSlot >= maxSlots() || toSlot < 0 || toSlot >= maxSlots() ||
        (from == to && fromSlot == toSlot))
        return;

    const bool hasFrom = m_pages.at(from).slotMap.contains(fromSlot);
    const bool hasTo = m_pages.at(to).slotMap.contains(toSlot);
    if (hasFrom == false && hasTo == false)
        return;

    const Slot a = m_pages[from].slotMap.take(fromSlot);
    const Slot b = m_pages[to].slotMap.take(toSlot);
    if (hasFrom)
        m_pages[to].slotMap.insert(toSlot, a);
    if (hasTo)
        m_pages[from].slotMap.insert(fromSlot, b);

    // the selection goes with its playback
    if (m_selectedPage == from && m_selectedSlot == fromSlot)
    {
        m_selectedPage = to;
        m_selectedSlot = toSlot;
    }
    else if (m_selectedPage == to && m_selectedSlot == toSlot)
    {
        m_selectedPage = from;
        m_selectedSlot = fromSlot;
    }
    fixSelection();

    m_doc->setModified();
    emit assignmentsChanged();
    bump();
}

int PlaybackManager::functionAt(int page, int slot) const
{
    const Slot *entry = slotAt(page, slot);
    return entry != nullptr ? int(entry->functionId) : -1;
}

int PlaybackManager::firstFreeSlot(int page) const
{
    const int p = resolvePage(page);
    if (p < 0)
        return -1;
    for (int i = 0; i < m_slotsPerPage; i++)
    {
        if (m_pages.at(p).slotMap.contains(i) == false)
            return i;
    }
    return -1;
}

void PlaybackManager::setSlotOption(int page, int slot, const QString &key, const QVariant &value)
{
    const int p = resolvePage(page);
    if (p < 0)
        return;
    auto it = m_pages[p].slotMap.find(slot);
    if (it == m_pages[p].slotMap.end())
        return;

    Slot &entry = it.value();
    if (key == QLatin1String("label"))
    {
        entry.label = value.toString().trimmed();
    }
    else if (key == QLatin1String("color"))
    {
        const QString color = value.toString().trimmed();
        if (color.isEmpty() == false && QColor(color).isValid() == false)
            return;
        entry.color = color;
    }
    else if (key == QLatin1String("button1") || key == QLatin1String("button2") || key == QLatin1String("button3"))
    {
        const QString action = value.toString();
        if (isAction(action) == false)
            return;
        (key.endsWith('1') ? entry.button1 : key.endsWith('2') ? entry.button2 : entry.button3) = action;
    }
    else if (key == QLatin1String("faderStarts"))
    {
        entry.faderStarts = value.toBool();
    }
    else if (key == QLatin1String("faderReleases"))
    {
        // true/false, or -1 for the default of the type
        entry.faderReleases = value.typeId() == QMetaType::Bool ? (value.toBool() ? 1 : 0)
                                                                : qBound(-1, value.toInt(), 1);
    }
    else if (key == QLatin1String("flashAttack") || key == QLatin1String("flashDecay"))
    {
        // (a flash that runs keeps the times it started with)
        int &time = key == QLatin1String("flashAttack") ? entry.flashAttack : entry.flashDecay;
        const int ms = qBound(0, value.toInt(), maxFlashTime());
        if (ms == time)
            return;
        time = ms;
    }
    else if (key == QLatin1String("faderMode"))
    {
        // the A/B crossfade is for Normal cuelists only
        const QString mode = value.toString();
        const bool crossfade = mode == QLatin1String("crossfade");
        const Cuelist *cl = qobject_cast<const Cuelist *>(function(entry.functionId));
        if ((crossfade == false && mode != QLatin1String("level")) ||
            (crossfade && (cl == nullptr || cl->mode() != Cuelist::ModeNormal)) || crossfade == entry.crossfade)
            return;
        entry.crossfade = crossfade;
        dropCrossfade(entry.functionId);
    }
    else
    {
        return;
    }

    m_doc->setModified();
    bump();
}

/*********************************************************************
 * Status
 *********************************************************************/

void PlaybackManager::cueIndexes(Cuelist *cuelist, int *current, int *next)
{
    const Cuelist::Status status = cuelist->status();
    const int count = cuelist->cueCount();
    const bool playing = isPlaying(cuelist);

    // the cue on stage by its uid: cues recorded or deleted before it move
    // it to another index (a paused list, or before its next frame)
    *current = playing ? cuelist->indexOfUid(status.currentUid) : -1;
    // a stopped or paused list has no frames to publish its next cue (Next,
    // a start cue, edits): it is read from the list itself
    const int upcoming = playing && cuelist->isPaused() == false ? status.next : cuelist->nextIndex();
    *next = upcoming < count ? upcoming : -1;
    // a list that never played starts at its first cue
    if (playing == false && *next < 0 && count > 0)
        *next = 0;
}

int PlaybackManager::functionState(quint32 functionId) const
{
    Function *f = function(functionId);
    if (f == nullptr)
        return -1;
    if (isPlaying(f) == false)
        return 0;
    if (f->isPaused())
        return 3;
    if (Cuelist *cl = qobject_cast<Cuelist *>(f))
    {
        const Cuelist::Status status = cl->status();
        if (status.fading || status.waiting)
            return 2;
    }
    return 1;
}

int PlaybackManager::state(int page, int slot) const
{
    const Slot *entry = slotAt(page, slot);
    return entry != nullptr ? functionState(entry->functionId) : -1;
}

bool PlaybackManager::isFlashing(int page, int slot) const
{
    const Slot *entry = slotAt(page, slot);
    return entry != nullptr && isFlashLit(entry->functionId);
}

bool PlaybackManager::isSelected(int page, int slot) const
{
    const int p = resolvePage(page);
    return p >= 0 && p == m_selectedPage && slot == m_selectedSlot;
}

int PlaybackManager::currentCue(int page, int slot) const
{
    const Slot *entry = slotAt(page, slot);
    Cuelist *cl = entry != nullptr ? qobject_cast<Cuelist *>(function(entry->functionId)) : nullptr;
    if (cl == nullptr)
        return -1;
    int current, next;
    cueIndexes(cl, &current, &next);
    return current;
}

int PlaybackManager::nextCue(int page, int slot) const
{
    const Slot *entry = slotAt(page, slot);
    Cuelist *cl = entry != nullptr ? qobject_cast<Cuelist *>(function(entry->functionId)) : nullptr;
    if (cl == nullptr)
        return -1;
    int current, next;
    cueIndexes(cl, &current, &next);
    return next;
}

int PlaybackManager::cueCount(int page, int slot) const
{
    const Slot *entry = slotAt(page, slot);
    Cuelist *cl = entry != nullptr ? qobject_cast<Cuelist *>(function(entry->functionId)) : nullptr;
    return cl != nullptr ? cl->cueCount() : 0;
}

void PlaybackManager::fillStatus(QVariantMap &info, Function *f) const
{
    const quint32 fid = f != nullptr ? f->id() : Function::invalidId();
    Cuelist *cl = qobject_cast<Cuelist *>(f);
    const bool playing = isPlaying(f);

    info.insert("empty", f == nullptr);
    info.insert("functionId", f != nullptr ? int(fid) : -1);
    info.insert("name", f != nullptr ? f->name() : QString());
    info.insert("type", f != nullptr ? Function::typeToString(f->type()) : QString());
    info.insert("isCuelist", cl != nullptr);
    info.insert("running", playing);
    info.insert("paused", playing && f->isPaused());
    info.insert("state", f != nullptr ? functionState(fid) : -1);
    info.insert("level", f != nullptr ? m_levels.value(fid, 1.0) : 0.0);
    // (lit until its decay is over; held: a press holds it now)
    info.insert("flashing", f != nullptr && isFlashLit(fid));
    info.insert("flashHeld", f != nullptr && isFlashing(fid));

    int current = -1, next = -1, count = 0;
    int linkJump = 0, linkCount = 0;
    qreal progress = playing ? 1.0 : 0.0;
    const Cuelist::Status status = cl != nullptr ? cl->status() : Cuelist::Status();
    if (cl != nullptr)
    {
        cueIndexes(cl, &current, &next);
        count = cl->cueCount();
        progress = current >= 0 ? status.progress : 0.0;
        // the next cue is a jump of a link: which one, of how many (0: always)
        if (playing && current >= 0)
        {
            linkJump = status.linkJump;
            linkCount = status.linkCount;
        }
    }
    const Cuelist::Cue cue = cl != nullptr && current >= 0 ? cl->cueAt(current) : Cuelist::Cue();
    const Cuelist::Cue nextCue = cl != nullptr && next >= 0 ? cl->cueAt(next) : Cuelist::Cue();
    info.insert("cue", current);
    info.insert("cueNumber", current >= 0 ? Cuelist::numberToString(cue.number) : QString());
    info.insert("cueName", current >= 0 ? cue.name : QString());
    info.insert("next", next);
    info.insert("nextNumber", next >= 0 ? Cuelist::numberToString(nextCue.number) : QString());
    info.insert("nextName", next >= 0 ? nextCue.name : QString());
    info.insert("linkJump", linkJump);
    info.insert("linkCount", linkCount);
    info.insert("cueCount", count);
    info.insert("progress", progress);
    // the type of list (its tag), and whether its fader rules it
    info.insert("mode", cl != nullptr ? Cuelist::modeToString(cl->mode()).toLower() : QString());
    info.insert("ruledByFader", ruledByFader(f));
    // a Chase: the steps a minute it plays at (its BPM, the show tempo or a
    // tapped rhythm); 0 for the other lists
    info.insert("chaseBpm", cl != nullptr ? cl->effectiveChaseBpm() : 0.0);

    // Live Time: the time of the cue that fades, as it runs
    const bool live = playing && current >= 0;
    info.insert("fading", live && status.fading);
    info.insert("fadeTotal", live ? int(status.fadeTotal) : 0);
    info.insert("fadeElapsed", live ? int(status.fadeElapsed) : 0);
    info.insert("liveScale", live ? status.liveScale : 1.0);
    // it runs with the cue's own times: Live Time can be stored in it
    info.insert("ownTimes", live && status.ownTimes);
    // the A/B fader has the fade in its hand (no Live Time for it)
    info.insert("manual", live && status.manual);
    // DMX Total Control: the clock of a Timecode list that plays (ms; -1: none)
    info.insert("clock", playing && status.clock >= 0 ? double(status.clock) : -1.0);
    // (the middle of a Live Time fader)
    info.insert("fadeBase", live && status.liveScale > 0 ? int(qRound(qreal(status.fadeTotal) / status.liveScale))
                                                       : int(cue.length()));
}

void PlaybackManager::fillOptions(QVariantMap &info, const Slot &slot, const Function *f)
{
    info.insert("label", slot.label);
    info.insert("color", slot.color);
    info.insert("button1", slot.button1);
    info.insert("button2", slot.button2);
    info.insert("button3", slot.button3);
    info.insert("faderStarts", slot.faderStarts);
    info.insert("faderReleases", faderReleases(slot, f));
    // what the properties show: -1 the default for the type, 0 no, 1 yes
    info.insert("faderReleasesMode", slot.faderReleases);
    info.insert("flashAttack", slot.flashAttack);
    info.insert("flashDecay", slot.flashDecay);
    info.insert("faderMode", crossfades(slot, f) ? QStringLiteral("crossfade") : QStringLiteral("level"));
}

QVariantMap PlaybackManager::slotInfo(int page, int slot) const
{
    QVariantMap info;
    const int p = resolvePage(page);
    info.insert("page", p);
    info.insert("slot", slot);

    // (an unknown page is empty, not the current one)
    const Slot *entry = p >= 0 ? slotAt(p, slot) : nullptr;
    Function *f = entry != nullptr ? function(entry->functionId) : nullptr;
    fillStatus(info, f);
    fillOptions(info, entry != nullptr ? *entry : Slot(), f);
    if (entry != nullptr && entry->label.isEmpty() == false)
        info.insert("name", entry->label);
    info.insert("selected", p >= 0 && isSelected(p, slot));
    // where an A/B fader is (the strip shows it instead of the level)
    info.insert("xfadePos", entry != nullptr && crossfades(*entry, f) ? m_crossfades.value(f->id()).pos : 0.0);
    return info;
}

QVariantMap PlaybackManager::functionStatus(int functionId) const
{
    QVariantMap info;
    Function *f = functionId >= 0 ? function(quint32(functionId)) : nullptr;
    fillStatus(info, f);
    fillOptions(info, Slot(), f);
    const int selectedId = m_selectedPage >= 0 ? functionAt(m_selectedPage, m_selectedSlot) : -1;
    info.insert("selected", f != nullptr && selectedId == functionId);
    return info;
}

QVariantMap PlaybackManager::selected() const
{
    QVariantMap map;
    const int fid = m_selectedPage >= 0 ? functionAt(m_selectedPage, m_selectedSlot) : -1;
    if (fid < 0)
        return map;
    map.insert("page", m_selectedPage);
    map.insert("slot", m_selectedSlot);
    map.insert("functionId", fid);
    return map;
}

void PlaybackManager::fixSelection()
{
    if (m_selectedPage < 0 || functionAt(m_selectedPage, m_selectedSlot) < 0)
    {
        m_selectedPage = -1;
        m_selectedSlot = -1;
    }
    if (m_lastGoPage < 0 || functionAt(m_lastGoPage, m_lastGoSlot) < 0)
    {
        m_lastGoPage = -1;
        m_lastGoSlot = -1;
    }
}

/*********************************************************************
 * Starting and the level
 *********************************************************************/

void PlaybackManager::ensureConnected(Function *f)
{
    // engine signals go by name (MinGW)
    connect(f, SIGNAL(running(quint32)), this, SLOT(slotFunctionRunning(quint32)), Qt::UniqueConnection);
    connect(f, SIGNAL(stopped(quint32)), this, SLOT(slotFunctionStopped(quint32)), Qt::UniqueConnection);
}

void PlaybackManager::startFunction(Function *f, int cueIndex)
{
    ensureConnected(f);
    if (Cuelist *cl = qobject_cast<Cuelist *>(f))
    {
        if (cueIndex >= 0)
            cl->setStartCue(cueIndex);
    }
    // a function paused while it was stopping would only resume (Function::start)
    if (f->isPaused())
        f->setPause(false);

    // the level first, so its first frame isn't at full
    applyLevel(f);
    m_held.insert(f->id());
    f->start(m_doc->masterTimer(), playbackParent(), 0, Function::defaultSpeed(), Function::defaultSpeed());
    ensurePolling();
}

qreal PlaybackManager::effectiveLevel(quint32 functionId) const
{
    auto ramp = m_flashRamps.constFind(functionId);
    if (ramp != m_flashRamps.cend())
        return rampValue(ramp.value());
    return isFlashing(functionId) ? 1.0 : m_levels.value(functionId, 1.0);
}

void PlaybackManager::applyLevel(Function *f)
{
    const qreal level = effectiveLevel(f->id());

    // a super scene has no intensity of its own: a level per track
    if (Show *show = qobject_cast<Show *>(f))
    {
        // the overrides we have, on the track each one is on now (a track
        // removed while it plays moves the ones after it down a place)
        const int count = int(show->attributes().count());
        QList<int> ids(count, Function::invalidAttributeId());
        for (int id : m_showOverrides.value(show->id()))
        {
            const int index = show->adjustAttribute(level, id);
            if (index >= 0 && index < count && ids.at(index) == Function::invalidAttributeId())
                ids[index] = id;
        }
        bool requested = false;
        for (int i = 0; i < count; i++)
        {
            // an override of a previous run is gone (a function drops them
            // when it stops): ask for a new one
            if (ids.at(i) != Function::invalidAttributeId())
                continue;
            ids[i] = show->requestAttributeOverride(i, level);
            requested = true;
        }
        m_showOverrides.insert(show->id(), ids);
        // its stop must forget these IDs, whoever stops it: the next run
        // gives the same numbers to other overrides (a group's master)
        if (requested)
            ensureConnected(show);

        // the static values play with it, as a scene of its own
        if (Function *values = function(show->staticScene()))
            values->requestAttributeOverride(Function::Intensity, level);
        return;
    }

    // Intensity allows a single override, shared with the groups and the
    // SHOW faders (the last one wins): no id to keep
    f->requestAttributeOverride(Function::Intensity, level);
}

void PlaybackManager::levelUpForGo(quint32 functionId)
{
    if (m_levels.value(functionId, 1.0) < kFaderDown)
        m_levels.insert(functionId, 1.0);
}

void PlaybackManager::setHeldByFader(const std::function<bool(quint32)> &held)
{
    m_heldByFader = held;
}

void PlaybackManager::releaseIt(Function *f)
{
    // (one a fader of the SHOW holds up plays on: only that fader stops it)
    if (m_heldByFader == nullptr || m_heldByFader(f->id()) == false)
        f->stop(playbackParent());
    dropFlashHolds(f->id());
    m_flashRamps.remove(f->id());
    m_startedByFlash.remove(f->id());
    m_held.remove(f->id());
}

void PlaybackManager::dropFlashHolds(quint32 functionId)
{
    m_flashHolds.remove(functionId);
    m_flashDecays.remove(functionId);
    // (their releases find nothing to let go)
    for (auto it = m_flashPresses.begin(); it != m_flashPresses.end();)
    {
        it.value().removeAll(functionId);
        if (it.value().isEmpty())
            it = m_flashPresses.erase(it);
        else
            ++it;
    }
}

void PlaybackManager::pauseFunction(Function *f)
{
    if (isPlaying(f))
        f->setPause(f->isPaused() == false);
}

void PlaybackManager::releaseOthersOnGo(quint32 functionId)
{
    for (Function *f : m_doc->functionsByType(Function::CuelistType))
    {
        Cuelist *cl = qobject_cast<Cuelist *>(f);
        if (cl != nullptr && cl->id() != functionId && isPlaying(cl) && cl->releaseOnNextGo())
            releaseIt(cl);
    }
}

void PlaybackManager::forgetRuntime(quint32 functionId)
{
    m_showOverrides.remove(functionId);
    dropFlashHolds(functionId);
    m_flashRamps.remove(functionId);
    m_startedByFlash.remove(functionId);
    m_held.remove(functionId);
}

void PlaybackManager::slotFunctionRunning(quint32 id)
{
    // a restart drops the overrides of the run before: the level again
    Function *f = function(id);
    if (f != nullptr && m_held.contains(id) && isPlaying(f))
        applyLevel(f);
    bump();
}

void PlaybackManager::slotFunctionStopped(quint32 id)
{
    // the signal of a previous run, arriving after it was started again: the
    // playbacks hold the new run when they started it again (a release and a
    // GO in the same frame). Started again by someone else only (a group's
    // quick second press), what they held of the run before is over: the
    // flash that started it would release the group's run
    Function *f = function(id);
    if (f != nullptr && f->stopped() == false && f->isStartedBy(playbackParent()))
        return;

    forgetRuntime(id);
    bump();
}

/*********************************************************************
 * Transport
 *********************************************************************/

void PlaybackManager::go(int page, int slot)
{
    const Slot *entry = slotAt(page, slot);
    if (entry == nullptr)
        return;
    m_lastGoPage = resolvePage(page);
    m_lastGoSlot = slot;
    goFunction(int(entry->functionId));
}

void PlaybackManager::chooseStartCue(Cuelist *cuelist, int step)
{
    const int count = cuelist->cueCount();
    if (count == 0)
        return;
    int current, next;
    cueIndexes(cuelist, &current, &next);
    int index = qMax(0, next + step);
    if (index >= count)
        index = cuelist->atEnd() == Cuelist::AtEndStay ? count - 1 : 0;
    cuelist->setStartCue(index);
}

void PlaybackManager::goFunction(int functionId)
{
    Function *f = functionId >= 0 ? function(quint32(functionId)) : nullptr;
    if (f == nullptr)
        return;
    const quint32 fid = f->id();
    // a GO keeps a playback a flash started
    keepFlashStart(fid);

    if (Cuelist *cl = qobject_cast<Cuelist *>(f))
    {
        // a stopped submaster: its fader starts it, GO only chooses the cue
        if (isPlaying(cl) == false && ruledByFader(cl))
        {
            chooseStartCue(cl, 1);
            bump();
            return;
        }
        releaseOthersOnGo(fid);
        // (paused: GO only resumes it)
        if (isPlaying(cl))
            cl->go();
        else
        {
            levelUpForGo(fid);
            startFunction(cl);
        }
    }
    else if (isPlaying(f) == false)
    {
        levelUpForGo(fid);
        startFunction(f);
    }
    else if (f->isPaused())
    {
        f->setPause(false);
    }
    else if (Chaser *chaser = qobject_cast<Chaser *>(f))
    {
        // steps: the next one, at the level it plays at
        ChaserAction action;
        action.m_action = ChaserNextStep;
        action.m_masterIntensity = chaser->getAttributeValue(Function::Intensity);
        action.m_stepIntensity = 1.0;
        action.m_fadeMode = Chaser::FromFunction;
        action.m_stepIndex = -1;
        chaser->setAction(action);
    }
    // a super scene or a scene that plays: nothing to do
    bump();
}

void PlaybackManager::back(int page, int slot)
{
    const Slot *entry = slotAt(page, slot);
    if (entry != nullptr)
        backFunction(int(entry->functionId));
}

void PlaybackManager::backFunction(int functionId)
{
    Function *f = functionId >= 0 ? function(quint32(functionId)) : nullptr;
    // a stopped submaster: the cue before, for its fader
    if (f != nullptr && isPlaying(f) == false && ruledByFader(f))
    {
        chooseStartCue(qobject_cast<Cuelist *>(f), -1);
        bump();
        return;
    }
    if (isPlaying(f) == false)
        return;

    if (Cuelist *cl = qobject_cast<Cuelist *>(f))
    {
        cl->back();
    }
    else if (Chaser *chaser = qobject_cast<Chaser *>(f))
    {
        if (chaser->isPaused())
            chaser->setPause(false);
        ChaserAction action;
        action.m_action = ChaserPreviousStep;
        action.m_masterIntensity = chaser->getAttributeValue(Function::Intensity);
        action.m_stepIntensity = 1.0;
        action.m_fadeMode = Chaser::FromFunction;
        action.m_stepIndex = -1;
        chaser->setAction(action);
    }
    bump();
}

void PlaybackManager::pauseBack(int page, int slot)
{
    const Slot *entry = slotAt(page, slot);
    if (entry != nullptr)
        pauseBackFunction(int(entry->functionId));
}

void PlaybackManager::pauseBackFunction(int functionId)
{
    Function *f = functionId >= 0 ? function(quint32(functionId)) : nullptr;
    // a stopped submaster has no fade to pause: Back
    if (f != nullptr && isPlaying(f) == false && ruledByFader(f))
    {
        backFunction(functionId);
        return;
    }
    if (isPlaying(f) == false)
        return;

    if (Cuelist *cl = qobject_cast<Cuelist *>(f))
    {
        // a fading cue pauses; paused, or with the cue done, it goes back
        if (cl->isPaused())
        {
            cl->setPause(false);
            cl->back();
        }
        else if (cl->status().fading)
        {
            cl->setPause(true);
        }
        else
        {
            cl->back();
        }
    }
    else
    {
        pauseFunction(f);
    }
    bump();
}

void PlaybackManager::pause(int page, int slot)
{
    const Slot *entry = slotAt(page, slot);
    Function *f = entry != nullptr ? function(entry->functionId) : nullptr;
    if (isPlaying(f) == false)
        return;
    pauseFunction(f);
    bump();
}

void PlaybackManager::release(int page, int slot)
{
    const Slot *entry = slotAt(page, slot);
    if (entry != nullptr)
        releaseFunction(int(entry->functionId));
}

void PlaybackManager::releaseFunction(int functionId)
{
    Function *f = functionId >= 0 ? function(quint32(functionId)) : nullptr;
    if (f == nullptr)
        return;
    // (a group that also plays it loses it too: one function, one state)
    if (isPlaying(f))
        releaseIt(f);
    else
        forgetRuntime(f->id());
    bump();
}

void PlaybackManager::toggle(int page, int slot)
{
    const Slot *entry = slotAt(page, slot);
    if (entry == nullptr)
        return;
    Function *f = function(entry->functionId);
    if (isPlaying(f))
        release(page, slot);
    // (a stopped submaster: its fader starts it)
    else if (ruledByFader(f) == false)
        go(page, slot);
}

void PlaybackManager::select(int page, int slot)
{
    const Slot *entry = slotAt(page, slot);
    if (entry == nullptr)
        return;

    m_selectedPage = resolvePage(page);
    m_selectedSlot = slot;
    // the cuelist editors and the command line work on it too
    if (m_cuelists != nullptr && qobject_cast<Cuelist *>(function(entry->functionId)) != nullptr)
        m_cuelists->setSelectedId(int(entry->functionId));
    bump();
}

void PlaybackManager::goTo(int page, int slot, int cueIndex)
{
    const Slot *entry = slotAt(page, slot);
    if (entry == nullptr)
        return;
    m_lastGoPage = resolvePage(page);
    m_lastGoSlot = slot;
    goToFunction(int(entry->functionId), cueIndex);
}

void PlaybackManager::goToFunction(int functionId, int cueIndex)
{
    Function *f = functionId >= 0 ? function(quint32(functionId)) : nullptr;
    if (f == nullptr || cueIndex < 0)
        return;
    const quint32 fid = f->id();

    if (Cuelist *cl = qobject_cast<Cuelist *>(f))
    {
        if (cueIndex >= cl->cueCount())
            return;
        // a stopped submaster: the cue its fader starts on, as a GO
        if (isPlaying(cl) == false && ruledByFader(cl))
        {
            cl->setStartCue(cueIndex);
            bump();
            return;
        }
        keepFlashStart(fid);
        releaseOthersOnGo(fid);
        // (goToCue resumes a paused list)
        if (isPlaying(cl))
            cl->goToCue(cueIndex);
        else
        {
            levelUpForGo(fid);
            startFunction(cl, cueIndex);
        }
    }
    else if (Chaser *chaser = qobject_cast<Chaser *>(f))
    {
        if (cueIndex >= chaser->stepsCount())
            return;
        keepFlashStart(fid);
        // a stopped chaser keeps it and starts from that step
        ChaserAction action;
        action.m_action = ChaserSetStepIndex;
        action.m_masterIntensity = chaser->getAttributeValue(Function::Intensity);
        action.m_stepIntensity = 1.0;
        action.m_fadeMode = Chaser::FromFunction;
        action.m_stepIndex = cueIndex;
        if (isPlaying(chaser) && chaser->isPaused())
            chaser->setPause(false);
        chaser->setAction(action);
        if (isPlaying(chaser) == false)
        {
            levelUpForGo(fid);
            startFunction(chaser);
        }
    }
    else
    {
        return;
    }
    bump();
}

void PlaybackManager::setNextCue(int page, int slot, int cueIndex)
{
    const Slot *entry = slotAt(page, slot);
    Cuelist *cl = entry != nullptr ? qobject_cast<Cuelist *>(function(entry->functionId)) : nullptr;
    if (cl == nullptr)
        return;
    // a run starts without a pre-selected cue: a stopped list starts there
    if (isPlaying(cl))
        cl->setNextCue(cueIndex);
    else
        cl->setStartCue(cueIndex);
    bump();
}

void PlaybackManager::flash(int page, int slot, bool down)
{
    flashPress(page, slot, 0, down);
}

void PlaybackManager::flashPress(int page, int slot, int button, bool down)
{
    // the page as given: -1 is the current page of the press, whatever page
    // is current when the button comes up
    const std::tuple<int, int, int> key(page, slot, button);
    if (down)
    {
        const Slot *entry = slotAt(page, slot);
        Function *f = entry != nullptr ? function(entry->functionId) : nullptr;
        if (f == nullptr)
            return;
        // (with the flash times of this slot)
        const int attack = entry->flashAttack;
        const int decay = entry->flashDecay;
        m_flashPresses[key].append(f->id());
        flashDown(f, attack, decay);
        bump();
        return;
    }

    // the release lets go of what the press held, even when the page changed
    // or the slot holds something else now (nothing: a release, a stop all
    // or a stop let it go already)
    auto it = m_flashPresses.find(key);
    if (it == m_flashPresses.end())
        return;
    const quint32 fid = it.value().takeLast();
    if (it.value().isEmpty())
        m_flashPresses.erase(it);
    flashFunction(int(fid), false);
}

void PlaybackManager::flashFunction(int functionId, bool down)
{
    Function *f = functionId >= 0 ? function(quint32(functionId)) : nullptr;
    if (f == nullptr)
        return;

    if (down)
    {
        // (a SHOW button, a mapping on a function: the times of its first slot)
        int attack = 0, decay = 0;
        flashTimesOf(f->id(), &attack, &decay);
        flashDown(f, attack, decay);
    }
    else
    {
        if (isFlashing(f->id()) == false)
            return;
        flashUp(f);
    }
    bump();
}

void PlaybackManager::flashTimesOf(quint32 functionId, int *attack, int *decay) const
{
    *attack = 0;
    *decay = 0;
    for (const Page &page : m_pages)
    {
        for (const Slot &entry : page.slotMap)
        {
            if (entry.functionId == functionId)
            {
                *attack = entry.flashAttack;
                *decay = entry.flashDecay;
                return;
            }
        }
    }
}

void PlaybackManager::flashDown(Function *f, int attack, int decay)
{
    const quint32 fid = f->id();
    const bool playing = isPlaying(f);
    // up from where it is (a press during a decay: no dip); one the flash
    // starts comes up from dark
    const qreal from = playing ? effectiveLevel(fid) : 0.0;

    // several buttons can hold the same function (clones, mappings)
    m_flashHolds.insert(fid, m_flashHolds.value(fid, 0) + 1);
    m_flashDecays.insert(fid, decay);
    if (attack > 0 && from < 1.0)
        startRamp(fid, from, 1.0, attack, false);
    else
        m_flashRamps.remove(fid);

    if (playing == false)
    {
        // (the ramp is there first: its first frame is dark)
        m_startedByFlash.insert(fid);
        startFunction(f);
    }
    else
    {
        applyLevel(f);
    }
}

void PlaybackManager::flashUp(Function *f)
{
    const quint32 fid = f->id();
    const int holds = m_flashHolds.value(fid, 0);
    if (holds <= 0)
        return;
    if (holds > 1)
    {
        m_flashHolds.insert(fid, holds - 1);
        return;
    }

    const int decay = m_flashDecays.value(fid, 0);
    const qreal from = effectiveLevel(fid);
    dropFlashHolds(fid);

    // back to its level in the decay time; what the flash started goes to
    // dark, and is released at the end (it stays started by the flash until
    // then: a GO meanwhile keeps it). A run that someone else stopped and
    // started again in this frame isn't the one the flash started
    const bool started = m_startedByFlash.contains(fid);
    const qreal to = started ? 0.0 : m_levels.value(fid, 1.0);
    if (decay > 0 && isPlaying(f) && (started == false || f->isStartedBy(playbackParent())) &&
        qAbs(from - to) > 0.001)
    {
        startRamp(fid, from, to, decay, started);
        applyLevel(f);
        return;
    }

    // at once: what the flash started goes; the rest gets its level back
    // (the run of someone else stays)
    m_flashRamps.remove(fid);
    if (m_startedByFlash.remove(fid))
    {
        if (isPlaying(f) && f->isStartedBy(playbackParent()))
            releaseIt(f);
    }
    else if (isPlaying(f))
    {
        applyLevel(f);
    }
}

void PlaybackManager::keepFlashStart(quint32 functionId)
{
    m_startedByFlash.remove(functionId);
    auto it = m_flashRamps.find(functionId);
    if (it != m_flashRamps.end() && it->releaseAtEnd)
    {
        it->releaseAtEnd = false;
        retargetRamp(it.value(), m_levels.value(functionId, 1.0));
    }
}

qreal PlaybackManager::rampValue(const FlashRamp &ramp) const
{
    const qint64 elapsed = m_rampClock.elapsed() - ramp.start;
    if (ramp.duration <= 0 || elapsed >= ramp.duration)
        return ramp.to;
    const qreal t = qMax(qreal(0.0), qreal(elapsed) / qreal(ramp.duration));
    return ramp.from + (ramp.to - ramp.from) * t;
}

void PlaybackManager::startRamp(quint32 functionId, qreal from, qreal to, int duration, bool releaseAtEnd)
{
    FlashRamp ramp;
    ramp.from = from;
    ramp.to = to;
    ramp.start = m_rampClock.elapsed();
    ramp.duration = duration;
    ramp.releaseAtEnd = releaseAtEnd;
    m_flashRamps.insert(functionId, ramp);
    if (m_rampTimer->isActive() == false)
        m_rampTimer->start();
}

void PlaybackManager::retargetRamp(FlashRamp &ramp, qreal to)
{
    // from where it is now, in the time it had left: no jump
    const qint64 now = m_rampClock.elapsed();
    ramp.from = rampValue(ramp);
    ramp.duration = int(qMax(qint64(0), ramp.start + ramp.duration - now));
    ramp.start = now;
    ramp.to = to;
}

void PlaybackManager::slotFlashRamps()
{
    const qint64 now = m_rampClock.elapsed();
    bool ended = false;
    // (a release at the end changes the ramps: by their IDs)
    const QList<quint32> ids = m_flashRamps.keys();
    for (quint32 id : ids)
    {
        auto it = m_flashRamps.find(id);
        if (it == m_flashRamps.end())
            continue;
        Function *f = function(id);
        if (f == nullptr)
        {
            m_flashRamps.erase(it);
            continue;
        }
        if (now - it->start < it->duration)
        {
            if (isPlaying(f))
                applyLevel(f);
            continue;
        }

        // the end: at full (an attack), at its level (a decay), or released
        // (the decay of what the flash started, when the run is still ours)
        const bool release = it->releaseAtEnd;
        m_flashRamps.erase(it);
        ended = true;
        if (release && m_startedByFlash.remove(id) && isPlaying(f) && f->isStartedBy(playbackParent()))
            releaseIt(f);
        else if (isPlaying(f))
            applyLevel(f);
    }
    if (m_flashRamps.isEmpty())
        m_rampTimer->stop();
    // (the Flash button goes dark)
    if (ended)
        bump();
}

void PlaybackManager::pressButton(int page, int slot, int button, bool down)
{
    if (button < 1 || button > 3)
        return;
    // only a flash acts on the release: it lets go of what its press held
    // (the page, the slot or the action of the button may have changed)
    if (down == false)
    {
        flashPress(page, slot, button, false);
        return;
    }

    const Slot *entry = slotAt(page, slot);
    Function *f = entry != nullptr ? function(entry->functionId) : nullptr;
    if (f == nullptr)
        return;
    const quint32 fid = f->id();
    const bool isCuelist = f->type() == Function::CuelistType;
    const QString action = button == 1 ? entry->button1 : button == 2 ? entry->button2 : entry->button3;
    const bool playing = isPlaying(f);

    if (action == QLatin1String("flash"))
    {
        flashPress(page, slot, button, true);
    }
    else if (action == QLatin1String("flashGo"))
    {
        // at full while pressed, and a GO
        flashPress(page, slot, button, true);
        if (playing)
        {
            go(page, slot);
        }
        else if (ruledByFader(f) == false && m_startedByFlash.remove(fid))
        {
            // a stopped one started with the flash: that start is its GO (not
            // a second cue), and it plays on when the button comes up. (Not a
            // submaster: only its fader starts it, the flash only flashes it)
            m_lastGoPage = resolvePage(page);
            m_lastGoSlot = slot;
            levelUpForGo(fid);
            if (isCuelist)
                releaseOthersOnGo(fid);
            bump();
        }
    }
    else if (action == QLatin1String("go"))
        go(page, slot);
    else if (action == QLatin1String("back"))
        back(page, slot);
    else if (action == QLatin1String("pauseBack"))
        pauseBack(page, slot);
    else if (action == QLatin1String("pause"))
        pause(page, slot);
    else if (action == QLatin1String("release"))
        release(page, slot);
    else if (action == QLatin1String("toggle"))
        toggle(page, slot);
    else if (action == QLatin1String("select"))
        select(page, slot);
}

void PlaybackManager::setLevel(int page, int slot, qreal level)
{
    const Slot *entry = slotAt(page, slot);
    Function *f = entry != nullptr ? function(entry->functionId) : nullptr;
    if (f == nullptr)
        return;
    const Slot options = *entry;
    const quint32 fid = f->id();

    level = qBound(0.0, level, 1.0);
    // an A/B fader is no intensity: it plays at the level it had
    if (crossfades(options, f))
    {
        crossfadeTo(qobject_cast<Cuelist *>(f), level);
        bump();
        return;
    }
    const qreal old = m_levels.value(fid, 1.0);
    m_levels.insert(fid, level);
    // a decay ends at the new level (during an attack, only the level
    // changes: it comes back after the flash). What the flash started goes
    // to dark anyway
    auto ramp = m_flashRamps.find(fid);
    if (ramp != m_flashRamps.end() && isFlashing(fid) == false && ramp->releaseAtEnd == false)
        retargetRamp(ramp.value(), level);
    // (a flash holds it at full: the level comes back after it)
    if (isPlaying(f) && isFlashing(fid) == false)
        applyLevel(f);

    // the fader up from 0 starts it, down to 0 releases it (by default only
    // scenes and submasters). A submaster starts at the level of its fader,
    // never at full, on its next cue (a GO would only choose that cue).
    // During the decay of what a flash started it plays already: it is kept
    // at the level of the fader, as with a GO (not a second cue)
    const bool faderUp = options.faderStarts && old < kFaderDown && level >= kFaderDown;
    if (faderUp && isPlaying(f) && isFlashing(fid) == false && m_startedByFlash.contains(fid))
    {
        keepFlashStart(fid);
        if (f->type() == Function::CuelistType)
            releaseOthersOnGo(fid);
    }
    else if (faderUp && isPlaying(f) == false)
    {
        if (ruledByFader(f))
        {
            releaseOthersOnGo(fid);
            startFunction(f);
        }
        else
        {
            goFunction(int(fid));
        }
    }
    else if (faderReleases(options, f) && isPlaying(f) && old > 0.0 && level <= 0.0)
        releaseIt(f);
    bump();
}

qreal PlaybackManager::level(int page, int slot) const
{
    const Slot *entry = slotAt(page, slot);
    if (entry == nullptr)
        return 0.0;
    // (an A/B fader: where it is)
    if (crossfades(*entry, function(entry->functionId)))
        return m_crossfades.value(entry->functionId).pos;
    return m_levels.value(entry->functionId, 1.0);
}

bool PlaybackManager::faderKnown(int page, int slot) const
{
    const Slot *entry = slotAt(page, slot);
    return entry == nullptr || crossfades(*entry, function(entry->functionId)) == false ||
           m_crossfades.value(entry->functionId).end >= 0;
}

bool PlaybackManager::crossfadesAt(int page, int slot) const
{
    const Slot *entry = slotAt(page, slot);
    return entry != nullptr && crossfades(*entry, function(entry->functionId));
}

void PlaybackManager::crossfadeTo(Cuelist *cuelist, qreal pos)
{
    const quint32 fid = cuelist->id();
    Crossfade &xfade = m_crossfades[fid];
    // the first value only tells where the fader is
    if (xfade.end < 0)
    {
        xfade.end = pos >= 0.5 ? 1 : 0;
        xfade.pos = pos;
        return;
    }
    xfade.pos = pos;
    const qreal travel = qAbs(pos - xfade.end);

    // it leaves its end: a GO, the next cue fading with the fader (a
    // stopped list starts on its cue)
    if (xfade.active == false)
    {
        if (travel <= kCrossfadeEdge)
            return;
        keepFlashStart(fid);
        releaseOthersOnGo(fid);
        if (isPlaying(cuelist))
        {
            cuelist->manualGo();
        }
        else
        {
            levelUpForGo(fid);
            cuelist->setStartManual(true);
            startFunction(cuelist);
        }
        xfade.active = true;
    }

    if (travel >= 1.0 - kCrossfadeEdge)
    {
        // at the other end the cue is complete: the next travel, the
        // other way, plays the following one
        cuelist->setManualPosition(1.0);
        cuelist->endManual(Cuelist::ManualComplete);
        xfade.end = 1 - xfade.end;
        xfade.active = false;
    }
    else if (travel <= kCrossfadeEdge)
    {
        // back where it started: cancelled, the cue before is back
        cuelist->setManualPosition(0.0);
        cuelist->endManual(Cuelist::ManualCancel);
        xfade.active = false;
    }
    else
    {
        cuelist->setManualPosition(travel);
    }
}

void PlaybackManager::dropCrossfade(quint32 functionId)
{
    auto it = m_crossfades.find(functionId);
    if (it == m_crossfades.end())
        return;
    Cuelist *cl = qobject_cast<Cuelist *>(function(functionId));
    if (it->active && cl != nullptr && isPlaying(cl))
        cl->endManual(Cuelist::ManualOnClock);
    m_crossfades.erase(it);
}

void PlaybackManager::mainGo()
{
    // the selected playback, else the one that got the last GO
    if (m_selectedPage >= 0 && functionAt(m_selectedPage, m_selectedSlot) >= 0)
        go(m_selectedPage, m_selectedSlot);
    else if (m_lastGoPage >= 0 && functionAt(m_lastGoPage, m_lastGoSlot) >= 0)
        go(m_lastGoPage, m_lastGoSlot);
    else
        emit message(tr("Select a playback first"), true);
}

void PlaybackManager::mainBack()
{
    if (m_selectedPage >= 0 && functionAt(m_selectedPage, m_selectedSlot) >= 0)
        back(m_selectedPage, m_selectedSlot);
    else if (m_lastGoPage >= 0 && functionAt(m_lastGoPage, m_lastGoSlot) >= 0)
        back(m_lastGoPage, m_lastGoSlot);
    else
        emit message(tr("Select a playback first"), true);
}

void PlaybackManager::releaseAll(int except)
{
    // (all of them but $except)
    const auto other = [except](quint32 id) { return except < 0 || id != quint32(except); };
    QSet<quint32> ids;
    for (const Page &page : std::as_const(m_pages))
        for (const Slot &entry : page.slotMap)
            ids.insert(entry.functionId);
    for (Function *f : m_doc->functionsByType(Function::CuelistType))
        ids.insert(f->id());
    // and what the flashes started, also off the slots (a slot cleared while
    // held, a mapping on a function): their holds are gone below, so their
    // releases would let go of nothing and it would play on
    for (quint32 id : std::as_const(m_startedByFlash))
        ids.insert(id);

    for (quint32 id : std::as_const(ids))
    {
        Function *f = function(id);
        if (other(id) && isPlaying(f))
            releaseIt(f);
    }
    // what they held at full and plays on (off the slots: a GO from a
    // cleared slot, a group) gets its level back, also in an attack or a decay
    QList<quint32> held = m_flashHolds.keys();
    held.append(m_flashRamps.keys());
    held.removeIf([&](quint32 id) { return other(id) == false; });
    m_flashHolds.removeIf([&](QHash<quint32, int>::iterator it) { return other(it.key()); });
    m_flashDecays.removeIf([&](QHash<quint32, int>::iterator it) { return other(it.key()); });
    m_flashRamps.removeIf([&](QHash<quint32, FlashRamp>::iterator it) { return other(it.key()); });
    if (m_flashRamps.isEmpty())
        m_rampTimer->stop();
    for (auto it = m_flashPresses.begin(); it != m_flashPresses.end();)
    {
        it.value().removeIf(other);
        if (it.value().isEmpty())
            it = m_flashPresses.erase(it);
        else
            ++it;
    }
    m_startedByFlash.removeIf(other);
    for (quint32 id : std::as_const(held))
    {
        Function *f = function(id);
        if (isPlaying(f))
            applyLevel(f);
    }
    bump();
}

/*********************************************************************
 * Cue speed and Live Time
 *********************************************************************/

qreal PlaybackManager::cueSpeed() const
{
    return m_doc->masterTimer()->cueSpeed();
}

void PlaybackManager::setCueSpeed(qreal speed)
{
    // (the timer tells cueSpeedChanged)
    m_doc->masterTimer()->setCueSpeed(speed);
}

void PlaybackManager::setLiveTime(int page, int slot, int ms)
{
    const Slot *entry = slotAt(page, slot);
    if (entry != nullptr)
        setLiveTimeFunction(int(entry->functionId), ms);
}

void PlaybackManager::scaleLiveTime(int page, int slot, qreal factor)
{
    const Slot *entry = slotAt(page, slot);
    if (entry != nullptr)
        scaleLiveTimeFunction(int(entry->functionId), factor);
}

bool PlaybackManager::storeLiveTime(int page, int slot)
{
    const Slot *entry = slotAt(page, slot);
    return entry != nullptr && storeLiveTimeFunction(int(entry->functionId));
}

void PlaybackManager::setLiveTimeFunction(int functionId, int ms)
{
    // (the cuelist does it on its next frame, when a cue fades)
    Cuelist *cl = functionId >= 0 ? qobject_cast<Cuelist *>(function(quint32(functionId))) : nullptr;
    if (cl != nullptr && ms >= 0 && isPlaying(cl))
        cl->setLiveTime(uint(ms));
}

void PlaybackManager::scaleLiveTimeFunction(int functionId, qreal factor)
{
    Cuelist *cl = functionId >= 0 ? qobject_cast<Cuelist *>(function(quint32(functionId))) : nullptr;
    if (cl != nullptr && factor > 0 && isPlaying(cl))
        cl->scaleLiveTime(factor);
}

bool PlaybackManager::storeLiveTimeFunction(int functionId)
{
    Cuelist *cl = functionId >= 0 ? qobject_cast<Cuelist *>(function(quint32(functionId))) : nullptr;
    if (cl == nullptr || m_cuelists == nullptr || isPlaying(cl) == false)
        return false;
    const Cuelist::Status status = cl->status();
    const int index = cl->indexOfUid(status.currentUid);
    if (index < 0 || qAbs(status.liveScale - 1.0) < 1e-6)
        return false;
    // (a BACK time, a fade-in or a Chase change: not the times of the cue,
    // a scale of them would be a time nobody saw)
    if (status.ownTimes == false)
        return false;
    // the cue takes the times it runs with; the run goes on with them as its own
    m_cuelists->scaleCueTimes(cl->id(), index, status.liveScale);
    cl->liveTimeStored();
    bump();
    return true;
}

/*********************************************************************
 * Record
 *********************************************************************/

void PlaybackManager::afterRecord()
{
    if (m_programmer != nullptr && m_settings != nullptr && m_settings->clearAfterRecord())
        m_programmer->clear();
}

int PlaybackManager::recordToSlot(int page, int slot)
{
    const int p = resolvePage(page);
    if (p < 0 || slot < 0 || slot >= maxSlots() || m_cuelists == nullptr)
        return -1;

    const Slot *entry = slotAt(p, slot);
    if (entry == nullptr)
    {
        // Record + an empty playback: a new cuelist, the programmer is
        // its cue 1 and its fader goes up
        const int id = m_cuelists->create(QString(), -1, m_settings != nullptr ? m_settings->defaultPriority() : -1);
        if (id < 0)
            return -1;   // the editor tells why
        assign(p, slot, id);
        m_levels.insert(quint32(id), 1.0);
        select(p, slot);
        emit message(tr("New cuelist on playback %1 with cue 1").arg(slot + 1), false);
        afterRecord();
        bump();
        return 0;
    }

    Cuelist *cl = qobject_cast<Cuelist *>(function(entry->functionId));
    if (cl == nullptr)
    {
        emit message(tr("This playback holds a scene: record on an empty playback or on a cuelist"), true);
        return -1;
    }

    // the next whole number after the last cue. What the editor says while it
    // records (a cue with no changes is recorded, with a warning) goes with
    // the result: the screen shows one message at a time
    QString said;
    const QMetaObject::Connection listening =
        connect(m_cuelists, &CuelistEditor::message, this, [&said](const QString &text) { said = text; });
    QVariantMap opts;
    opts.insert("existing", "ask");
    const int index = m_cuelists->recordCue(cl->id(), QString(), opts);
    disconnect(listening);
    if (index < 0)
        return -1;
    const QString recorded = tr("Cue %1 recorded in %2").arg(Cuelist::numberToString(cl->cueAt(index).number), cl->name());
    if (said.isEmpty())
        emit message(recorded, false);
    else
        emit message(recorded + QStringLiteral(" · ") + said, true);
    afterRecord();
    bump();
    return index;
}

bool PlaybackManager::updateSelected(bool cueOnly)
{
    const Slot *entry = m_selectedPage >= 0 ? slotAt(m_selectedPage, m_selectedSlot) : nullptr;
    Cuelist *cl = entry != nullptr ? qobject_cast<Cuelist *>(function(entry->functionId)) : nullptr;
    if (cl == nullptr || m_cuelists == nullptr)
    {
        emit message(tr("Select a cuelist playback first"), true);
        return false;
    }

    // the cue loaded in the programmer, else the one it plays (by its uid:
    // a cue recorded or deleted before it moves it)
    int index = -1;
    const QVariantMap editing = m_cuelists->editing();
    if (editing.isEmpty() == false && editing.value("id").toUInt() == cl->id())
        index = editing.value("index").toInt();
    else if (isPlaying(cl))
        index = cl->indexOfUid(cl->status().currentUid);
    if (index < 0 || index >= cl->cueCount())
    {
        emit message(tr("Load a cue of %1 or play it first").arg(cl->name()), true);
        return false;
    }

    QVariantMap opts;
    opts.insert("cueOnly", cueOnly);
    if (m_cuelists->updateCue(cl->id(), index, opts) == false)
        return false;   // the editor tells why
    emit message(tr("Cue %1 updated in %2").arg(Cuelist::numberToString(cl->cueAt(index).number), cl->name()),
                 false);
    afterRecord();
    bump();
    return true;
}

/*********************************************************************
 * Polling
 *********************************************************************/

void PlaybackManager::ensurePolling()
{
    if (m_pollTimer->isActive() == false)
        m_pollTimer->start();
}

void PlaybackManager::poll(bool force)
{
    // every function a playback shows, and the cuelists (groups play them too)
    QSet<quint32> ids;
    for (const Page &page : std::as_const(m_pages))
        for (const Slot &entry : page.slotMap)
            ids.insert(entry.functionId);
    for (Function *f : m_doc->functionsByType(Function::CuelistType))
        ids.insert(f->id());

    QHash<quint32, QList<int>> signatures;
    QHash<quint32, QPair<int, qint64>> progress;
    bool active = false;
    for (quint32 id : std::as_const(ids))
    {
        Function *f = function(id);
        if (f == nullptr)
            continue;
        const bool playing = isPlaying(f);
        if (playing || f->isRunning())
            active = true;

        QList<int> signature = { playing, playing && f->isPaused(), -1, -1, 0, 0, isFlashLit(id), 0, 0, 1000, 0 };
        int step = 0;
        qint64 clock = -1;
        if (Cuelist *cl = qobject_cast<Cuelist *>(f))
        {
            const Cuelist::Status status = cl->status();
            signature[2] = status.current;
            signature[3] = status.next;
            signature[4] = status.fading;
            signature[5] = status.waiting;
            signature[7] = status.linkJump;
            // a Live Time change
            signature[8] = int(status.fadeTotal);
            signature[9] = qRound(status.liveScale * 1000.0);
            // the speed of a Chase (the show tempo, a tapped rhythm)
            signature[10] = qRound(cl->effectiveChaseBpm() * 10.0);
            // in steps of 5 %
            if (playing && status.current >= 0)
                step = qRound(status.progress * 20.0);
            // DMX Total Control: the clock of a Timecode list, as it shows (tenths)
            if (playing && status.clock >= 0)
                clock = status.clock / 100;
        }
        signatures.insert(id, signature);
        progress.insert(id, qMakePair(step, clock));
    }

    const bool changed = signatures != m_signatures;
    const bool moved = progress != m_progress;
    m_signatures = signatures;
    m_progress = progress;
    if (changed || force)
        bump();
    if (moved)
    {
        m_progressRevision++;
        emit progressChanged();
    }

    if (active)
        ensurePolling();
    else
        m_pollTimer->stop();
}

void PlaybackManager::slotPoll()
{
    poll(false);
}

void PlaybackManager::slotFunctionListChanged()
{
    // something started or stopped
    poll(false);
}

void PlaybackManager::slotCueStarted(quint32 functionId, quint32 cueUid)
{
    Q_UNUSED(functionId)
    Q_UNUSED(cueUid)
    poll(true);
}

/*********************************************************************
 * Functions removed and replaced
 *********************************************************************/

void PlaybackManager::slotFunctionRemoved(quint32 id)
{
    bool found = false;
    for (Page &page : m_pages)
    {
        for (auto it = page.slotMap.begin(); it != page.slotMap.end();)
        {
            if (it.value().functionId == id)
            {
                it = page.slotMap.erase(it);
                found = true;
            }
            else
            {
                ++it;
            }
        }
    }

    m_levels.remove(id);
    m_crossfades.remove(id);
    forgetRuntime(id);
    m_signatures.remove(id);
    m_progress.remove(id);
    if (found)
    {
        fixSelection();
        emit assignmentsChanged();
    }
    // (a show cleared: told once, when it is, not for each of its functions)
    if (m_doc->isClearing() == false)
        bump();
}

void PlaybackManager::slotDocCleared()
{
    bump();
}

void PlaybackManager::slotFunctionReplaced(quint32 id)
{
    // the same ID, a new function (steps converted to a cuelist): its slots
    // and level stay, the runtime of the old one goes
    forgetRuntime(id);
    m_crossfades.remove(id);
    bump();
}

void PlaybackManager::slotFunctionChanged(quint32 id)
{
    // cues recorded, deleted, renamed or renumbered: what the playbacks show
    // (the cue on stage, the next one, how many) is read again, also when
    // the list is stopped and nothing is polled
    if (qobject_cast<Cuelist *>(function(id)) != nullptr)
        bump();
}

/*********************************************************************
 * Stop all, new show
 *********************************************************************/

void PlaybackManager::resetLive()
{
    // the flashes are let go: what they hold at full, or bring up or down,
    // gets its level back
    QList<quint32> flashing = m_flashHolds.keys();
    flashing.append(m_flashRamps.keys());
    m_flashHolds.clear();
    m_flashDecays.clear();
    m_flashRamps.clear();
    m_rampTimer->stop();
    m_flashPresses.clear();
    m_startedByFlash.clear();
    for (quint32 id : std::as_const(flashing))
    {
        Function *f = function(id);
        if (isPlaying(f))
            applyLevel(f);
    }
    bump();
}

void PlaybackManager::clear()
{
    m_pages.clear();
    m_pages.append(defaultPage(1));
    m_currentPage = 0;
    m_slotsPerPage = kDefaultSlots;
    m_selectedPage = -1;
    m_selectedSlot = -1;
    m_lastGoPage = -1;
    m_lastGoSlot = -1;

    m_levels.clear();
    m_flashHolds.clear();
    m_flashDecays.clear();
    m_flashRamps.clear();
    m_rampTimer->stop();
    m_flashPresses.clear();
    m_startedByFlash.clear();
    m_held.clear();
    m_showOverrides.clear();
    m_crossfades.clear();
    m_signatures.clear();
    m_progress.clear();
    // (not saved: every show opens at x1)
    setCueSpeed(1.0);

    emit pagesChanged();
    emit pageChanged();
    emit assignmentsChanged();
    bump();
}

/*********************************************************************
 * Load & save
 *********************************************************************/

bool PlaybackManager::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLDTCPlaybacks)
    {
        qWarning() << Q_FUNC_INFO << "Playbacks node not found";
        return false;
    }

    clear();
    m_pages.clear();

    // (a value that isn't a number, in a damaged file, keeps the default)
    const QXmlStreamAttributes attrs = root.attributes();
    bool slotsOk = false;
    const int slotCount = attrs.value(KXMLDTCPlaybackSlotsPerPage).toInt(&slotsOk);
    if (slotsOk)
        m_slotsPerPage = qBound(kMinSlots, slotCount, maxSlots());
    const int current = attrs.value(KXMLDTCPlaybackCurrentPage).toInt();

    while (root.readNextStartElement())
    {
        if (root.name() != KXMLDTCPlaybackPage)
        {
            qWarning() << Q_FUNC_INFO << "Unknown Playbacks tag:" << root.name().toString();
            root.skipCurrentElement();
            continue;
        }

        Page page = defaultPage(int(m_pages.count()) + 1);
        const QString name = root.attributes().value(KXMLDTCPlaybackPageName).toString().trimmed();
        if (name.isEmpty() == false)
            page.name = name;

        while (root.readNextStartElement())
        {
            const QXmlStreamAttributes slotAttrs = root.attributes();
            bool indexOk = false, functionOk = false;
            const int index = slotAttrs.value(KXMLDTCPlaybackIndex).toInt(&indexOk);
            const quint32 fid = slotAttrs.value(KXMLDTCPlaybackFunction).toUInt(&functionOk);

            // the slots of functions that are not in the show any more go
            if (root.name() == KXMLDTCPlaybackSlot && indexOk && functionOk && index >= 0 &&
                index < maxSlots() && function(fid) != nullptr)
            {
                Slot entry;
                entry.functionId = fid;
                entry.label = slotAttrs.value(KXMLDTCPlaybackLabel).toString();
                const QString color = slotAttrs.value(KXMLDTCPlaybackColor).toString();
                if (QColor(color).isValid())
                    entry.color = color;
                const QString button1 = slotAttrs.value(KXMLDTCPlaybackButton1).toString();
                const QString button2 = slotAttrs.value(KXMLDTCPlaybackButton2).toString();
                const QString button3 = slotAttrs.value(KXMLDTCPlaybackButton3).toString();
                if (isAction(button1))
                    entry.button1 = button1;
                if (isAction(button2))
                    entry.button2 = button2;
                if (isAction(button3))
                    entry.button3 = button3;
                bool startsOk = false, releasesOk = false, levelOk = false;
                const int starts = slotAttrs.value(KXMLDTCPlaybackFaderStarts).toInt(&startsOk);
                if (startsOk)
                    entry.faderStarts = starts != 0;
                const int releases = slotAttrs.value(KXMLDTCPlaybackFaderRelease).toInt(&releasesOk);
                if (releasesOk)
                    entry.faderReleases = qBound(-1, releases, 1);
                // (not there, or not a number: at once)
                entry.flashAttack = qBound(0, slotAttrs.value(KXMLDTCPlaybackFlashAttack).toInt(), maxFlashTime());
                entry.flashDecay = qBound(0, slotAttrs.value(KXMLDTCPlaybackFlashDecay).toInt(), maxFlashTime());
                // (an A/B fader on a cuelist only)
                entry.crossfade = slotAttrs.value(KXMLDTCPlaybackFaderMode) == QLatin1String("crossfade") &&
                                  qobject_cast<Cuelist *>(function(fid)) != nullptr;
                page.slotMap.insert(index, entry);

                // clones carry the same level: the last one read wins (a
                // level that isn't a number is full, not a fader down)
                const qreal level = slotAttrs.value(KXMLDTCPlaybackLevel).toDouble(&levelOk);
                m_levels.insert(fid, levelOk && qIsFinite(level) ? qBound(0.0, level, 1.0) : 1.0);
            }
            root.skipCurrentElement();
        }
        m_pages.append(page);
    }

    if (m_pages.isEmpty())
        m_pages.append(defaultPage(1));
    m_currentPage = qBound(0, current, int(m_pages.count()) - 1);

    emit pagesChanged();
    emit pageChanged();
    emit assignmentsChanged();
    bump();
    return true;
}

bool PlaybackManager::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != nullptr);

    const Slot defaults;
    doc->writeStartElement(KXMLDTCPlaybacks);
    doc->writeAttribute(KXMLDTCPlaybackSlotsPerPage, QString::number(m_slotsPerPage));
    doc->writeAttribute(KXMLDTCPlaybackCurrentPage, QString::number(m_currentPage));

    for (const Page &page : m_pages)
    {
        doc->writeStartElement(KXMLDTCPlaybackPage);
        doc->writeAttribute(KXMLDTCPlaybackPageName, page.name);
        for (auto it = page.slotMap.cbegin(); it != page.slotMap.cend(); ++it)
        {
            const Slot &entry = it.value();
            if (function(entry.functionId) == nullptr)
                continue;

            // defaults are left out
            doc->writeStartElement(KXMLDTCPlaybackSlot);
            doc->writeAttribute(KXMLDTCPlaybackIndex, QString::number(it.key()));
            doc->writeAttribute(KXMLDTCPlaybackFunction, QString::number(entry.functionId));
            const qreal level = m_levels.value(entry.functionId, 1.0);
            if (level < 1.0)
                doc->writeAttribute(KXMLDTCPlaybackLevel, QString::number(level));
            if (entry.label.isEmpty() == false)
                doc->writeAttribute(KXMLDTCPlaybackLabel, entry.label);
            if (entry.color.isEmpty() == false)
                doc->writeAttribute(KXMLDTCPlaybackColor, entry.color);
            if (entry.button1 != defaults.button1)
                doc->writeAttribute(KXMLDTCPlaybackButton1, entry.button1);
            if (entry.button2 != defaults.button2)
                doc->writeAttribute(KXMLDTCPlaybackButton2, entry.button2);
            if (entry.button3 != defaults.button3)
                doc->writeAttribute(KXMLDTCPlaybackButton3, entry.button3);
            if (entry.faderStarts != defaults.faderStarts)
                doc->writeAttribute(KXMLDTCPlaybackFaderStarts, entry.faderStarts ? "1" : "0");
            if (entry.faderReleases >= 0)
                doc->writeAttribute(KXMLDTCPlaybackFaderRelease, QString::number(entry.faderReleases));
            if (entry.flashAttack > 0)
                doc->writeAttribute(KXMLDTCPlaybackFlashAttack, QString::number(entry.flashAttack));
            if (entry.flashDecay > 0)
                doc->writeAttribute(KXMLDTCPlaybackFlashDecay, QString::number(entry.flashDecay));
            if (entry.crossfade)
                doc->writeAttribute(KXMLDTCPlaybackFaderMode, QStringLiteral("crossfade"));
            doc->writeEndElement();
        }
        doc->writeEndElement();
    }

    doc->writeEndElement();
    return true;
}

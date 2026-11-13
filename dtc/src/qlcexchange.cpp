/*
  DMX Total Control
  qlcexchange.cpp

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

#include <QRegularExpression>
#include <QCoreApplication>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QKeySequence>
#include <QLocale>
#include <QRect>
#include <QHash>
#include <QSet>
#include <algorithm>
#include <climits>
#include <cmath>
#include <memory>

#include "qlcexchange.h"
#include "playbackmanager.h"
#include "controlmapper.h"
#include "fxtools.h"

#include "qlcpalette.h"
#include "qlcchannel.h"
#include "mastertimer.h"
#include "fadechannel.h"
#include "collection.h"
#include "chaserstep.h"
#include "sequence.h"
#include "qlcfile.h"
#include "cuelist.h"
#include "fxscene.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

namespace
{

typedef Cuelist::Key Key;
typedef Cuelist::FxKey FxKey;

/** What separates the number from the name, and the label from the note */
const QString NameSeparator = QStringLiteral(" · ");
const QString NoteSeparator = QStringLiteral(" — ");

/** Line breaks become spaces: a step note is one line */
QString oneLine(const QString &text)
{
    QString line = text;
    line.replace(QLatin1String("\r\n"), QLatin1String(" "));
    line.replace(QLatin1Char('\n'), QLatin1Char(' '));
    line.replace(QLatin1Char('\r'), QLatin1Char(' '));
    return line;
}

/** A channel of a fixture of the show */
bool inDoc(const Doc *doc, const Key &key)
{
    const Fixture *fixture = doc->fixture(key.first);
    return fixture != nullptr && key.second < fixture->channels();
}

/** What a channel sends when nothing controls it */
uchar restValue(const Doc *doc, const Key &key)
{
    const Fixture *fixture = doc->fixture(key.first);
    const QLCChannel *channel = fixture != nullptr ? fixture->channel(key.second) : nullptr;
    return channel != nullptr ? channel->defaultValue() : 0;
}

/** The cue that comes after cue $index, or nullptr when the list ends
 *  there. Past the last cue: the first one in a loop, and with Release the
 *  trigger of the first one releases the list (only a GO does it with Stay) */
const Cuelist::Cue *nextCue(const QList<Cuelist::Cue> &cues, int index, Cuelist::AtEnd atEnd)
{
    if (index + 1 < cues.count())
        return &cues.at(index + 1);
    const Cuelist::Cue &first = cues.first();
    if (atEnd == Cuelist::AtEndLoop || (atEnd == Cuelist::AtEndRelease && first.trigger != Cuelist::TriggerGo))
        return &first;
    return nullptr;
}

/** How long the step of $cue lasts, its fade in included: until $next
 *  starts by itself, Function::infiniteSpeed() when it waits for a GO */
uint stepLength(const Cuelist::Cue &cue, const Cuelist::Cue *next)
{
    if (next == nullptr || next->trigger == Cuelist::TriggerGo)
        return Function::infiniteSpeed();

    // a Wait counts from the GO of the cue, a Follow from its end
    quint64 length = next->triggerTime;
    if (next->trigger == Cuelist::TriggerFollow)
        length += cue.length();
    // (QLC+ reads a time past INT_MAX as forever)
    return uint(qMin(length, quint64(INT_MAX)));
}

/** How long the step of cue $index of a Timecode list lasts, its fade in
 *  included: until the time of the next cue. A cue without a time, and
 *  the last one, wait for a GO */
uint timecodeLength(const QList<Cuelist::Cue> &cues, int index)
{
    const Cuelist::Cue &cue = cues.at(index);
    if (index + 1 >= cues.count() || cue.time < 0 || cues.at(index + 1).time < 0)
        return Function::infiniteSpeed();
    // (a time before the one of the cue before: the next step at once)
    return uint(qMax(0, cues.at(index + 1).time - cue.time));
}

/** The ms of a step of a Chase, as Cuelist::chasePeriodLocked: a beat of
 *  its BPM, or of the show tempo now, by its multiplier; and those steps a
 *  minute in $perMinute. Not its tapped rhythm (saved with the show, but
 *  QLC+ gets the programmed rate; the lost line says so) */
uint chasePeriod(const Doc *doc, const Cuelist *cl, qreal *perMinute)
{
    const MasterTimer *timer = doc->masterTimer();
    const bool showTempo = cl->chaseShowTempo() && timer != nullptr && timer->beatTimeDuration() > 0;
    const qreal beat = showTempo ? qreal(timer->beatTimeDuration()) : 60000.0 / qreal(cl->chaseBpm());
    const qreal multiplier = cl->chaseMultiplier();
    if (perMinute != nullptr)
        *perMinute = (showTempo ? qreal(timer->bpmNumber()) : qreal(cl->chaseBpm())) * multiplier;
    return uint(qMax(20.0, std::round(beat / multiplier)));
}

/** $ms as seconds in the language of the program, as the cuelist editor
 *  writes them: "5 s", "12,5 s" */
QString secondsText(uint ms)
{
    const int decimals = ms % 1000 == 0 ? 0 : ms % 100 == 0 ? 1 : ms % 10 == 0 ? 2 : 3;
    return QLocale().toString(ms / 1000.0, 'f', decimals) + QStringLiteral(" s");
}

/** What the times of the attributes and the fixtures of a cue have that
 *  its step can't play: a delay, a fade other than the cue's, a curve */
struct OwnParts
{
    bool delay = false;
    bool fade = false;
    bool curve = false;
};

OwnParts ownParts(const Cuelist::Cue &cue)
{
    OwnParts own;
    auto add = [&](const Cuelist::PartTime &part) {
        if (part.delay > 0)
            own.delay = true;
        if (part.fade >= 0 && uint(part.fade) != cue.fade)
            own.fade = true;
        // (-1: the cue's curve)
        if (part.curve > int(FadeChannel::CurveLinear))
            own.curve = true;
    };
    for (const Cuelist::PartTime &part : cue.groupTimes)
        add(part);
    for (const Cuelist::PartTime &part : cue.fixtureTimes)
        add(part);
    return own;
}

/*********************************************************************
 * QLC+ steps to cues
 *********************************************************************/

/** As many cues as steps can become */
const int MaxCues = 1000;
/** The deepest rack (in racks) whose look a step takes */
const int MaxRackDepth = 8;

/** The time of a step in ms: steps that follow the BPM hold beats */
uint stepMs(uint time, bool beats, int beatMs)
{
    if (time == Function::defaultSpeed())
        return 0;
    if (time == Function::infiniteSpeed() || beats == false)
        return time;
    return Function::beatsToTime(time, beatMs);
}

/** A fade can't last forever */
uint finite(uint ms)
{
    return ms == Function::infiniteSpeed() ? 0u : qMin(ms, uint(INT_MAX));
}

/** What the function of a Chaser step shows */
struct Look
{
    QHash<Key, uchar> values;
    QList<FXScene::Layer> fx;
    /** it is a scene, an FX or a rack */
    bool converted = false;
    /** the names of what of it is none of them */
    QStringList left;
    /** what of the step doesn't play in its cue, for its note */
    QString warning;
};

/** The name of $f to the user (its type without one), or its ID when it is missing */
QString nameOf(const Function *f, quint32 id)
{
    if (f == nullptr)
        return QString::number(id);
    return f->name().isEmpty() ? Function::typeToString(f->type()) : f->name();
}

void addValue(Look *look, const SceneValue &scv)
{
    look->values.insert(Key(scv.fxi, scv.channel), scv.value);
}

/** The values of a scene, those of its QLC+ palettes first (as it plays them) */
void addSceneLook(Doc *doc, const Scene *scene, Look *look)
{
    for (quint32 id : scene->palettes())
    {
        const QLCPalette *palette = doc->palette(id);
        if (palette == nullptr)
            continue;
        for (const SceneValue &scv : palette->valuesFromFixtureGroups(doc, scene->fixtureGroups()))
            addValue(look, scv);
        for (const SceneValue &scv : palette->valuesFromFixtures(doc, scene->fixtures()))
            addValue(look, scv);
    }
    for (const SceneValue &scv : scene->values())
        addValue(look, scv);
    look->converted = true;
}

/** The base values and the layers of an FX scene; a layer without fixtures has all of the scene */
void addFxLook(const FXScene *fx, Look *look)
{
    const QHash<Key, uchar> values = fx->values();
    for (auto it = values.cbegin(); it != values.cend(); ++it)
        look->values.insert(it.key(), it.value());
    for (FXScene::Layer layer : fx->layers())
    {
        if (layer.fixtures.isEmpty())
            layer.fixtures = fx->fixtures();
        if (layer.fixtures.isEmpty() == false)
            look->fx.append(layer);
    }
    look->converted = true;
}

/** What $f adds to $look: a scene, an FX (Move FX and Curve FX as FX), or
 *  the members of a rack in order (the later one wins). $racks: the racks
 *  already in, so that a rack in itself counts once */
void addLook(Doc *doc, const Function *f, Look *look, QSet<quint32> *racks, int depth)
{
    if (const Scene *scene = qobject_cast<const Scene *>(f))
    {
        addSceneLook(doc, scene, look);
        return;
    }
    if (const FXScene *fx = qobject_cast<const FXScene *>(f))
    {
        addFxLook(fx, look);
        return;
    }
    // (a converted copy, not in the show)
    const std::unique_ptr<FXScene> converted(f != nullptr ? FxTools::convert(doc, f) : nullptr);
    if (converted != nullptr)
    {
        addFxLook(converted.get(), look);
        return;
    }
    if (const Collection *rack = qobject_cast<const Collection *>(f))
    {
        look->converted = true;
        if (depth >= MaxRackDepth || racks->contains(rack->id()))
            return;
        racks->insert(rack->id());
        for (quint32 id : rack->functions())
        {
            const Function *member = doc->function(id);
            if (member != nullptr)
                addLook(doc, member, look, racks, depth + 1);
        }
        return;
    }
    if (f != nullptr)
        look->left.append(nameOf(f, f->id()));
}

/** The layers of a look as they play: a later layer takes the fixtures it
 *  shares with an earlier one (the whole fixture: a layer can't be split),
 *  as Cuelist::trackedState() has them */
QList<FXScene::Layer> playedLayers(const QList<FXScene::Layer> &layers)
{
    QList<FXScene::Layer> played;
    // the slots of the layers after the one looked at
    QSet<FxKey> later;
    for (qsizetype l = layers.count() - 1; l >= 0; l--)
    {
        FXScene::Layer layer = layers.at(l);
        const QStringList axes = Cuelist::fxAxes(layer.param);
        layer.fixtures.removeIf([&](quint32 fixture) {
            for (const QString &axis : axes)
                if (later.contains(FxKey(axis, fixture)))
                    return true;
            return false;
        });
        later.unite(Cuelist::fxKeysOf(layers.at(l)));
        if (layer.fixtures.isEmpty())
            continue;
        layer.own = layer.tidyOwn();
        played.prepend(layer);
    }
    return played;
}

/** Cue $cue goes from $stage (and the FX $tracked) to $look and its FX
 *  $fx: it holds what differs (everything for the first cue) and releases
 *  what $look doesn't have */
void setChanges(Cuelist::Cue *cue, bool first, const QHash<Key, uchar> &stage, const QList<FXScene::Layer> &tracked,
                const QHash<Key, uchar> &look, const QList<FXScene::Layer> &fx)
{
    for (auto it = look.cbegin(); it != look.cend(); ++it)
    {
        auto old = stage.constFind(it.key());
        if (first || old == stage.cend() || old.value() != it.value())
            cue->values.insert(it.key(), it.value());
    }
    for (auto it = stage.cbegin(); it != stage.cend(); ++it)
    {
        if (look.contains(it.key()) == false)
            cue->releases.insert(it.key());
    }

    // the FX it doesn't track as they are, and the slots it frees
    QSet<FxKey> held;
    for (const FXScene::Layer &layer : fx)
    {
        held.unite(Cuelist::fxKeysOf(layer));
        if (tracked.contains(layer) == false)
            cue->fx.append(layer);
    }
    for (const FXScene::Layer &layer : tracked)
    {
        for (const FxKey &key : Cuelist::fxKeysOf(layer))
            if (held.contains(key) == false)
                cue->fxReleases.insert(key);
    }
}

/** The numbers and names of cues read back from their $labels ("Q2.5 ·
 *  Verse", from a copy for QLC+): only when every label is one, with
 *  numbers that go up. $rests: what follows " — " in each */
bool parseLabels(const QStringList &labels, QList<int> *numbers, QStringList *names, QStringList *rests)
{
    int last = 0;
    for (const QString &label : labels)
    {
        int number = 0;
        QString name, rest;
        if (QlcExchange::parseCueLabel(label, &number, &name, &rest) == false || number <= last)
            return false;
        last = number;
        numbers->append(number);
        names->append(name);
        rests->append(rest);
    }
    return true;
}

/** What step $number of a Chaser shows: its scene, its FX as they play */
Look stepLook(Doc *doc, const ChaserStep &step, int number)
{
    Look look;
    QSet<quint32> racks;
    const Function *f = doc->function(step.fid);
    addLook(doc, f, &look, &racks, 0);
    look.fx = playedLayers(look.fx);

    const QString name = nameOf(f, step.fid);
    if (look.converted == false)
    {
        look.warning = QCoreApplication::translate("QlcExchange", "Step %1 (“%2”) isn't a scene or FX: its cue is empty")
                           .arg(QString::number(number), name);
    }
    else if (look.left.isEmpty() == false)
    {
        look.warning = QCoreApplication::translate("QlcExchange", "Step %1 (“%2”): what isn't a scene or FX stays out of its cue: %3")
                           .arg(QString::number(number), name, look.left.join(QStringLiteral(", ")));
    }
    return look;
}

/** The step indexes of one cycle of $count steps: there and back in a
 *  ping-pong (the ends once), forward in any other order */
QList<int> cycleOf(int count, Function::RunOrder order)
{
    QList<int> cycle;
    for (int i = 0; i < count; i++)
        cycle.append(i);
    if (order == Function::PingPong)
        for (int i = count - 2; i >= 1; i--)
            cycle.append(i);
    return cycle;
}

/** The times the steps of a chaser play with, in ms */
struct StepTimes
{
    const Doc *doc = nullptr;
    const Chaser *chaser = nullptr;
    /** a Sequence: its hidden scene */
    const Sequence *sequence = nullptr;
    const Function *bound = nullptr;
    /** steps in beats, and a beat in ms */
    bool beats = false;
    int beatMs = 0;

    StepTimes(const Doc *d, const Chaser *c, int beat)
        : doc(d)
        , chaser(c)
        , sequence(qobject_cast<const Sequence *>(c))
        , beats(c->tempoType() == Function::Beats)
        , beatMs(beat)
    {
        if (sequence != nullptr)
            bound = doc->function(sequence->boundSceneID());
    }

    /** Default speeds: the function of the step fades its own way. A
     *  Sequence: its hidden scene, in the tempo of the steps (as before) */
    uint own(const ChaserStep &step, bool in) const
    {
        const Function *f = sequence != nullptr ? bound : doc->function(step.fid);
        if (f == nullptr)
            return 0;
        const bool inBeats = sequence != nullptr ? beats : f->tempoType() == Function::Beats;
        return stepMs(in ? f->fadeInSpeed() : f->fadeOutSpeed(), inBeats, beatMs);
    }

    uint fadeIn(const ChaserStep &step) const
    {
        switch (chaser->fadeInMode())
        {
            case Chaser::Common: return stepMs(chaser->fadeInSpeed(), beats, beatMs);
            case Chaser::PerStep: return stepMs(step.fadeIn, beats, beatMs);
            default: return own(step, true);
        }
    }

    uint fadeOut(const ChaserStep &step) const
    {
        switch (chaser->fadeOutMode())
        {
            case Chaser::Common: return stepMs(chaser->fadeOutSpeed(), beats, beatMs);
            case Chaser::PerStep: return stepMs(step.fadeOut, beats, beatMs);
            default: return own(step, false);
        }
    }

    /** Default and Common: the duration of the chaser */
    uint duration(const ChaserStep &step) const
    {
        return stepMs(chaser->durationMode() == Chaser::PerStep ? step.duration : chaser->duration(), beats, beatMs);
    }
};

/** Cue $cue of a Sequence goes from $stage to the values of its step: a
 *  channel the step doesn't have keeps what the step before left there (they
 *  reuse the scene). It holds what changes, everything for the first cue.
 *  The values keep the palettes of the step ($stageRefs: the palettes of the
 *  stage): a value that comes from another palette, or from none now, is a
 *  change too */
void setStepChanges(Cuelist::Cue *cue, bool first, QHash<Key, uchar> *stage, QHash<Key, int> *stageRefs,
                    const ChaserStep &step)
{
    QHash<Key, uchar> target = *stage;
    QHash<Key, int> targetRefs = *stageRefs;
    for (const SceneValue &scv : step.values)
    {
        const Key key(scv.fxi, scv.channel);
        target.insert(key, scv.value);
        const int ref = step.palettes.value(key, -1);
        if (ref >= 0)
            targetRefs.insert(key, ref);
        else
            targetRefs.remove(key);
    }
    for (auto it = target.cbegin(); it != target.cend(); ++it)
    {
        auto old = stage->constFind(it.key());
        const int ref = targetRefs.value(it.key(), -1);
        if (first || old == stage->cend() || old.value() != it.value() || stageRefs->value(it.key(), -1) != ref)
        {
            cue->values.insert(it.key(), it.value());
            if (ref >= 0)
                cue->palettes.insert(it.key(), ref);
        }
    }
    *stage = target;
    *stageRefs = targetRefs;
}

/** The name of the function of a step, empty when it is missing */
QString functionName(const Doc *doc, quint32 id)
{
    const Function *f = doc->function(id);
    return f != nullptr ? f->name() : QString();
}

void setError(QString *error, const QString &text)
{
    if (error != nullptr)
        *error = text;
}

/*********************************************************************
 * The Virtual Console of QLC+ (ui/src/virtualconsole)
 *********************************************************************/

using QlcExchange::VcCueList;
using QlcExchange::VcControl;

/** The elements of the widgets, whose ID attribute is a widget ID */
const QStringList WidgetTags = { "Frame", "SoloFrame", "Button", "Slider", "XYPad", "CueList", "SpeedDial",
                                 "Label", "AudioTriggers", "Clock", "Matrix" };

/** A Cue List as QLC+ makes it, the gaps around it and the header of a
 *  frame, above its first row; three in a row */
const int ListWidth = 300;
const int ListHeight = 220;
const int Gap = 10;
const int FrameTop = 40;
const int Columns = 3;
/** The size of the Virtual Console of a new show of QLC+ */
const QSize ConsoleSize(1920, 1080);

/** The lists of the frame "Cuelists", or of the solo frame */
QList<VcCueList> listsOf(const QList<VcCueList> &lists, bool solo)
{
    QList<VcCueList> part;
    for (const VcCueList &list : lists)
        if (list.solo == solo)
            part.append(list);
    return part;
}

/** The size of a frame of $count Cue Lists */
QSize frameSize(int count)
{
    const int columns = qMin(count, Columns);
    const int rows = (count + Columns - 1) / Columns;
    return QSize(Gap + columns * (ListWidth + Gap), FrameTop + rows * (ListHeight + Gap));
}

/** Where the frames of $lists go from $at: "Cuelists", and the solo frame on its right */
QRect framesArea(const QList<VcCueList> &lists, const QPoint &at)
{
    QRect area;
    QPoint pos = at;
    for (bool solo : { false, true })
    {
        const int count = listsOf(lists, solo).count();
        if (count == 0)
            continue;
        const QRect frame(pos, frameSize(count));
        area = area.united(frame);
        pos.setX(frame.right() + 1 + Gap);
    }
    return area;
}

/** The size of a Virtual Console of $size that holds $area too */
QSize grownSize(const QSize &size, const QRect &area)
{
    return QSize(qMax(size.width(), area.right() + 1 + Gap), qMax(size.height(), area.bottom() + 1 + Gap));
}

void writeAppearance(QXmlStreamWriter &writer, const QString &frameStyle)
{
    writer.writeStartElement("Appearance");
    writer.writeTextElement("FrameStyle", frameStyle);
    writer.writeTextElement("ForegroundColor", "Default");
    writer.writeTextElement("BackgroundColor", "Default");
    writer.writeTextElement("BackgroundImage", "None");
    writer.writeTextElement("Font", "Default");
    writer.writeEndElement();
}

void writeWindowState(QXmlStreamWriter &writer, const QRect &rect)
{
    writer.writeStartElement("WindowState");
    writer.writeAttribute("Visible", KXMLQLCFalse);
    writer.writeAttribute("X", QString::number(rect.x()));
    writer.writeAttribute("Y", QString::number(rect.y()));
    writer.writeAttribute("Width", QString::number(rect.width()));
    writer.writeAttribute("Height", QString::number(rect.height()));
    writer.writeEndElement();
}

void writeControl(QXmlStreamWriter &writer, const QString &tag, const VcControl &control)
{
    writer.writeStartElement(tag);
    if (control.key.isEmpty() == false)
        writer.writeTextElement("Key", control.key);
    if (control.universe >= 0)
    {
        writer.writeStartElement("Input");
        writer.writeAttribute("Universe", QString::number(control.universe));
        writer.writeAttribute("Channel", QString::number(control.channel));
        writer.writeEndElement();
    }
    writer.writeEndElement();
}

void writeCueList(QXmlStreamWriter &writer, const VcCueList &list, const QRect &rect, quint32 *nextId)
{
    writer.writeStartElement("CueList");
    writer.writeAttribute("Caption", list.caption);
    writer.writeAttribute("ID", QString::number((*nextId)++));
    writeWindowState(writer, rect);
    writeAppearance(writer, "Sunken");
    writer.writeTextElement("Chaser", QString::number(list.functionId));
    writer.writeTextElement("NextPrevBehavior", QString::number(list.nextPrev));
    writeControl(writer, "Next", list.next);
    writeControl(writer, "Previous", list.previous);
    writeControl(writer, "Playback", list.playback);
    writeControl(writer, "Stop", list.stop);
    writer.writeEndElement();
}

/** The frame "Cuelists" (or the solo frame) with $lists, at $at */
void writeFrame(QXmlStreamWriter &writer, bool solo, const QList<VcCueList> &lists, const QPoint &at,
                quint32 *nextId)
{
    writer.writeStartElement(solo ? "SoloFrame" : "Frame");
    writer.writeAttribute("Caption", solo ? QCoreApplication::translate("QlcExchange", "Cuelists (one at a time)")
                                          : QCoreApplication::translate("QlcExchange", "Cuelists"));
    writer.writeAttribute("ID", QString::number((*nextId)++));
    writeAppearance(writer, "Sunken");
    writeWindowState(writer, QRect(at, frameSize(lists.count())));
    writer.writeTextElement("AllowChildren", KXMLQLCTrue);
    writer.writeTextElement("AllowResize", KXMLQLCTrue);
    writer.writeTextElement("ShowHeader", KXMLQLCTrue);
    writer.writeTextElement("ShowEnableButton", KXMLQLCTrue);
    if (solo)
        writer.writeTextElement("Mixing", KXMLQLCFalse);
    writer.writeTextElement("Collapsed", KXMLQLCFalse);
    writer.writeTextElement("Disabled", KXMLQLCFalse);
    for (int i = 0; i < lists.count(); i++)
    {
        const QPoint pos(Gap + (i % Columns) * (ListWidth + Gap), FrameTop + (i / Columns) * (ListHeight + Gap));
        writeCueList(writer, lists.at(i), QRect(pos, QSize(ListWidth, ListHeight)), nextId);
    }
    writer.writeEndElement();
}

/** The frames of $lists from $at (see framesArea) */
void writeFrames(QXmlStreamWriter &writer, const QList<VcCueList> &lists, const QPoint &at, quint32 *nextId)
{
    QPoint pos = at;
    for (bool solo : { false, true })
    {
        const QList<VcCueList> part = listsOf(lists, solo);
        if (part.isEmpty())
            continue;
        writeFrame(writer, solo, part, pos, nextId);
        pos.rx() += frameSize(part.count()).width() + Gap;
    }
}

/** The frame of the whole Virtual Console of QLC+ 4, with the frames of $lists */
void writeRootFrame(QXmlStreamWriter &writer, const QList<VcCueList> &lists, quint32 *nextId)
{
    writer.writeStartElement("Frame");
    writer.writeAttribute("Caption", QString());
    writeAppearance(writer, "None");
    writeFrames(writer, lists, QPoint(Gap, Gap), nextId);
    writer.writeEndElement();
}

void writeSize(QXmlStreamWriter &writer, const QSize &size)
{
    writer.writeStartElement("Size");
    writer.writeAttribute("Width", QString::number(size.width()));
    writer.writeAttribute("Height", QString::number(size.height()));
    writer.writeEndElement();
}

void writeProperties(QXmlStreamWriter &writer, const QSize &size)
{
    writer.writeStartElement("Properties");
    writeSize(writer, size);
    writer.writeStartElement("GrandMaster");
    writer.writeAttribute("Visible", "1");
    writer.writeAttribute("ChannelMode", "Intensity");
    writer.writeAttribute("ValueMode", "Reduce");
    writer.writeAttribute("SliderMode", "Normal");
    writer.writeEndElement();
    writer.writeEndElement();
}

/** What a Virtual Console holds, for the frames added to it */
struct ConsoleScan
{
    /** the highest widget ID, -1: none */
    qint64 lastId = -1;
    /** the functions a Cue List plays */
    QSet<quint32> chasers;
    /** its first frame (the whole console in QLC+ 4, the first page in
     *  QLC+ 5), and where its widgets end */
    bool hasFrame = false;
    int bottom = 0;
    bool hasProperties = false;
    bool hasSize = false;
};

ConsoleScan scanConsole(const QByteArray &section)
{
    ConsoleScan scan;
    QXmlStreamReader reader(section);
    // the elements open, from <VirtualConsole>
    QStringList path;
    int frames = 0;
    while (reader.atEnd() == false && reader.hasError() == false)
    {
        reader.readNext();
        if (reader.isEndElement() && path.isEmpty() == false)
            path.removeLast();
        if (reader.isStartElement() == false)
            continue;

        const QString name = reader.name().toString();
        path.append(name);
        const int depth = path.count();
        const QXmlStreamAttributes attrs = reader.attributes();
        bool ok = false;
        const uint id = attrs.value("ID").toUInt(&ok);
        if (ok && WidgetTags.contains(name))
            scan.lastId = qMax(scan.lastId, qint64(id));

        if (depth == 2 && name == QLatin1String("Frame"))
        {
            frames++;
            scan.hasFrame = true;
        }
        else if (depth == 2 && name == QLatin1String("Properties"))
        {
            scan.hasProperties = true;
        }
        else if (depth == 3 && name == QLatin1String("Size") && path.at(1) == QLatin1String("Properties"))
        {
            scan.hasSize = true;
        }
        else if (depth == 4 && name == QLatin1String("WindowState") && frames == 1 &&
                 path.at(1) == QLatin1String("Frame") && WidgetTags.contains(path.at(2)))
        {
            // a widget right in the first frame
            const int bottom = attrs.value("Y").toInt() + attrs.value("Height").toInt();
            scan.bottom = qMax(scan.bottom, bottom);
        }
        else if (depth >= 2 && name == QLatin1String("Chaser") && path.at(depth - 2) == QLatin1String("CueList"))
        {
            // (its text, up to its end)
            scan.chasers.insert(reader.readElementText().toUInt());
            path.removeLast();
        }
    }
    return scan;
}

/** The <Size> $reader is on, grown to hold $area */
void writeGrownSize(QXmlStreamReader &reader, QXmlStreamWriter &writer, const QRect &area)
{
    const QXmlStreamAttributes attrs = reader.attributes();
    const QSize size = grownSize(QSize(attrs.value("Width").toInt(), attrs.value("Height").toInt()), area);
    writer.writeStartElement(reader.qualifiedName().toString());
    for (const QXmlStreamAttribute &attr : attrs)
    {
        if (attr.name() != QLatin1String("Width") && attr.name() != QLatin1String("Height"))
            writer.writeAttribute(attr.qualifiedName().toString(), attr.value().toString());
    }
    writer.writeAttribute("Width", QString::number(size.width()));
    writer.writeAttribute("Height", QString::number(size.height()));
}

/*********************************************************************
 * The cue lists of a QLC+ show (vccuelist.cpp: loadXML)
 *********************************************************************/

using QlcExchange::VcCueListInfo;
using QlcExchange::Candidate;

/** The deepest frame (in frames) whose Cue Lists are read */
const int MaxFrameDepth = 32;

/** The button $reader is on (<Next>, <Previous>...): its key and its input */
VcControl readControl(QXmlStreamReader &reader)
{
    VcControl control;
    while (reader.readNextStartElement())
    {
        if (reader.name() == QLatin1String("Key"))
        {
            const QString key = reader.readElementText();
            if (control.key.isEmpty())
                control.key = key;
        }
        else if (reader.name() == QLatin1String("Input"))
        {
            // (QLC+: a universe past the last one is none)
            const QXmlStreamAttributes attrs = reader.attributes();
            bool ok = false;
            const uint universe = attrs.value("Universe").toUInt(&ok);
            if (ok && universe < uint(INT_MAX) && control.universe < 0)
            {
                control.universe = int(universe);
                control.channel = attrs.value("Channel").toUInt();
            }
            reader.skipCurrentElement();
        }
        else
        {
            reader.skipCurrentElement();
        }
    }
    return control;
}

/** The <CueList> $reader is on, the $index th of the console */
VcCueListInfo readCueList(QXmlStreamReader &reader, int index)
{
    VcCueListInfo info;
    info.index = index;
    const QXmlStreamAttributes attrs = reader.attributes();
    info.caption = attrs.value("Caption").toString();
    bool ok = false;
    const uint id = attrs.value("ID").toUInt(&ok);
    if (ok)
        info.widgetId = id;

    while (reader.readNextStartElement())
    {
        const QStringView name = reader.name();
        if (name == QLatin1String("Chaser"))
        {
            const uint chaser = reader.readElementText().toUInt(&ok);
            info.chaserId = ok ? chaser : Function::invalidId();
        }
        else if (name == QLatin1String("NextPrevBehavior"))
            info.nextPrev = reader.readElementText().toInt();
        else if (name == QLatin1String("SlidersMode"))
            info.slidersMode = reader.readElementText();
        else if (name == QLatin1String("Next"))
            info.next = readControl(reader);
        else if (name == QLatin1String("Previous"))
            info.previous = readControl(reader);
        else if (name == QLatin1String("Playback"))
            info.playback = readControl(reader);
        else if (name == QLatin1String("Stop"))
            info.stop = readControl(reader);
        else if (name == QLatin1String("Key"))
        {
            // (older files: the key of Next)
            const QString key = reader.readElementText();
            if (info.next.key.isEmpty())
                info.next.key = key;
        }
        else if (name == QLatin1String("Function"))
        {
            // the old format: a step for each function
            info.legacy = true;
            info.legacySteps++;
            reader.skipCurrentElement();
        }
        else
        {
            reader.skipCurrentElement();
        }
    }
    return info;
}

/** The Cue Lists in the element $reader is on (the console, a frame), and
 *  in its frames */
void readWidgets(QXmlStreamReader &reader, QList<VcCueListInfo> *lists, int depth)
{
    while (reader.readNextStartElement())
    {
        if (reader.name() == QLatin1String("CueList"))
            lists->append(readCueList(reader, int(lists->count())));
        else if ((reader.name() == QLatin1String("Frame") || reader.name() == QLatin1String("SoloFrame")) &&
                 depth < MaxFrameDepth)
            readWidgets(reader, lists, depth + 1);
        else
            reader.skipCurrentElement();
    }
}

/** The side fader of a Cue List plays nothing in a cuelist */
bool hasSideFader(const VcCueListInfo &info)
{
    return info.slidersMode.isEmpty() == false && info.slidersMode != QLatin1String("None");
}

/** A Cue List of the old format: listed, but it can't be converted */
Candidate legacyCandidate(const VcCueListInfo &info)
{
    Candidate candidate;
    candidate.key = QStringLiteral("vc:%1").arg(info.index);
    candidate.kind = QStringLiteral("vcCueList");
    candidate.name = info.caption;
    candidate.steps = info.legacySteps;
    candidate.enabled = false;
    candidate.warnings << QCoreApplication::translate("QlcExchange", "old QLC+ format: open and save it in QLC+ first");
    candidate.lists << info;
    return candidate;
}

/*********************************************************************
 * The keys and the controller buttons of the Cue Lists
 *********************************************************************/

/** $line in $lost, once */
void addLine(QStringList *lost, const QString &line)
{
    if (lost != nullptr && lost->contains(line) == false)
        lost->append(line);
}

/** The universe of the only device of $kind enabled, -1 when there is not just one */
int onlyUniverse(const ControlMapper *mapper, const QString &kind)
{
    QList<int> universes;
    for (const QVariant &v : mapper->devices())
    {
        const QVariantMap device = v.toMap();
        if (device.value("enabled").toBool() && device.value("kind").toString() == kind)
            universes.append(device.value("universe").toInt());
    }
    return universes.count() == 1 ? universes.first() : -1;
}

/** Mapping $m drives the function $functionId: bound to it, or to the
 *  playback slot that holds it now */
bool drives(const QVariantMap &m, quint32 functionId, const PlaybackManager *playbacks)
{
    const int fid = m.value("functionId", -1).toInt();
    if (fid >= 0)
        return quint32(fid) == functionId;
    if (playbacks == nullptr)
        return false;
    const int page = playbacks->resolvePage(m.value("pbPage", -1).toInt());
    return page >= 0 && playbacks->functionAt(page, m.value("slot").toInt()) == int(functionId);
}

/** A mapping of $mappings is on the control of $m already: the same key,
 *  or the same channel of its device (or of any device) */
bool mappedAlready(const QVariantList &mappings, const QVariantMap &m)
{
    const QString source = m.value("source").toString();
    for (const QVariant &v : mappings)
    {
        const QVariantMap other = v.toMap();
        if (other.value("source").toString() != source)
            continue;
        if (source == QLatin1String("key"))
        {
            if (other.value("key").toString().compare(m.value("key").toString(), Qt::CaseInsensitive) == 0)
                return true;
            continue;
        }
        const QString device = other.value("device").toString();
        if (other.value("channel").toUInt() == m.value("channel").toUInt() &&
            (device.isEmpty() || device == m.value("device").toString()))
            return true;
    }
    return false;
}

} // namespace

namespace QlcExchange
{

CueSteps toSteps(const Doc *doc, const Cuelist *cl, bool fillRest)
{
    CueSteps result;
    if (doc == nullptr || cl == nullptr)
        return result;

    // what the stage shows on each cue, in one pass (with the cues of that moment)
    QList<Cuelist::Cue> cues;
    const QList<Cuelist::State> states = cl->trackedStates(&cues);

    // a Chase plays its cues by itself, a step of its speed, and a Timecode
    // list each cue at its time: their triggers and links don't count, nor
    // After the last cue in a Chase
    const Cuelist::Mode mode = cl->mode();
    const bool chase = mode == Cuelist::ModeChase;
    const bool timecode = mode == Cuelist::ModeTimecode;
    const bool linked = chase == false && timecode == false;
    // the last cue linked always to the first one: the list loops (the
    // link comes before After the last cue)
    const bool loopLink = linked && cues.isEmpty() == false && cues.last().link != 0 &&
                          cues.last().link == cues.first().uid && cues.last().linkCount == 0;
    const Cuelist::AtEnd atEnd = loopLink ? Cuelist::AtEndLoop : cl->atEnd();
    result.runOrder = atEnd == Cuelist::AtEndLoop ? Function::Loop : Function::SingleShot;
    qreal chaseRate = 0;
    const uint period = chase ? chasePeriod(doc, cl, &chaseRate) : 0;
    // a Chase changes by a part of its step (-1: by the times of each cue)
    const int chaseXfade = chase ? cl->chaseXfade() : -1;
    if (chase)
    {
        // round and round: there and back, at random, or the other way
        const Cuelist::ChaseDirection direction = cl->chaseDirection();
        result.runOrder = direction == Cuelist::ChaseBounce ? Function::PingPong
                        : direction == Cuelist::ChaseRandom ? Function::Random : Function::Loop;
        if (direction == Cuelist::ChaseBackward)
            result.direction = Function::Backward;
    }
    // (QLC+: DefaultRunFirst, RunNext)
    result.nextPrevBehavior = cl->resetOnRelease() ? 0 : 1;
    result.solo = cl->releaseOnNextGo();

    if (cues.isEmpty())
    {
        result.lost << QCoreApplication::translate("QlcExchange", "the cuelist has no cues");
        return result;
    }

    // what the stage shows on each cue, and every channel the list sets or
    // releases (of the fixtures of the show)
    QList<QHash<Key, uchar>> looks;
    QSet<Key> keys;
    int fxCues = 0;
    for (int i = 0; i < cues.count(); i++)
    {
        const Cuelist::State &state = states.at(i);
        looks.append(state.values);
        if (state.fx.isEmpty() == false)
            fxCues++;
        for (auto it = state.values.cbegin(); it != state.values.cend(); ++it)
            keys.insert(it.key());
        for (const Key &key : cues.at(i).releases)
            keys.insert(key);
    }
    QList<Key> channels;
    for (const Key &key : std::as_const(keys))
        if (inDoc(doc, key))
            channels.append(key);
    // (fixture, channel): the order of SceneValue
    std::sort(channels.begin(), channels.end());
    QHash<Key, uchar> rest;
    for (const Key &key : std::as_const(channels))
    {
        result.channels.append(SceneValue(key.first, key.second, 0));
        rest.insert(key, restValue(doc, key));
    }

    bool restUsed = false;
    int delays = 0;
    int downFades = 0;
    int shortWaits = 0;
    int ownTimes = 0;
    int curves = 0;
    int longFades = 0;
    int untimed = 0;
    int links = 0;
    int actions = 0;
    bool prepared = false;
    for (int i = 0; i < cues.count(); i++)
    {
        const Cuelist::Cue &cue = cues.at(i);
        const QHash<Key, uchar> &look = looks.at(i);
        Step step;
        for (const Key &key : std::as_const(channels))
        {
            auto value = look.constFind(key);
            if (value != look.cend())
            {
                step.values.append(SceneValue(key.first, key.second, value.value()));
            }
            else if (fillRest)
            {
                step.values.append(SceneValue(key.first, key.second, rest.value(key)));
                restUsed = true;
            }
        }

        // the inverse of CuelistEditor::convertFromSteps. A Chase with a
        // change of its own: a part of its step for everything, linear and
        // with no delay (as the cuelist plays it)
        const bool cueTimes = chaseXfade < 0;
        if (cueTimes)
        {
            step.fadeIn = cue.fade;
            step.fadeOut = cue.effectiveDown();
        }
        else
        {
            step.fadeIn = step.fadeOut = uint(quint64(period) * uint(chaseXfade) / 100);
        }
        // until the next cue starts by itself: a step of the Chase, the
        // time of the next cue, else the trigger of the next cue
        const uint length = chase ? period : timecode ? timecodeLength(cues, i)
                                                       : stepLength(cue, nextCue(cues, i, atEnd));
        if (length == Function::infiniteSpeed())
        {
            step.hold = Function::infiniteSpeed();
        }
        else if (length >= step.fadeIn)
        {
            step.hold = length - step.fadeIn;
        }
        else
        {
            // a step lasts at least its fade in
            step.hold = 0;
            if (linked)
                shortWaits++;
            else
                longFades++;
        }

        step.label = cueLabel(cue.number, cue.name);
        step.note = fillRest ? cueNote(cue.number, cue.name, cue.note) : cue.note;
        result.steps.append(step);

        if (cueTimes)
        {
            // the times of the attributes and fixtures: a delay of theirs
            // is a delay of the cue
            const OwnParts own = ownParts(cue);
            if (cue.delay > 0 || own.delay)
                delays++;
            if (cue.downFade >= 0 && uint(cue.downFade) != cue.fade)
                downFades++;
            if (own.fade)
                ownTimes++;
            if (cue.curve != int(FadeChannel::CurveLinear) || own.curve)
                curves++;
        }
        if (timecode && cue.time < 0)
            untimed++;
        // (the link that loops is the run order)
        if (linked && cue.link != 0 && (loopLink == false || i + 1 < cues.count()))
            links++;
        if (cue.actions.isEmpty() == false)
            actions++;
        if (cue.noMib == false)
            prepared = true;
    }

    // what QLC+ won't get (the values of palettes go, as values). First
    // what the type of the list does
    if (chase)
    {
        // (in the language of the program: 62,5. The rates are quarters, so
        // 6 digits are exact)
        const QString bpm = QLocale().toString(chaseRate, 'g', 6);
        const bool showTempo = cl->chaseShowTempo();
        const bool tapped = cl->tapPeriod() > 0;
        if (showTempo && tapped)
            result.lost << QCoreApplication::translate("QlcExchange",
                                                       "Chase: the steps keep %1 BPM; the BPM of the show and the "
                                                       "tapped rhythm aren't followed").arg(bpm);
        else if (showTempo)
            result.lost << QCoreApplication::translate("QlcExchange",
                                                       "Chase: the steps keep %1 BPM; the BPM of the show isn't followed")
                               .arg(bpm);
        else if (tapped)
            result.lost << QCoreApplication::translate("QlcExchange",
                                                       "Chase: the steps keep %1 BPM; the tapped rhythm isn't followed")
                               .arg(bpm);
    }
    else if (timecode)
    {
        if (cl->tcSource() == Cuelist::TcMtc)
            result.lost << QCoreApplication::translate("QlcExchange",
                                                       "Timecode: the steps run from the first GO; the MIDI timecode "
                                                       "isn't followed");
        else
            result.lost << QCoreApplication::translate("QlcExchange",
                                                       "Timecode: the steps run from the first GO; the timeline of a "
                                                       "super scene isn't followed");
        // the clock of the list starts at 00:00 and holds nothing until the
        // first time (as Cuelist::tcFirstLocked); the steps play the first
        // one at once. (A MIDI timecode isn't followed at all: its line says
        // enough)
        int first = -1;
        for (const Cuelist::Cue &cue : std::as_const(cues))
            if (cue.time >= 0 && (first < 0 || cue.time < first))
                first = cue.time;
        if (cl->tcSource() != Cuelist::TcMtc && first > 0)
            result.lost << QCoreApplication::translate("QlcExchange",
                                                       "Timecode: the first step plays at the GO, not %1 after it")
                               .arg(secondsText(uint(first)));
    }
    else if (mode == Cuelist::ModeSubmaster)
    {
        result.lost << QCoreApplication::translate("QlcExchange", "Submaster: the steps start with a GO, not with the fader");
    }
    else if (mode == Cuelist::ModeOverride)
    {
        result.lost << QCoreApplication::translate("QlcExchange", "Override: the steps mix with what's below like any others");
    }
    if (fxCues > 0)
        result.lost << QCoreApplication::translate("QlcExchange", "FX in %n cue(s): only their base values",
                                                   nullptr, fxCues);
    if (delays > 0)
        result.lost << QCoreApplication::translate("QlcExchange", "delay in %n cue(s): the step starts at once",
                                                   nullptr, delays);
    if (downFades > 0)
        result.lost << QCoreApplication::translate("QlcExchange", "down fade in %n cue(s): only approximate",
                                                   nullptr, downFades);
    if (ownTimes > 0)
        result.lost << QCoreApplication::translate("QlcExchange",
                                                   "times per attribute or fixture in %n cue(s): only the cue's fade",
                                                   nullptr, ownTimes);
    if (curves > 0)
        result.lost << QCoreApplication::translate("QlcExchange", "fade curves in %n cue(s): only linear",
                                                   nullptr, curves);
    if (shortWaits > 0)
        result.lost << QCoreApplication::translate("QlcExchange",
                                                   "Wait shorter than the fade in %n cue(s): the next step waits for the fade",
                                                   nullptr, shortWaits);
    if (longFades > 0)
        result.lost << QCoreApplication::translate("QlcExchange",
                                                   "fade longer than the step in %n cue(s): the next step waits for the fade",
                                                   nullptr, longFades);
    if (untimed > 0)
        result.lost << QCoreApplication::translate("QlcExchange", "no time in %n cue(s): the steps stop there until a GO",
                                                   nullptr, untimed);
    if (links > 0)
        result.lost << QCoreApplication::translate("QlcExchange", "links in %n cue(s): the steps play in order",
                                                   nullptr, links);
    if (actions > 0)
        result.lost << QCoreApplication::translate("QlcExchange", "actions in %n cue(s): they don't run",
                                                   nullptr, actions);
    if (restUsed)
        result.lost << QCoreApplication::translate("QlcExchange", "released values: their rest value");
    // (a Chase loops whatever it says)
    if (chase == false && atEnd == Cuelist::AtEndStay)
        result.lost << QCoreApplication::translate("QlcExchange", "Stay at the end: GO on the last cue stops the steps");
    if (cl->driveMode() == Function::DriveBeatGO || cl->driveMode() == Function::DrivePulse)
        result.lost << QCoreApplication::translate("QlcExchange", "BeatGO and Pulse: the steps don't follow the beat");
    // (a Chase said it above)
    if (chase == false && cl->tapPeriod() > 0)
        result.lost << QCoreApplication::translate("QlcExchange", "tapped rhythm: the steps keep their own times");
    // the fixtures dark that the next cue lights up don't take its moves
    // before it (not in a Chase nor an Override list, nor when every cue
    // shows its moves)
    if (cl->moveInBlack() && chase == false && mode != Cuelist::ModeOverride && prepared)
        result.lost << QCoreApplication::translate("QlcExchange", "Move in black: the fixtures move as they fade up");
    if (result.solo)
        result.lost << QCoreApplication::translate("QlcExchange", "released by another GO: in a solo frame of QLC+");
    return result;
}

QString cueLabel(int number, const QString &name)
{
    QString label = QStringLiteral("Q") + Cuelist::numberToString(number);
    if (name.isEmpty() == false)
        label += NameSeparator + oneLine(name);
    return label;
}

QString cueNote(int number, const QString &name, const QString &note)
{
    QString text = cueLabel(number, name);
    if (note.isEmpty() == false)
        text += NoteSeparator + oneLine(note);
    return text;
}

bool parseCueLabel(const QString &text, int *number, QString *name, QString *rest)
{
    // Q, up to 5 digits and 3 decimals, then " · name" and " — note"
    static const QRegularExpression label(
        QRegularExpression::anchoredPattern(QStringLiteral("Q([0-9]{1,5}(?:\\.[0-9]{1,3})?)"
                                                           "(?: · (.*?))?(?: — (.*))?")),
        QRegularExpression::DotMatchesEverythingOption);
    const QRegularExpressionMatch match = label.match(text);
    if (match.hasMatch() == false)
        return false;
    const int parsed = Cuelist::parseNumber(match.captured(1));
    if (parsed < 0)
        return false;

    if (number != nullptr)
        *number = parsed;
    if (name != nullptr)
        *name = match.captured(2);
    if (rest != nullptr)
        *rest = match.captured(3);
    return true;
}

/*********************************************************************
 * QLC+ steps to cues
 *********************************************************************/

bool stepsToCues(Doc *doc, const Chaser *chaser, int beatMs, StepsImport *out, QString *error)
{
    if (doc == nullptr || chaser == nullptr || out == nullptr)
        return false;
    *out = StepsImport();
    const Sequence *sequence = qobject_cast<const Sequence *>(chaser);
    QList<ChaserStep> steps = chaser->steps();
    if (steps.isEmpty())
    {
        setError(error, QCoreApplication::translate("QlcExchange", "These steps are empty: there is nothing to convert"));
        return false;
    }

    // how many times the steps play, 0 for ever ("Play once" of older shows
    // is one loop, as the playback panel shows it). A few loops play out as
    // cues, one after the other
    const QList<int> cycle = cycleOf(int(steps.count()), chaser->runOrder());
    const int loops = chaser->runOrder() == Function::SingleShot && chaser->loopCount() == 0
                          ? 1 : qMax(0, chaser->loopCount());
    const qint64 cueCount = qint64(cycle.count()) * qMax(1, loops);
    if (cueCount > MaxCues)
    {
        setError(error, QCoreApplication::translate("QlcExchange",
                                                    "These steps play too many times to become cues: lower the number of times first"));
        return false;
    }
    QList<int> order;
    for (int c = 0; c < int(cueCount); c++)
        order.append(cycle.at(c % cycle.count()));

    // what each step of a Chaser shows (numbered as the user sees them)
    QList<Look> looks;
    if (sequence == nullptr)
    {
        for (int i = 0; i < steps.count(); i++)
        {
            looks.append(stepLook(doc, steps.at(i), i + 1));
            if (looks.last().warning.isEmpty() == false)
                out->warnings << looks.last().warning;
            if (looks.last().converted == false)
                out->notConverted++;
        }
    }
    if (chaser->direction() == Function::Backward)
    {
        std::reverse(steps.begin(), steps.end());
        std::reverse(looks.begin(), looks.end());
    }
    if (chaser->runOrder() == Function::Random)
        out->warnings << QCoreApplication::translate("QlcExchange", "random order: they play in order");
    if (chaser->tempoType() == Function::Beats)
    {
        const int bpm = beatMs > 0 ? qRound(60000.0 / beatMs) : 0;
        out->warnings << QCoreApplication::translate("QlcExchange", "in beats: in seconds at %1 BPM").arg(bpm);
    }

    // a list from a copy for QLC+ comes back with its numbers and names: in
    // the notes of a Sequence, the names of the scenes of a Chaser
    QStringList labels;
    for (int i : std::as_const(order))
        labels << (sequence != nullptr ? steps.at(i).note : functionName(doc, steps.at(i).fid));
    QList<int> numbers;
    QStringList names, rests;
    const bool labelled = parseLabels(labels, &numbers, &names, &rests);

    const StepTimes times(doc, chaser, beatMs);
    // driven by the beats or the pulses: each division is a GO, the times don't count
    const bool driven = chaser->driveMode() == Function::DriveBeatGO || chaser->driveMode() == Function::DrivePulse;

    // a Sequence: cue 1 holds every channel of the steps (zeros are real),
    // the next cues what changes; the first step starts from zero. A Chaser:
    // each cue goes from the look of the step before to its own
    QHash<Key, uchar> stage;
    // (and the palettes the values of the steps come from)
    QHash<Key, int> stageRefs;
    QList<FXScene::Layer> tracked;
    if (const Scene *bound = qobject_cast<const Scene *>(times.bound))
        for (const SceneValue &scv : bound->values())
            stage.insert(Key(scv.fxi, scv.channel), 0);

    for (int c = 0; c < order.count(); c++)
    {
        const int i = order.at(c);
        const ChaserStep &step = steps.at(i);
        Cuelist::Cue cue;
        if (sequence != nullptr)
        {
            setStepChanges(&cue, c == 0, &stage, &stageRefs, step);
        }
        else
        {
            const Look &look = looks.at(i);
            setChanges(&cue, c == 0, stage, tracked, look.values, look.fx);
            stage = look.values;
            tracked = look.fx;
        }

        cue.number = labelled ? numbers.at(c) : (c + 1) * 1000;
        cue.name = labelled ? names.at(c) : sequence != nullptr ? QString() : labels.at(c);
        cue.note = labelled && sequence != nullptr ? rests.at(c) : step.note;
        // what of the step doesn't play, first
        if (sequence == nullptr && looks.at(i).warning.isEmpty() == false)
            cue.note = cue.note.isEmpty() ? looks.at(i).warning : looks.at(i).warning + QLatin1Char('\n') + cue.note;

        cue.fade = finite(times.fadeIn(step));
        const uint fadeOut = finite(times.fadeOut(step));
        cue.downFade = fadeOut == cue.fade ? -1 : int(fadeOut);

        // each cue starts when the step before it ends; an endless step waits for a GO
        if (c > 0 && driven == false)
        {
            const uint duration = times.duration(steps.at(order.at(c - 1)));
            if (duration != Function::infiniteSpeed())
            {
                cue.trigger = Cuelist::TriggerWait;
                cue.triggerTime = duration;
            }
        }
        out->cues.append(cue);
    }

    // a release takes the fade out of the steps
    out->releaseTime = finite(times.fadeOut(steps.first()));
    if (driven)
    {
        out->drive = chaser->driveMode();
        out->divisions = chaser->divisions();
    }

    // for ever: a loop, the first cue follows the last step. Played out: it
    // holds the last look, or it's released when the last step ends (the
    // trigger of the first cue goes on past the last one, as in a loop)
    const bool holds = loops > 0 && chaser->endAction() == Function::EndPause;
    out->atEnd = loops == 0 ? Cuelist::AtEndLoop : holds ? Cuelist::AtEndStay : Cuelist::AtEndRelease;
    const uint lastDuration = times.duration(steps.at(order.last()));
    if (holds == false && driven == false && lastDuration != Function::infiniteSpeed())
    {
        out->cues[0].trigger = Cuelist::TriggerWait;
        out->cues[0].triggerTime = lastDuration;
    }
    return true;
}

Cuelist *newCuelist(Doc *doc, const Chaser *chaser, const StepsImport &in)
{
    Cuelist *cl = new Cuelist(doc);
    cl->setName(chaser->name());
    cl->setPriority(chaser->priority());
    cl->setPath(chaser->path(true));
    cl->setVisible(chaser->isVisible());
    cl->setFadeOutSpeed(in.releaseTime);
    if (in.drive != Function::DriveOff)
    {
        cl->setDriveMode(in.drive);
        cl->setDivisions(in.divisions);
    }
    cl->setAtEnd(in.atEnd);
    cl->setCues(in.cues);
    return cl;
}

/*********************************************************************
 * A copy of the show for QLC+
 *********************************************************************/

void copyToken(QXmlStreamReader &reader, QXmlStreamWriter &writer)
{
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

QByteArray copyElement(QXmlStreamReader &reader)
{
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

void writeEngine(QXmlStreamReader &reader, QXmlStreamWriter &writer, const QSet<quint32> &replaced,
                 const std::function<void(QXmlStreamWriter *)> &writeFunctions)
{
    bool written = false;
    auto writeNew = [&]() {
        if (written == false && writeFunctions)
            writeFunctions(&writer);
        written = true;
    };
    // the functions of this program go in an element QLC+ skips: on the
    // first one, QLC+ would stop reading the workspace (its Virtual Console too)
    bool wrapped = false;
    auto unwrap = [&]() {
        if (wrapped)
            writer.writeEndElement();
        wrapped = false;
    };

    // (depth 1: the children of the Engine)
    int depth = 0;
    while (reader.hasError() == false)
    {
        if (reader.isStartElement())
        {
            bool dtcFunction = false;
            if (depth == 1 && reader.name() == KXMLQLCFunction)
            {
                const QXmlStreamAttributes attrs = reader.attributes();
                const Function::Type type = Function::stringToType(attrs.value(KXMLQLCFunctionType).toString());
                if (type == Function::CuelistType && replaced.contains(attrs.value(KXMLQLCFunctionID).toUInt()))
                {
                    // its Sequence or Chaser takes its place
                    reader.skipCurrentElement();
                    reader.readNext();
                    continue;
                }
                dtcFunction = Function::isDtcType(type);
            }
            if (dtcFunction)
            {
                // (the new steps before them, where QLC+ reads them)
                writeNew();
                if (wrapped == false)
                    writer.writeStartElement(KXMLDTCFunctions);
                wrapped = true;
            }
            else if (depth == 1)
            {
                unwrap();
            }
            depth++;
        }
        else if (reader.isEndElement())
        {
            depth--;
            if (depth == 0)
            {
                unwrap();
                writeNew();
            }
        }
        copyToken(reader, writer);
        if (depth <= 0 || reader.atEnd())
            break;
        reader.readNext();
    }
}

QByteArray newVirtualConsole(const QList<VcCueList> &lists, quint32 firstId)
{
    QByteArray xml;
    QXmlStreamWriter writer(&xml);
    quint32 nextId = firstId;
    writer.writeStartElement("VirtualConsole");
    writeRootFrame(writer, lists, &nextId);
    writeProperties(writer, grownSize(ConsoleSize, framesArea(lists, QPoint(Gap, Gap))));
    writer.writeEndElement();
    return xml;
}

QByteArray mergeVirtualConsole(const QByteArray &section, const QList<VcCueList> &lists,
                               QSet<quint32> *alreadyThere)
{
    const ConsoleScan scan = scanConsole(section);
    QList<VcCueList> added;
    for (const VcCueList &list : lists)
    {
        if (scan.chasers.contains(list.functionId) == false)
            added.append(list);
        else if (alreadyThere != nullptr)
            alreadyThere->insert(list.functionId);
    }
    if (added.isEmpty())
        return section;

    // below what the first frame holds
    quint32 nextId = quint32(scan.lastId + 1);
    const QPoint at(Gap, scan.bottom + Gap);
    const QRect area = framesArea(added, at);

    QByteArray xml;
    QXmlStreamReader reader(section);
    QXmlStreamWriter writer(&xml);
    QStringList path;
    int frames = 0;
    while (reader.atEnd() == false && reader.hasError() == false)
    {
        reader.readNext();
        if (reader.isStartElement())
        {
            path.append(reader.name().toString());
            if (path.count() == 2 && path.last() == QLatin1String("Frame"))
                frames++;
            if (path.count() == 3 && path.last() == QLatin1String("Size") && path.at(1) == QLatin1String("Properties"))
            {
                writeGrownSize(reader, writer, area);
                continue;
            }
        }
        else if (reader.isEndElement() && path.isEmpty() == false)
        {
            const int depth = path.count();
            const QString name = path.takeLast();
            if (depth == 2 && name == QLatin1String("Frame") && frames == 1)
            {
                writeFrames(writer, added, at, &nextId);
            }
            else if (depth == 2 && name == QLatin1String("Properties") && scan.hasSize == false)
            {
                writeSize(writer, grownSize(ConsoleSize, area));
            }
            else if (depth == 1)
            {
                // (a console with no frame, or no properties)
                if (scan.hasFrame == false)
                    writeRootFrame(writer, added, &nextId);
                if (scan.hasProperties == false)
                    writeProperties(writer, grownSize(ConsoleSize, area));
            }
        }
        copyToken(reader, writer);
    }
    return xml;
}

/*********************************************************************
 * The cue lists of a QLC+ show
 *********************************************************************/

QList<VcCueListInfo> readVirtualConsole(const QByteArray &section)
{
    QList<VcCueListInfo> lists;
    QXmlStreamReader reader(section);
    if (reader.readNextStartElement() && reader.name() == QLatin1String("VirtualConsole"))
        readWidgets(reader, &lists, 0);
    return lists;
}

QList<Candidate> candidates(const Doc *doc, const QByteArray &virtualConsole)
{
    QList<Candidate> list;
    if (doc == nullptr)
        return list;

    // the Cue Lists, one for each Sequence or Chaser they play
    QHash<quint32, qsizetype> byFunction;
    for (const VcCueListInfo &info : readVirtualConsole(virtualConsole))
    {
        if (info.legacy)
        {
            list.append(legacyCandidate(info));
            continue;
        }
        const Chaser *chaser = qobject_cast<const Chaser *>(doc->function(info.chaserId));
        if (chaser == nullptr)
            continue;
        auto known = byFunction.constFind(chaser->id());
        if (known != byFunction.cend())
        {
            list[known.value()].lists.append(info);
            continue;
        }
        Candidate candidate;
        candidate.key = QStringLiteral("vc:%1").arg(info.index);
        candidate.kind = QStringLiteral("vcCueList");
        candidate.name = info.caption.isEmpty() ? chaser->name() : info.caption;
        candidate.functionId = chaser->id();
        candidate.steps = chaser->stepsCount();
        candidate.lists << info;
        byFunction.insert(chaser->id(), list.count());
        list.append(candidate);
    }
    for (Candidate &candidate : list)
    {
        if (std::any_of(candidate.lists.cbegin(), candidate.lists.cend(), hasSideFader))
            candidate.warnings << QCoreApplication::translate("QlcExchange", "its side fader isn't used");
    }

    // then the steps no Cue List plays (the hidden scenes of the Sequences
    // aren't steps)
    for (const Function *f : doc->functions())
    {
        const Chaser *chaser = qobject_cast<const Chaser *>(f);
        if (chaser == nullptr || chaser->isVisible() == false || byFunction.contains(chaser->id()))
            continue;
        Candidate candidate;
        candidate.key = QStringLiteral("fn:%1").arg(chaser->id());
        candidate.kind = Function::typeToString(chaser->type());
        candidate.name = chaser->name();
        candidate.functionId = chaser->id();
        candidate.steps = chaser->stepsCount();
        list.append(candidate);
    }
    return list;
}

void checkSteps(Doc *doc, Candidate *candidate, int beatMs)
{
    const Chaser *chaser = doc != nullptr ? qobject_cast<const Chaser *>(doc->function(candidate->functionId)) : nullptr;
    if (candidate->enabled == false || chaser == nullptr)
        return;
    StepsImport steps;
    QString error;
    if (stepsToCues(doc, chaser, beatMs, &steps, &error) == false)
    {
        candidate->enabled = false;
        candidate->warnings.prepend(error);
        return;
    }
    candidate->warnings = steps.warnings + candidate->warnings;
}

Cuelist *toCuelist(Doc *doc, const Candidate &candidate, int beatMs, StepsImport *out, QString *error)
{
    const Chaser *chaser = doc != nullptr ? qobject_cast<const Chaser *>(doc->function(candidate.functionId)) : nullptr;
    if (candidate.enabled == false || chaser == nullptr)
    {
        setError(error, candidate.warnings.value(0));
        return nullptr;
    }
    if (stepsToCues(doc, chaser, beatMs, out, error) == false)
        return nullptr;

    Cuelist *cl = newCuelist(doc, chaser, *out);
    cl->setName(candidate.name);
    // (QLC+: RunNext goes on from the step it was stopped on)
    if (candidate.lists.isEmpty() == false)
        cl->setResetOnRelease(candidate.lists.first().nextPrev != 1);
    return cl;
}

/*********************************************************************
 * The keys and the controller buttons of the Cue Lists
 *********************************************************************/

QStringList cueListTargets()
{
    return { QStringLiteral("playbackGo"), QStringLiteral("playbackBack"), QStringLiteral("playbackPauseBack"),
             QStringLiteral("playbackRelease") };
}

VcControl controlFrom(const QVariantMap &mapping, const ControlMapper *mapper, QStringList *lost)
{
    VcControl control;
    const QString source = mapping.value("source").toString();
    if (source == QLatin1String("key"))
    {
        // as QLC+ writes it: "N", "Space", "F9"
        const QKeySequence key(mapping.value("key").toString(), QKeySequence::PortableText);
        if (key.count() == 1)
            control.key = key.toString(QKeySequence::PortableText);
        return control;
    }
    if (source == QLatin1String("osc"))
    {
        // (QLC+ keeps a hash of the path, not the path)
        addLine(lost, QCoreApplication::translate("QlcExchange", "OSC controls aren't carried"));
        return control;
    }
    if (mapper == nullptr)
        return control;

    // MIDI and DMX inputs: the universe of their device
    const QString device = mapping.value("device").toString();
    int universe = -1;
    if (device.isEmpty())
    {
        universe = onlyUniverse(mapper, source);
        if (universe < 0)
            addLine(lost, QCoreApplication::translate("QlcExchange",
                                                      "controls on any device aren't carried: choose their device in Mappings"));
    }
    else
    {
        universe = mapper->universeOf(device);
        if (universe < 0)
            addLine(lost, QCoreApplication::translate("QlcExchange", "%1 isn't enabled: its controls aren't carried")
                              .arg(device.section('|', 1)));
    }
    if (universe >= 0)
    {
        control.universe = universe;
        control.channel = mapping.value("channel").toUInt();
    }
    return control;
}

void controlsOf(quint32 functionId, const ControlMapper *mapper, const PlaybackManager *playbacks,
                VcCueList *list, QStringList *lost)
{
    if (mapper == nullptr || list == nullptr)
        return;
    QList<QVariantMap> mappings;
    for (const QVariant &v : mapper->mappings())
        mappings.append(v.toMap());
    std::stable_sort(mappings.begin(), mappings.end(), [](const QVariantMap &a, const QVariantMap &b) {
        return a.value("id").toInt() < b.value("id").toInt();
    });

    // Next, Previous, Playback, Stop: one key and one input each
    const QStringList targets = cueListTargets();
    VcControl *buttons[] = { &list->next, &list->previous, &list->playback, &list->stop };
    bool more = false;
    for (const QVariantMap &m : std::as_const(mappings))
    {
        const qsizetype button = targets.indexOf(m.value("target").toString());
        if (button < 0 || drives(m, functionId, playbacks) == false)
            continue;
        const VcControl control = controlFrom(m, mapper, lost);
        VcControl *to = buttons[button];
        if (control.key.isEmpty() == false)
        {
            if (to->key.isEmpty())
                to->key = control.key;
            else
                more = true;
        }
        if (control.universe >= 0)
        {
            if (to->universe < 0)
            {
                to->universe = control.universe;
                to->channel = control.channel;
            }
            else
            {
                more = true;
            }
        }
    }
    if (more)
        addLine(lost, QCoreApplication::translate("QlcExchange",
                                                  "more controls for the same button: QLC+ takes the first key and the first input"));
}

QList<QVariantMap> mappingsFrom(const VcControl &control, const QString &target, quint32 functionId,
                                const ControlMapper *mapper, QStringList *lost)
{
    QList<QVariantMap> made;
    if (mapper == nullptr)
        return made;
    const QVariantList existing = mapper->mappings();
    auto add = [&](const QVariantMap &m) {
        if (mappedAlready(existing, m))
            addLine(lost, QCoreApplication::translate("QlcExchange", "already mapped: %1").arg(mapper->describeSource(m)));
        else
            made.append(m);
    };

    if (control.key.isEmpty() == false)
    {
        // a key alone, as the mappings of keys are
        const QKeySequence sequence(control.key, QKeySequence::PortableText);
        const QKeyCombination combination = sequence.isEmpty() ? QKeyCombination() : sequence[0];
        const bool alone = sequence.count() == 1 && combination.keyboardModifiers() == Qt::NoModifier;
        const QString key = alone ? ControlMapper::keyName(int(combination.key())) : QString();
        if (alone == false)
            addLine(lost, QCoreApplication::translate("QlcExchange", "keys with Ctrl, Alt or Shift aren't mapped"));
        else if (key.isEmpty())
            addLine(lost, QCoreApplication::translate("QlcExchange", "the key %1 can't be mapped").arg(control.key));
        else
            add({ { "source", "key" }, { "key", key }, { "target", target }, { "functionId", functionId },
                  { "feedback", false } });
    }

    if (control.universe >= 0)
    {
        QString kind;
        const QString device = mapper->deviceKeyOf(quint32(control.universe), &kind);
        if (device.isEmpty())
            addLine(lost, QCoreApplication::translate("QlcExchange", "no controller on input universe %1")
                              .arg(control.universe + 1));
        else if (kind == QLatin1String("osc"))
            addLine(lost, QCoreApplication::translate("QlcExchange", "OSC controls aren't carried"));
        else
        {
            QVariantMap m = { { "source", kind }, { "device", device }, { "channel", control.channel },
                              { "target", target }, { "functionId", functionId },
                              { "feedback", kind == QLatin1String("midi") } };
            // a known controller: its kind and the LEDs of its buttons, as
            // learning gives them (the buttons of a Cue List are buttons)
            const QVariantMap known = ControlMapper::controllerDefaults(kind, device, control.channel, true);
            for (auto it = known.cbegin(); it != known.cend(); ++it)
                m.insert(it.key(), it.value());
            add(m);
        }
    }
    return made;
}

} // namespace QlcExchange

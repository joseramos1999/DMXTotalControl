/*
  DMX Total Control
  qlcexchange.h

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

#ifndef QLCEXCHANGE_H
#define QLCEXCHANGE_H

#include <QStringList>
#include <QByteArray>
#include <QVariantMap>
#include <QString>
#include <QList>
#include <QSet>
#include <functional>

#include "scenevalue.h"
#include "function.h"
#include "cuelist.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class PlaybackManager;
class ControlMapper;
class Chaser;
class Doc;

/**
 * Cuelists as the steps of QLC+ (a Sequence or a Chaser), and what QLC+
 * won't get of them; and the steps of QLC+ as cues.
 *
 * QLC+ has no tracking: each cue becomes one step that holds the whole look
 * of that cue (what the stage shows on it). The step note keeps the number
 * and the name of the cue ("Q2.5 · Verse"), so they can come back.
 */
namespace QlcExchange
{
    /** One step of QLC+: one cue */
    struct Step
    {
        /** sorted by (fixture, channel) */
        QList<SceneValue> values;
        /** ms: the fade of the cue */
        uint fadeIn = 0;
        /** ms the step stays after its fade in, until the next cue starts:
         *  Function::infiniteSpeed() when the next cue waits for a GO */
        uint hold = 0;
        /** ms: the down fade of the cue */
        uint fadeOut = 0;
        /** "Q2.5 · Verse": the name of the scene of the cue in a Chaser */
        QString label;
        /** the note of the step: the label and the cue note in a Sequence,
         *  the cue note in a Chaser */
        QString note;
    };

    /** A cuelist as steps */
    struct CueSteps
    {
        /** every channel of the steps, sorted, at 0: the hidden scene of a Sequence */
        QList<SceneValue> channels;
        QList<Step> steps;
        /** Loop after the last cue, or SingleShot (it stops there). A Chase:
         *  Loop, PingPong (Bounce) or Random */
        Function::RunOrder runOrder = Function::Loop;
        /** Backward: a Chase that plays its cues the other way */
        Function::Direction direction = Function::Forward;
        /** for a Cue List of the QLC+ Virtual Console: 0 a new start runs the
         *  first step, 1 the step after the one it was stopped on */
        int nextPrevBehavior = 0;
        /** released by the GO of another cuelist: a solo frame of QLC+ */
        bool solo = false;
        /** what QLC+ won't get, one translated line each */
        QStringList lost;
    };

    /**
     * The steps of cuelist $cl. Only reads it: a list that plays goes on.
     * Each step lasts until the next cue starts by itself: by its trigger
     * (a last cue linked always to the first one loops); in a Chase, a step
     * of its speed (its BPM or the show tempo now, by its multiplier) with
     * its crossfade; in a Timecode list, until the time of the next cue (a
     * cue without a time waits for a GO).
     * @param fillRest true for a Sequence: every step holds every channel,
     *        those the cue doesn't show (released, not set yet) at their rest
     *        value. false for a Chaser: only what the cue shows, the rest is
     *        left to QLC+
     */
    CueSteps toSteps(const Doc *doc, const Cuelist *cl, bool fillRest);

    /** "Q2.5 · Verse" ($number in thousandths), "Q3" without a name */
    QString cueLabel(int number, const QString &name);
    /** The label and the note on one line: "Q2.5 · Verse — fog on" */
    QString cueNote(int number, const QString &name, const QString &note);
    /**
     * Reads a label (or a note made by cueNote) back.
     * @param name, rest set to the name and to what follows " — " (they
     *        may be nullptr)
     * @return false when $text is not a cue label
     */
    bool parseCueLabel(const QString &text, int *number, QString *name, QString *rest);

    /*********************************************************************
     * QLC+ steps to cues
     *********************************************************************/

    /** The steps of a Sequence or a Chaser as cues, and how the list ends */
    struct StepsImport
    {
        QList<Cuelist::Cue> cues;
        Cuelist::AtEnd atEnd = Cuelist::AtEndLoop;
        /** ms: the fade out of the first step */
        uint releaseTime = 0;
        /** BeatGO and Pulse: a GO each division (the times don't count) */
        Function::DriveMode drive = Function::DriveOff;
        int divisions = 1;
        /** what doesn't play the same, one translated line each */
        QStringList warnings;
        /** the steps of a Chaser that are no scene nor FX (an empty cue each) */
        int notConverted = 0;
    };

    /**
     * The cues of the steps of $chaser (a Sequence is a Chaser). Only reads it.
     * Sequence: cue 1 holds every channel of the steps, the next cues what
     * changes. Chaser: each cue holds the scene of its step (Move FX and
     * Curve FX become FX) and releases what the step before had and it
     * doesn't; a step that is no scene nor FX gives an empty cue with a note.
     * Loops play out as cues, ping-pong there and back, a random order
     * forward. Steps labelled as cues (a copy for QLC+, see cueNote) get
     * their numbers and names back. What doesn't play the same goes to
     * $out->warnings.
     * @param beatMs a beat in ms, for steps in beats
     * @return false (and $error) when the steps can't become cues
     */
    bool stepsToCues(Doc *doc, const Chaser *chaser, int beatMs, StepsImport *out, QString *error);
    /** A cuelist with the cues of $in and what $chaser is (name, priority,
     *  folder, visible): not added to $doc */
    Cuelist *newCuelist(Doc *doc, const Chaser *chaser, const StepsImport &in);

    /*********************************************************************
     * A copy of the show for QLC+
     *********************************************************************/

    /** What plays a button of a Cue List of the Virtual Console: a key
     *  and/or a channel of a controller (universe -1: none) */
    struct VcControl
    {
        QString key;
        int universe = -1;
        quint32 channel = 0;
    };

    /** A Cue List of the QLC+ Virtual Console, playing a Sequence or a Chaser */
    struct VcCueList
    {
        QString caption;
        quint32 functionId = 0;
        /** see CueSteps::nextPrevBehavior */
        int nextPrev = 0;
        /** in a solo frame: starting it stops the others */
        bool solo = false;
        /** Next is the GO, Previous the BACK, Playback Pause/Back, Stop the
         *  release: their keys and controller buttons (see controlsOf) */
        VcControl next, previous, playback, stop;
    };

    /** The current token of $reader, written to $writer by its qualified
     *  names (the elements stay in the namespace of the workspace) */
    void copyToken(QXmlStreamReader &reader, QXmlStreamWriter &writer);
    /** The element $reader is on as it is, from its start to its end, which
     *  stays the current token (like skipCurrentElement) */
    QByteArray copyElement(QXmlStreamReader &reader);

    /**
     * Copy the <Engine> $reader is on (its start) to $writer, up to its end,
     * without the cuelists in $replaced. $writeFunctions writes what takes
     * their place: before the first function of a type QLC+ doesn't know, or
     * at the end. The functions QLC+ doesn't know go in <DtcFunctions>: on
     * one of them QLC+ would stop reading the workspace.
     */
    void writeEngine(QXmlStreamReader &reader, QXmlStreamWriter &writer, const QSet<quint32> &replaced,
                     const std::function<void(QXmlStreamWriter *)> &writeFunctions);

    /** A <VirtualConsole> section as QLC+ 4 writes it, with the frames of
     *  $lists. The IDs of its widgets start at $firstId */
    QByteArray newVirtualConsole(const QList<VcCueList> &lists, quint32 firstId);
    /**
     * The <VirtualConsole> $section with the frames of $lists added at the
     * bottom of its first frame, with IDs no widget of it has, and its size
     * grown to hold them. A list whose function already has a Cue List there
     * gets no other one: its function goes to $alreadyThere (may be nullptr).
     */
    QByteArray mergeVirtualConsole(const QByteArray &section, const QList<VcCueList> &lists,
                                   QSet<quint32> *alreadyThere);

    /*********************************************************************
     * The cue lists of a QLC+ show
     *********************************************************************/

    /** A Cue List of the Virtual Console of a QLC+ show, as it is there */
    struct VcCueListInfo
    {
        /** its place among the Cue Lists of the console, in the file */
        int index = 0;
        /** its widget ID, -1: none */
        qint64 widgetId = -1;
        QString caption;
        /** the Sequence or Chaser it plays */
        quint32 chaserId = Function::invalidId();
        /** see CueSteps::nextPrevBehavior */
        int nextPrev = 0;
        /** its side fader: empty or "None" when it has none */
        QString slidersMode;
        /** the old format: functions as steps, without a Chaser (QLC+ makes
         *  one when it opens the show) */
        bool legacy = false;
        int legacySteps = 0;
        /** its Next, Previous, Playback and Stop buttons */
        VcControl next, previous, playback, stop;
    };

    /** The Cue Lists of the <VirtualConsole> $section, in the order of the
     *  file: in any frame and solo frame, on any page */
    QList<VcCueListInfo> readVirtualConsole(const QByteArray &section);

    /** Something of a QLC+ show that can become a cuelist */
    struct Candidate
    {
        /** "vc:<index of its first Cue List>" or "fn:<function ID>" */
        QString key;
        /** "vcCueList", "Chaser" or "Sequence" */
        QString kind;
        /** the caption of its Cue List, else the name of its steps */
        QString name;
        /** its Sequence or Chaser */
        quint32 functionId = Function::invalidId();
        int steps = 0;
        /** what doesn't play the same, one translated line each */
        QStringList warnings;
        /** false: it can't be converted (see warnings) */
        bool enabled = true;
        /** the Cue Lists that play it: the first one names it */
        QList<VcCueListInfo> lists;
    };

    /**
     * What of $doc can become a cuelist: the Cue Lists of its Virtual
     * Console $virtualConsole that play a Sequence or a Chaser (several on
     * the same steps are one), the old ones (they can't be converted), then
     * the visible Sequences and Chasers no Cue List plays.
     */
    QList<Candidate> candidates(const Doc *doc, const QByteArray &virtualConsole);
    /** The steps of $candidate tried as cues: what doesn't play the same
     *  goes to its warnings; steps that can't become cues disable it */
    void checkSteps(Doc *doc, Candidate *candidate, int beatMs);
    /**
     * The cuelist $candidate becomes, not added to $doc: named like it, and
     * a new start runs its first cue unless its Cue List goes on from the
     * step it was stopped on.
     * @param out its cues and what doesn't play the same
     * @return nullptr (and $error) when it can't be converted
     */
    Cuelist *toCuelist(Doc *doc, const Candidate &candidate, int beatMs, StepsImport *out, QString *error);

    /*********************************************************************
     * The keys and the controller buttons of the Cue Lists
     *********************************************************************/

    /** The playback targets of the buttons of a Cue List, in the order
     *  Next, Previous, Playback, Stop: GO, Back, Pause/Back and Release */
    QStringList cueListTargets();

    /**
     * The button of a Cue List that $mapping is: its key, or the universe
     * and the channel of its controller. What QLC+ can't have gives an empty
     * control and a line in $lost: OSC, a device that isn't enabled, "any
     * device" when not just one device of its kind is enabled.
     */
    VcControl controlFrom(const QVariantMap &mapping, const ControlMapper *mapper, QStringList *lost);
    /**
     * The buttons of the Cue List of the cuelist $functionId, from the
     * mappings of $mapper on its GO, Back, Pause/Back and Release: bound to
     * it, or to the playback that holds it now (the current page as it is).
     * The first key and the first controller input of each button go, by
     * mapping ID. What QLC+ won't get goes to $lost (may be nullptr).
     */
    void controlsOf(quint32 functionId, const ControlMapper *mapper, const PlaybackManager *playbacks,
                    VcCueList *list, QStringList *lost);
    /**
     * The mappings that button $control of a Cue List becomes: $target (see
     * cueListTargets) of the function $functionId, on its key and on the
     * controller of its input universe in this show (a known controller
     * with the LEDs learning gives, see ControlMapper::controllerDefaults). What can't be mapped
     * goes to $lost: keys with Ctrl, Alt or Shift, OSC, a universe with no
     * controller, a control that has a mapping already.
     */
    QList<QVariantMap> mappingsFrom(const VcControl &control, const QString &target, quint32 functionId,
                                    const ControlMapper *mapper, QStringList *lost);
}

#endif

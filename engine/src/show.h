/*
  Q Light Controller Plus
  show.h

  Copyright (c) Massimo Callegari

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

#ifndef SHOW_H
#define SHOW_H

#include <QMutex>
#include <QList>
#include <QSet>

#include "function.h"
#include "track.h"

class QXmlStreamReader;
class ShowRunner;

/** @addtogroup engine_functions Functions
 * @{
 */

class Show final : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(Show)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    Show(Doc* doc);
    virtual ~Show();

    /** @reimp */
    QIcon getIcon() const override;

    /** @reimp */
    quint32 totalDuration() override;

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimp */
    Function* createCopy(Doc* doc, bool addToDoc = true) override;

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function) override;

    /*********************************************************************
     * Time division
     *********************************************************************/
public:
    enum TimeDivision
    {
        Time = 0,
        BPM_4_4,
        BPM_3_4,
        BPM_2_4,
        Invalid
    };
    Q_ENUM(TimeDivision)

    /** Set the show time division type (Time, BPM) */
    void setTimeDivision(Show::TimeDivision type, int BPM);

    Show::TimeDivision timeDivisionType() const;
    void setTimeDivisionType(Show::TimeDivision type);
    int beatsDivision() const;

    int timeDivisionBPM() const;
    void setTimeDivisionBPM(int BPM);

    static QString tempoToString(Show::TimeDivision type);
    static Show::TimeDivision stringToTempo(const QString& tempo);

private:
    TimeDivision m_timeDivisionType;
    int m_timeDivisionBPM;

    /*********************************************************************
     * DMX Total Control: timeline settings
     *********************************************************************/
public:
    /** The end of the show (ms); 0: the end of its last block */
    void setEndTime(quint32 ms);
    quint32 endTime() const;

    /** At the end, back to the start */
    void setLoop(bool loop);
    bool loop() const;

    /** Where the first bar starts (ms), for a grid in bars */
    void setGridOffset(quint32 ms);
    quint32 gridOffset() const;

    /** Blocks are placed in milliseconds whatever the tempo of their
     *  functions (the shows made in DMX Total Control) */
    void setMsTimeline(bool ms);
    bool msTimeline() const;

    /** A hidden scene with the static values of the show, played with it */
    void setStaticScene(quint32 id);
    quint32 staticScene() const;

private:
    quint32 m_endTime;
    bool m_loop;
    quint32 m_gridOffset;
    bool m_msTimeline;
    quint32 m_staticScene;

    /*********************************************************************
     * Tracks
     *********************************************************************/
public:
    /**
     * Add a track to this show. If the track is already a
     * member of the show, this call fails.
     *
     * @param id The track to add
     * @return true if successful, otherwise false
     */
    bool addTrack(Track *track, quint32 id = Track::invalidId());

    /**
     * Remove a track from this show. If the track is not a
     * member of the show, this call fails.
     *
     * @param id The track to remove
     * @return true if successful, otherwise false
     */
    bool removeTrack(quint32 id);

    /** Get a track by id */
    Track *track(quint32 id) const;

    /** Get a reference to a Track from the provided Scene ID */
    Track *getTrackFromSceneID(quint32 id) const;

    /** Get a reference to a Track from the provided ShowFunction ID */
    Track *getTrackFromShowFunctionID(quint32 id) const;

    /** Get the number of tracks in the Show */
    int getTracksCount() const;

    /** Move a track ID up or down */
    void moveTrack(Track *track, int direction);

    /** Get a list of available tracks */
    QList <Track*> tracks() const;

private:
    /** Create a new track ID */
    quint32 createTrackId();

protected:
    /** Map of the available tracks coupled by ID */
    QMap <quint32,Track*> m_tracks;

    /** Latest assigned track ID */
    quint32 m_latestTrackId;

    /*********************************************************************
     * Show Functions
     *********************************************************************/
public:
    /** Get a unique ID for the creation of a new ShowFunction */
    quint32 getLatestShowFunctionId();

    /** Get a reference to a ShowFunction from the provided uinique ID */
    ShowFunction *showFunction(quint32 id) const;

protected:
    /** Latest assigned unique ShowFunction ID */
    quint32 m_latestShowFunctionID;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save function's contents to an XML document */
    bool saveXML(QXmlStreamWriter *doc) const override;

    /** Load function's contents from an XML document */
    bool loadXML(QXmlStreamReader &root) override;

    /** @reimp */
    void postLoad() override;

public:
    /** @reimp */
    bool contains(quint32 functionId) const override;

    /** @reimp */
    QList<quint32> components() const override;

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimp */
    void preRun(MasterTimer* timer) override;

    /** @reimp */
    void setPause(bool enable) override;

    /** @reimp */
    void write(MasterTimer* timer, QList<Universe*> universes) override;

    /** @reimp */
    void postRun(MasterTimer* timer, QList<Universe*> universes) override;

protected slots:
    /** Called whenever one of this function's child functions stops */
    void slotChildStopped(quint32 fid);

    /** DMX Total Control: Doc::functionRemoved and Doc::functionReplaced,
     *  before the Doc deletes the function: the running blocks let it go (a
     *  replaced one plays the new function) */
    void slotFunctionRemoved(quint32 fid);
    void slotFunctionReplaced(quint32 fid);

signals:
    void timeChanged(quint32);
    void showFinished();

public:
    /** DMX Total Control: held while the runner plays (timer thread) and
     *  while the tracks or their blocks change (tracks, blocks, automation):
     *  the runner holds the blocks and reads the tracks */
    QRecursiveMutex *runnerMutex();

    /** DMX Total Control: a block of a track is about to be deleted, or a
     *  whole track: a running show forgets them (what they play stops) */
    void forgetBlock(ShowFunction *sf);

protected:
    ShowRunner *m_runner;
    QRecursiveMutex m_runnerMutex;
    /** Number of currently running children */
    QSet <quint32> m_runningChildren;

    /*************************************************************************
     * Attributes
     *************************************************************************/
public:
    /** @reimp */
    int adjustAttribute(qreal fraction, int attributeId = 0) override;
};

/** @} */

#endif

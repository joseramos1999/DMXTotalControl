/*
  Q Light Controller
  genericfader.h

  Copyright (c) Heikki Junnila

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

#ifndef GENERICFADER
#define GENERICFADER

// DMX Total Control: forEachChannel
#include <functional>
#include <QObject>
#include <QList>
#include <QHash>
#include <QSharedPointer>
#include <QReadWriteLock>

#include "universe.h"
#include "scenevalue.h"

class FadeChannel;

/** @addtogroup engine Engine
 * @{
 */

/**
 *  GenericFader represents all the fading channels for one Function (or feature)
 *  and one Universe. For example a Scene will request one GenericFader for all the
 *  channels of all the fixtures on a specific Universe.
 *  In this way, Universes will handle a list of dedicated faders, without
 *  any lookup
 */

class GenericFader final : public QObject
{
    Q_OBJECT

public:
    GenericFader(QObject *parent = 0);
    ~GenericFader();

    /** Get/Set an arbitrary name for this fader */
    QString name() const;
    void setName(QString name);

    /** Get/Set the ID of the Function controlling this fader.
     *  VC widgets won't set this. */
    quint32 parentFunctionID() const;
    void setParentFunctionID(quint32 fid);

    /** Get/Set a priority for this fader.
     *  The Universe class is in charge of sorting faders by priority */
    int priority() const;
    void setPriority(int priority);

    /** Get/Set if this fader should handle primary/secondary channels
     *  when a caller requests a FadeChannel */
    bool handleSecondary() const;
    void setHandleSecondary(bool enable);

    /** Build a hash for a fader channel which is unique in a Universe.
     *  This is used to map channels and access them quickly */
    static quint32 channelHash(quint32 fixtureID, quint32 channel);

    /**
     * Add a channel that shall be faded from ch.start() to ch.target() within
     * the time specified by ch.fadeTime(). If ch.target() == 0, the channel will
     * be removed automatically from the fader when done.
     *
     * If the fader already contains the same channel, the one whose current
     * value is higher remains in the fader. With LTP channels this might result
     * in the value jumping in a weird way but LTP channels are rarely faded anyway.
     * With HTP channels the lower value has no meaning in the first place.
     *
     * @param ch The channel to fade
     */
    void add(const FadeChannel& ch);

    /** Replace an existing FaderChannel */
    void replace(const FadeChannel& ch);

    /** Remove a channel whose fixture & channel match with $fc's */
    void remove(const FadeChannel *ch);

    /** Remove all channels */
    void removeAll();

    /** DMX Total Control: remove the channel $channel of the fixture
     *  $fixtureID (it doesn't need the fixture: it may be deleted) */
    void removeChannel(quint32 fixtureID, quint32 channel);

    /** DMX Total Control: a copy of the FadeChannel of the channel $channel
     *  of the fixture $fixtureID into $out (a coarse/fine pair is one, by its
     *  coarse channel). False when the fader doesn't hold it */
    bool channelCopy(quint32 fixtureID, quint32 channel, FadeChannel *out) const;

    /** DMX Total Control: the fixtures moved or changed (mode, options):
     *  the channels follow them (address, type), those of the fixtures gone
     *  or now in another universe than $universeID leave the fader */
    void refreshFixtures(const Doc *doc, quint32 universeID);

    /** Get/Set a request of deletion of this fader */
    bool deleteRequested();
    void requestDelete();

    /** Returns a reference of a FadeChannel for the provided $fixtureID and $channel.
     *  If no FadeChannel is found, a new one is created and added to m_channels.
     *  Also, new channels will have a start value set depending on their type */
    FadeChannel *getChannelFader(const Doc *doc, Universe *universe, quint32 fixtureID, quint32 channel);

    /** Get all channels in a non-modifiable hashmap */
    QHash <quint32,FadeChannel> channels() const;

    /** Return the number of channel added to this fader */
    int channelsCount() const;

    /**
     * Run the channels forward by one step and write their current values to
     * the given Universe
     *
     * @param universe The universe that receives channel data.
     * @param elapsedMs Time elapsed from previous write.
     */
    void write(Universe *universe, uint elapsedMs);

    /**
     * Update one channel while holding the internal channel lock.
     * This avoids exposing raw FadeChannel pointers across threads.
     */
    template <typename Updater>
    void updateChannel(const Doc *doc, Universe *universe, quint32 fixtureID, quint32 channel, Updater &&updater)
    {
        QWriteLocker l(&m_channelsLock);
        FadeChannel &fc = channelFaderLocked(doc, universe, fixtureID, channel);
        updater(fc);
    }

    /** DMX Total Control: changes every channel the fader holds, with the
     *  channel lock held, as updateChannel (a running fade gets new times) */
    void forEachChannel(const std::function<void(FadeChannel &)> &updater);

    /** Get/Set the intensities of all channels in a 0.0 - 1.0 range */
    qreal intensity() const;
    void adjustIntensity(qreal fraction);

    /** Get/Set an optional intensity for the fader parent Function */
    qreal parentIntensity() const;
    void setParentIntensity(qreal fraction);

    /** Get/Set the pause state of this fader */
    bool isPaused() const;
    void setPaused(bool paused);

    /** Get/Set the enable state of this fader */
    bool isEnabled() const;
    void setEnabled(bool enable);

    /** Get the fade out status of this fader */
    bool isFadingOut() const;

    /** Set this fader to fade out. If $fadeTime is non-zero,
      * all the intensity channels will be updated */
    /** DMX Total Control: $delays per fixture ID, for staggered fade outs */
    void setFadeOut(bool enable, uint fadeTime, const QHash<quint32, uint> &delays = QHash<quint32, uint>());

    /**
     * Set the blend mode to be applied in the write method
     *
     * @param mode the blend mode as listed in Universe::BlendMode
     */
    void setBlendMode(Universe::BlendMode mode);

    /** Enable/disable universe monitoring before writing new data */
    void setMonitoring(bool enable);

    /** Remove the Crossfade flag from every fader handled by this class */
    void resetCrossfade();

signals:
    /** Signal emitted when monitoring is enabled.
     *  Data is preGM and includes the whole universe */
    void preWriteData(quint32 index, const QByteArray& universeData);

private:
    QString m_name;
    quint32 m_fid;
    int m_priority;
    bool m_handleSecondary;
    QHash <quint32,FadeChannel> m_channels;
    mutable QReadWriteLock m_channelsLock;
    qreal m_intensity;
    qreal m_parentIntensity;
    bool m_paused;
    bool m_enabled;
    bool m_fadeOut;
    bool m_deleteRequest;
    Universe::BlendMode m_blendMode;
    bool m_monitoring;
    QHash<quint32, QSharedPointer<FadeChannel>> m_channelCache;

    FadeChannel &channelFaderLocked(const Doc *doc, Universe *universe, quint32 fixtureID, quint32 channel);

    /** DMX Total Control: what the faders below send on $count channels
     *  from $address, as one value (16 bit for a pair) */
    static quint32 valueBelow(const Universe *universe, quint32 address, int count);
    /** DMX Total Control: a MixBelow channel's value mixed with $below by
     *  $level (0: $below, 1: its own value) */
    static quint32 mixBelow(const FadeChannel &fc, quint32 below, qreal level);
    /** DMX Total Control: the ms a channel steps in a frame of $elapsedMs:
     *  0 for a Manual one (a hand crossfade sets its time), unless the
     *  fader fades out */
    uint stepOf(int flags, uint elapsedMs) const;
    /** DMX Total Control: a channel at its end leaves the fader (fading
     *  out, or AutoRemove); a Manual one stays, unless the fader fades out */
    bool leaves(int flags, const FadeChannel &fc) const;
};

/** @} */

#endif

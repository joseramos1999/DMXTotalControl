/*
  Q Light Controller
  genericfader.cpp

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

#include <QDebug>
#include <cmath>

#include "genericfader.h"
#include "fadechannel.h"
#include "fixture.h"
#include "doc.h"

GenericFader::GenericFader(QObject *parent)
    : QObject(parent)
    , m_fid(Function::invalidId())
    , m_priority(Universe::Auto)
    , m_handleSecondary(false)
    , m_intensity(1.0)
    , m_parentIntensity(1.0)
    , m_paused(false)
    , m_enabled(true)
    , m_fadeOut(false)
    , m_deleteRequest(false)
    , m_blendMode(Universe::NormalBlend)
    , m_monitoring(false)
{
}

GenericFader::~GenericFader()
{
}

QString GenericFader::name() const
{
    return m_name;
}

void GenericFader::setName(QString name)
{
    m_name = name;
}

quint32 GenericFader::parentFunctionID() const
{
    return m_fid;
}

void GenericFader::setParentFunctionID(quint32 fid)
{
    m_fid = fid;
}

int GenericFader::priority() const
{
    return m_priority;
}

void GenericFader::setPriority(int priority)
{
    m_priority = priority;
}

bool GenericFader::handleSecondary() const
{
    return m_handleSecondary;
}

void GenericFader::setHandleSecondary(bool enable)
{
    m_handleSecondary = enable;
}

quint32 GenericFader::channelHash(quint32 fixtureID, quint32 channel)
{
    return ((fixtureID & 0x0000FFFF) << 16) | (channel & 0x0000FFFF);
}

void GenericFader::add(const FadeChannel& ch)
{
    quint32 hash = channelHash(ch.fixture(), ch.channel());

    QWriteLocker l(&m_channelsLock);
    QHash<quint32,FadeChannel>::iterator channelIterator = m_channels.find(hash);
    if (channelIterator != m_channels.end())
    {
        // perform a HTP check
        if (channelIterator.value().current() <= ch.current())
            channelIterator.value() = ch;
    }
    else
    {
        m_channels.insert(hash, ch);
    }
}

void GenericFader::replace(const FadeChannel &ch)
{
    quint32 hash = channelHash(ch.fixture(), ch.channel());
    QWriteLocker l(&m_channelsLock);
    m_channels.insert(hash, ch);
}

void GenericFader::remove(const FadeChannel *ch)
{
    if (ch == NULL)
        return;

    quint32 hash = channelHash(ch->fixture(), ch->channel());
    QWriteLocker l(&m_channelsLock);
    if (m_channels.remove(hash) == 0)
        qDebug() << "No FadeChannel found with hash" << hash;
}

void GenericFader::removeAll()
{
    QWriteLocker l(&m_channelsLock);
    m_channels.clear();
    m_channelCache.clear();
}

void GenericFader::removeChannel(quint32 fixtureID, quint32 channel)
{
    // DMX Total Control
    const quint32 hash = channelHash(fixtureID, channel);
    QWriteLocker l(&m_channelsLock);
    m_channels.remove(hash);
    m_channelCache.remove(hash);
}

bool GenericFader::channelCopy(quint32 fixtureID, quint32 channel, FadeChannel *out) const
{
    // DMX Total Control: the fine channel of a pair is kept with its coarse
    // one, as channelFaderLocked keeps it
    QReadLocker l(&m_channelsLock);
    quint32 hash = channelHash(fixtureID, channel);
    if (m_channels.contains(hash) == false && m_handleSecondary)
    {
        const QSharedPointer<FadeChannel> channelTemplate = m_channelCache.value(hash);
        if (channelTemplate.isNull() == false && channelTemplate->primaryChannel() != QLCChannel::invalid())
            hash = channelHash(fixtureID, channelTemplate->primaryChannel());
    }

    auto it = m_channels.constFind(hash);
    if (it == m_channels.cend())
        return false;
    *out = it.value();
    return true;
}

void GenericFader::refreshFixtures(const Doc *doc, quint32 universeID)
{
    // DMX Total Control: new channels detect the fixtures again
    QWriteLocker l(&m_channelsLock);
    m_channelCache.clear();

    QMutableHashIterator<quint32, FadeChannel> it(m_channels);
    while (it.hasNext())
    {
        FadeChannel &fc = it.next().value();
        // (a channel by its absolute address has no fixture to follow)
        if (fc.fixture() == Fixture::invalidId())
            continue;
        if (fc.refreshFixture(doc, universeID) == false)
            it.remove();
    }
}

bool GenericFader::deleteRequested()
{
    return m_deleteRequest;
}

void GenericFader::requestDelete()
{
    m_deleteRequest = true;
}

FadeChannel *GenericFader::getChannelFader(const Doc *doc, Universe *universe, quint32 fixtureID, quint32 channel)
{
    QWriteLocker l(&m_channelsLock);
    return &channelFaderLocked(doc, universe, fixtureID, channel);
}

QHash<quint32, FadeChannel> GenericFader::channels() const
{
    QReadLocker l(&m_channelsLock);
    return m_channels;
}

int GenericFader::channelsCount() const
{
    QReadLocker l(&m_channelsLock);
    return m_channels.count();
}

FadeChannel &GenericFader::channelFaderLocked(const Doc *doc, Universe *universe, quint32 fixtureID, quint32 channel)
{
    const quint32 requestedHash = channelHash(fixtureID, channel);

    QSharedPointer<FadeChannel> channelTemplate = m_channelCache.value(requestedHash);
    if (channelTemplate.isNull())
    {
        channelTemplate = QSharedPointer<FadeChannel>(new FadeChannel(doc, fixtureID, channel));
        m_channelCache.insert(requestedHash, channelTemplate);
    }

    const quint32 primary = channelTemplate->primaryChannel();
    const quint32 hash = (handleSecondary() && primary != QLCChannel::invalid())
                         ? channelHash(channelTemplate->fixture(), primary)
                         : channelHash(channelTemplate->fixture(), channelTemplate->channel());

    QHash<quint32, FadeChannel>::iterator channelIterator = m_channels.find(hash);
    if (channelIterator != m_channels.end())
    {
        FadeChannel &fc = channelIterator.value();
        if (handleSecondary() &&
            fc.channelCount() == 1 &&
            primary != QLCChannel::invalid() &&
            channel != primary)
        {
            fc.addChannel(channel);
            if (universe)
                fc.setCurrent(universe->preGMValue(fc.address() + 1), 1);
        }
        return fc;
    }

    FadeChannel fc;
    if (handleSecondary() && primary != QLCChannel::invalid() && channel != primary)
    {
        const quint32 primaryRequestHash = channelHash(fixtureID, primary);
        QSharedPointer<FadeChannel> primaryTemplate = m_channelCache.value(primaryRequestHash);
        if (primaryTemplate.isNull())
        {
            primaryTemplate = QSharedPointer<FadeChannel>(new FadeChannel(doc, fixtureID, primary));
            m_channelCache.insert(primaryRequestHash, primaryTemplate);
        }

        fc = *primaryTemplate;
        fc.addChannel(channel);

        if (universe)
        {
            fc.setCurrent(universe->preGMValue(fc.address()), 0);
            fc.setCurrent(universe->preGMValue(fc.address() + 1), 1);
        }
    }
    else
    {
        fc = *channelTemplate;
        if (universe)
            fc.setCurrent(universe->preGMValue(fc.address()));
    }

    channelIterator = m_channels.insert(hash, fc);
    return channelIterator.value();
}

void GenericFader::forEachChannel(const std::function<void(FadeChannel &)> &updater)
{
    // DMX Total Control: as updateChannel, on all of them
    QWriteLocker l(&m_channelsLock);
    for (FadeChannel &fc : m_channels)
        updater(fc);
}

quint32 GenericFader::valueBelow(const Universe *universe, quint32 address, int count)
{
    // DMX Total Control
    quint32 value = 0;
    for (int i = 0; i < count; i++)
        value = (value << 8) + universe->valueBelow(int(address) + i);
    return value;
}

quint32 GenericFader::mixBelow(const FadeChannel &fc, quint32 below, qreal level)
{
    // DMX Total Control: a channel that can't fade jumps as soon as the
    // level is up
    const qint64 own = qint64(fc.current());
    if (fc.canFade() == false)
        return level > 0 ? quint32(own) : below;
    const qint64 maxValue = (qint64(1) << (8 * fc.channelCount())) - 1;
    const qint64 out = qint64(below) + qRound64(qreal(own - qint64(below)) * level);
    return quint32(qBound(qint64(0), out, maxValue));
}

uint GenericFader::stepOf(int flags, uint elapsedMs) const
{
    // DMX Total Control: a hand crossfade sets the time of a Manual channel:
    // a step of 0 ms only takes a new target (one going to what's below).
    // Fading out, it runs on the clock
    return (flags & FadeChannel::Manual) && m_fadeOut == false ? 0 : elapsedMs;
}

bool GenericFader::leaves(int flags, const FadeChannel &fc) const
{
    // DMX Total Control: a Manual channel stays at its end: the hand can
    // bring it back
    if ((flags & FadeChannel::Manual) && m_fadeOut == false)
        return false;
    return (m_fadeOut || (flags & FadeChannel::AutoRemove)) && fc.isReady();
}

void GenericFader::write(Universe *universe, uint elapsedMs)
{
    if (m_monitoring)
        emit preWriteData(universe->id(), universe->preGMValues());

    qreal compIntensity = intensity() * parentIntensity();

    //qDebug() << "[GenericFader] writing channels: " << this << m_channels.count();

    // iterate through all the channels handled by this fader
    QWriteLocker l(&m_channelsLock);
    QMutableHashIterator <quint32,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());
        int flags = fc.flags();
        quint32 address = fc.addressInUniverse();
        int channelCount = fc.channelCount();

        if (address == QLCChannel::invalid())
        {
            qWarning() << "Invalid channel found";
            continue;
        }

        // DMX Total Control: fading out, the target is what the faders below
        // write, every frame: a real crossfade to what's below, even when it
        // moves, and with a higher priority it doesn't drop to zero first.
        // With nothing below, an LTP channel goes back to its rest value
        if (flags & FadeChannel::SetTarget)
        {
            for (int i = 0; i < channelCount; i++)
                fc.setTarget(universe->valueBelow(address + i), i);
        }

        // DMX Total Control: an Override cuelist. The level mixes its value
        // with what the faders below send (both bytes of a pair as one): at
        // 0 what's below, at full its value, which replaces it (no HTP)
        if (flags & FadeChannel::MixBelow)
        {
            if (m_paused == false)
                fc.nextStep(stepOf(flags, elapsedMs));
            const quint32 out = mixBelow(fc, valueBelow(universe, address, channelCount), compIntensity);
            fc.setModulatedOutput(out);
            for (int i = 0; i < channelCount; i++)
                universe->write(int(address) + i, uchar(out >> (8 * (channelCount - 1 - i))), true);
            if (leaves(flags, fc))
                it.remove();
            continue;
        }

        // DMX Total Control: an FX layer. Its base (fading in) or, without a
        // base, what the faders below send now, plus the offset of the wave
        if (flags & FadeChannel::Modulated)
        {
            // with a fine channel the values are 16 bit (in this universe)
            const quint32 fine = fc.modulationFine();
            const bool wide = fine != QLCChannel::invalid() && fine < UNIVERSE_SIZE;
            const bool below = fc.modulatesBelow();
            const qint64 maxValue = wide ? 0xFFFF : 0xFF;
            qint64 base = 0;
            qint64 offset = fc.modulationOffset();
            if (below)
            {
                base = universe->valueBelow(address);
                if (wide)
                    base = (base << 8) + universe->valueBelow(fine);
            }
            else
            {
                if (m_paused == false)
                    fc.nextStep(elapsedMs);
                base = (flags & FadeChannel::Intensity) ? fc.current(compIntensity) : fc.current();
            }
            // the fader of an intensity scales its effect too
            if (flags & FadeChannel::Intensity)
                offset = qint64(qreal(offset) * compIntensity);

            quint32 value = quint32(qBound(qint64(0), base + offset, maxValue));
            // DMX Total Control: a virtual dimmer dims the value, as a dimmer
            // would. On what's below, the fader of an intensity scales its
            // effect too: at 0 what's below stays. (1: the exact value)
            qreal scale = fc.modulationScale();
            if (below && (flags & FadeChannel::Intensity))
                scale = 1.0 + (scale - 1.0) * compIntensity;
            if (scale != 1.0)
                value = quint32(qBound(qint64(0), qRound64(qreal(value) * scale), maxValue));
            fc.setModulatedOutput(value);
            const uchar coarse = uchar(wide ? value >> 8 : value);
            if (below)
            {
                // it already holds what's below: it replaces it
                universe->write(address, coarse, true);
                if (wide)
                    universe->write(fine, uchar(value & 0xFF), true);
            }
            else
            {
                universe->writeBlended(address, coarse, 1, m_blendMode);
                if (wide)
                    universe->writeBlended(fine, value & 0xFF, 1, m_blendMode);
            }
            continue;
        }

        // Calculate the next step
        // DMX Total Control: a Manual channel at the time its crossfade set
        if (m_paused == false)
            fc.nextStep(stepOf(flags, elapsedMs));

        quint32 value = fc.current();

        // Apply intensity to channels that can fade
        if (fc.canFade())
        {
            if ((flags & FadeChannel::CrossFade) && fc.fadeTime() == 0)
            {
                // morph start <-> target depending on intensities
                bool rampUp = fc.target() > fc.start() ? true : false;
                value = rampUp ? fc.target() - fc.start() : fc.start() - fc.target();
                value = qreal(value) * intensity();
                value = qreal(rampUp ? fc.start() + value : fc.start() - value) * parentIntensity();
            }
            else if ((flags & FadeChannel::Intensity) && (flags & FadeChannel::SetTarget) == 0)
            {
                // (fading out to what's below, the values are already scaled)
                if (flags & FadeChannel::Relative)
                {
                    // DMX Total Control: a relative value is an offset from
                    // its zero: the intensity scales the offset only
                    const qint64 zero = channelCount == 1 ? RELATIVE_ZERO_8BIT : RELATIVE_ZERO_16BIT;
                    const qint64 offset = qint64(fc.current()) - zero;
                    value = quint32(zero + qint64(std::round(qreal(offset) * compIntensity)));
                }
                else
                {
                    value = fc.current(compIntensity);
                }
            }
        }

        //qDebug() << "[GenericFader] >>> uni:" << universe->id() << ", address:" << address << ", value:" << value << "int:" << compIntensity;
        if (flags & FadeChannel::Override)
        {
            universe->write(address, value, true);
            continue;
        }
        else if (flags & FadeChannel::Relative)
        {
            universe->writeRelative(address, value, channelCount);
        }
        else if (flags & FadeChannel::Flashing)
        {
            for (int i = 0; i < channelCount; i++)
                universe->write(address + i, ((uchar *)&value)[channelCount - 1 - i],
                                flags & FadeChannel::ForceLTP ? true : false);
            continue;
        }
        else
        {
            // treat value as a whole, so do this just once per FadeChannel
            universe->writeBlended(address, value, channelCount, m_blendMode);
        }

        // DMX Total Control: fading out, a channel is done when its fade is
        // over. While the fader runs its zeros stay: with a higher priority
        // they hide the HTP values of the lower faders. A single channel
        // flagged AutoRemove (a cuelist releasing it) leaves too, once it
        // has reached what's below
        if (leaves(flags, fc))
            it.remove();
    }

    // self-request deletion when fadeout is complete
    if (m_fadeOut && m_channels.isEmpty())
    {
        m_fadeOut = false;
        requestDelete();
    }
}

qreal GenericFader::intensity() const
{
    return m_intensity;
}

void GenericFader::adjustIntensity(qreal fraction)
{
    //qDebug() << name() << "I FADER intensity" << fraction << ", PARENT:" << m_parentIntensity;
    m_intensity = fraction;
}

qreal GenericFader::parentIntensity() const
{
    return m_parentIntensity;
}

void GenericFader::setParentIntensity(qreal fraction)
{
    //qDebug() << name() << "P FADER intensity" << m_intensity << ", PARENT:" << fraction;
    m_parentIntensity = fraction;
}

bool GenericFader::isPaused() const
{
    return m_paused;
}

void GenericFader::setPaused(bool paused)
{
    m_paused = paused;
}

bool GenericFader::isEnabled() const
{
    return m_enabled;
}

void GenericFader::setEnabled(bool enable)
{
    m_enabled = enable;
}

bool GenericFader::isFadingOut() const
{
    return m_fadeOut;
}

void GenericFader::setFadeOut(bool enable, uint fadeTime, const QHash<quint32, uint> &delays)
{
    m_fadeOut = enable;

    if (fadeTime == 0 && delays.isEmpty())
        return;

    QWriteLocker l(&m_channelsLock);
    QMutableHashIterator <quint32,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());

        // DMX Total Control: every channel fades to what the faders below
        // write (the write method sets the target), intensities too: from
        // the value it sends now, master included
        if (fc.flags() & FadeChannel::Modulated)
        {
            // an FX: from the last value it sent, wave and master included
            // (its fine channel stays, then rests)
            quint32 output = fc.modulatedOutput();
            if (fc.modulationFine() != QLCChannel::invalid())
                output >>= 8;
            fc.removeFlag(FadeChannel::Modulated);
            fc.setModulation(0, false);
            fc.setCurrent(output);
            fc.setStart(output);
            fc.addFlag(FadeChannel::SetTarget);
        }
        else if ((fc.flags() & FadeChannel::Relative) == 0)
        {
            // DMX Total Control: a channel already fading to what's below
            // (SetTarget) holds output values: scaling it again would dip.
            // One mixed with what's below keeps its own value: it goes on
            // mixing it, as it goes to what's below
            if (fc.canFade() && (fc.flags() & FadeChannel::Intensity) &&
                (fc.flags() & FadeChannel::CrossFade) == 0 &&
                (fc.flags() & (FadeChannel::SetTarget | FadeChannel::MixBelow)) == 0)
                fc.setStart(fc.current(intensity() * parentIntensity()));
            else
                fc.setStart(fc.current());
            fc.addFlag(FadeChannel::SetTarget);
        }
        else
        {
            fc.setStart(fc.current());
        }
        fc.setTarget(0);
        fc.setElapsed(0);
        fc.setReady(false);
        fc.setFadeTime(fc.canFade() ? fadeTime : 0);
        fc.setDelay(delays.value(fc.fixture(), 0));
        // DMX Total Control: a release is linear (not the curve of a cue)
        fc.setCurve(FadeChannel::CurveLinear);
        // if flashing, remove the flag and treat
        // it like a regular fade out to target
        fc.removeFlag(FadeChannel::Flashing);
    }
}

void GenericFader::setBlendMode(Universe::BlendMode mode)
{
    m_blendMode = mode;
}

void GenericFader::setMonitoring(bool enable)
{
    m_monitoring = enable;
}

void GenericFader::resetCrossfade()
{
    qDebug() << name() << "resetting crossfade channels";
    QWriteLocker l(&m_channelsLock);
    QMutableHashIterator <quint32,FadeChannel> it(m_channels);
    while (it.hasNext() == true)
    {
        FadeChannel& fc(it.next().value());
        fc.removeFlag(FadeChannel::CrossFade);
    }
}

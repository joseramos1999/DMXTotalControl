/*
  DMX Total Control
  timecodereader.cpp

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

#include <QLocale>
#include <QRegularExpression>
#include <QStringList>

#include "timecodereader.h"
#include "mastertimer.h"
#include "doc.h"

namespace
{

/** 29.97 drop-frame */
const int kFps2997 = 29;
/** Until a timecode comes */
const int kDefaultFps = 25;

bool knownFps(int fps)
{
    return fps == 24 || fps == 25 || fps == kFps2997 || fps == 30;
}

/** The frames of a second in the labels (30 at 29.97 drop-frame) */
int labelFps(int fps)
{
    return fps == kFps2997 ? 30 : fps;
}

}

TimecodeReader::TimecodeReader(Doc *doc, QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_fps(0)
{
    // in the thread the inputs come in: the time goes on to the cuelists
    // at once (setTimecode is safe in any thread)
    connect(m_doc->inputOutputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar,QString)),
            this, SLOT(slotInputValueChanged(quint32,quint32,uchar,QString)), Qt::DirectConnection);
}

void TimecodeReader::slotInputValueChanged(quint32, quint32 channel, uchar, const QString &key)
{
    // (the DMX that comes in on that channel has no key)
    if (channel != kChannel || key.startsWith(QLatin1String("mtc:")) == false)
        return;

    const QStringList parts = key.split(QLatin1Char(':'));
    if (parts.count() != 3)
        return;
    bool msOk = false;
    bool fpsOk = false;
    const qint64 ms = parts.at(1).toLongLong(&msOk);
    const int fps = parts.at(2).toInt(&fpsOk);
    if (msOk == false || fpsOk == false || ms < 0 || knownFps(fps) == false)
        return;

    m_fps = fps;
    m_doc->masterTimer()->setTimecode(ms, fps);
}

QString TimecodeReader::status() const
{
    int fps = 0;
    const qint64 ms = m_doc->masterTimer()->timecode(&fps);
    if (ms < 0)
        return tr("No MIDI timecode");
    const QString rate = fps == kFps2997 ? QLocale().toString(29.97, 'f', 2) : QString::number(fps);
    return tr("MTC %1 · %2 fps").arg(timecodeText(ms, fps), rate);
}

bool TimecodeReader::receiving() const
{
    return m_doc->masterTimer()->timecode() >= 0;
}

QString TimecodeReader::offsetText(int ms) const
{
    return timecodeText(ms, frameRate());
}

int TimecodeReader::parseOffset(const QString &text) const
{
    static const QRegularExpression separators(QStringLiteral("[:;.]"));
    const QStringList parts = text.trimmed().split(separators);
    if (parts.isEmpty() || parts.count() > 4)
        return -1;

    // hours first: "1" is an hour, "01:30" an hour and a half
    int fields[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < parts.count(); i++)
    {
        bool ok = false;
        fields[i] = parts.at(i).trimmed().toInt(&ok);
        if (ok == false)
            return -1;
    }
    const qint64 ms = timecodeMs(fields[0], fields[1], fields[2], fields[3], frameRate());
    return ms < 0 ? -1 : int(ms);
}

QString TimecodeReader::timecodeText(qint64 ms, int fps)
{
    if (knownFps(fps) == false)
        fps = kDefaultFps;
    ms = qMax(qint64(0), ms);

    // the frame the time is in (half a ms more: the ms of a frame are rounded)
    qint64 frames = fps == kFps2997 ? (2 * ms + 1) * 30 / 2002 : (2 * ms + 1) * fps / 2000;
    // drop-frame: the labels it skips (00 and 01 of each minute, but every
    // tenth one) count again
    if (fps == kFps2997)
    {
        const qint64 tens = frames / 17982;
        const qint64 rest = frames % 17982;
        frames += 18 * tens + (rest > 2 ? 2 * ((rest - 2) / 1798) : 0);
    }

    const int perSecond = labelFps(fps);
    const int ff = int(frames % perSecond);
    frames /= perSecond;
    const int ss = int(frames % 60);
    frames /= 60;
    const int mm = int(frames % 60);
    const int hh = int(frames / 60);
    return QString::asprintf("%02d:%02d:%02d:%02d", hh, mm, ss, ff);
}

qint64 TimecodeReader::timecodeMs(int hours, int minutes, int seconds, int frames, int fps)
{
    if (knownFps(fps) == false)
        return -1;
    const int perSecond = labelFps(fps);
    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59 ||
        frames < 0 || frames >= perSecond)
        return -1;

    // as the MIDI plugin counts them (MtcDecoder): in real time
    qint64 count = (qint64(hours * 60 + minutes) * 60 + seconds) * perSecond + frames;
    if (fps == kFps2997)
    {
        const int total = hours * 60 + minutes;
        count -= 2 * (total - total / 10);
        return qRound64(qreal(count) * 1001.0 / 30.0);
    }
    return qRound64(qreal(count) * 1000.0 / qreal(fps));
}

int TimecodeReader::frameRate() const
{
    const int fps = m_fps.load();
    return fps > 0 ? fps : kDefaultFps;
}

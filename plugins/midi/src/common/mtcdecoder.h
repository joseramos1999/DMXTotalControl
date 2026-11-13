/*
  DMX Total Control
  mtcdecoder.h

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

#ifndef MTCDECODER_H
#define MTCDECODER_H

#include <QByteArray>
#include <QtGlobal>

/**
 * DMX Total Control: reads a MIDI timecode (MTC).
 *
 * A time comes in 8 quarter frames (F1 xx), pieces 0 to 7, and takes two
 * frames: the time is the one of its first piece. Once a whole time has
 * come in, each quarter frame gives the time of now: that time and a quarter
 * of a frame for each piece since its first one. A source that runs
 * backwards sends the pieces from 7 down to 0, and its time goes down.
 * A gap in the pieces (a lost message) waits for the next whole time.
 *
 * A full frame (the SysEx F0 7F <device> 01 01 hh mm ss ff F7, which a
 * source sends when it locates) gives its time at once.
 *
 * The times are in ms from 00:00:00:00, in real time: at 29.97 drop-frame
 * the frames 00 and 01 of each minute, except every tenth one, don't exist.
 */
class MtcDecoder
{
public:
    /** The frame rates; 29 is 29.97 drop-frame */
    enum { Fps24 = 24, Fps25 = 25, Fps2997 = 29, Fps30 = 30 };

    MtcDecoder();

    /** Forgets the pieces and the time (a device that opens again) */
    void reset();

    /** The data byte of a quarter frame. @return true when it gives a time */
    bool feedQuarterFrame(uchar data);

    /** A SysEx message, from F0 to F7. @return true when it is a full frame */
    bool feedFullFrame(const QByteArray &message);

    /** The time the last quarter frame or full frame gave, and its frame rate */
    qint64 ms() const { return m_ms; }
    int fps() const { return m_fps; }

    /** The frames from 00:00:00:00 to $hours:$minutes:$seconds:$frames at
     *  $fps; -1 when it is not a time at that rate */
    static qint64 labelToFrames(int hours, int minutes, int seconds, int frames, int fps);
    /** The ms of $quarters quarter frames at $fps */
    static qint64 quartersToMs(qint64 quarters, int fps);

private:
    /** The whole time of the 8 pieces, as quarter frames; false when it is not one */
    bool assemble();

private:
    uchar m_pieces[8];
    /** the last piece that came (-1: none), the way they go (1: up, -1:
     *  down, 0: not known) and how many came in a row that way */
    int m_last;
    int m_way;
    int m_run;
    /** a whole time came in, and the pieces went on in a row since then */
    bool m_valid;
    /** that time (quarter frames) and the quarter frames since its first piece */
    qint64 m_base;
    int m_quarters;

    qint64 m_ms;
    int m_fps;
};

#endif

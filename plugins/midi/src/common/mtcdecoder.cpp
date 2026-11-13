/*
  DMX Total Control
  mtcdecoder.cpp

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

#include "mtcdecoder.h"

namespace
{

/** The frame rate of the 2 bits of rate in the hours */
int rateToFps(int rate)
{
    static const int fps[4] = { MtcDecoder::Fps24, MtcDecoder::Fps25, MtcDecoder::Fps2997, MtcDecoder::Fps30 };
    return fps[rate & 0x03];
}

}

MtcDecoder::MtcDecoder()
{
    reset();
}

void MtcDecoder::reset()
{
    for (uchar &piece : m_pieces)
        piece = 0;
    m_last = -1;
    m_way = 0;
    m_run = 0;
    m_valid = false;
    m_base = 0;
    m_quarters = 0;
    m_ms = 0;
    m_fps = Fps25;
}

bool MtcDecoder::feedQuarterFrame(uchar data)
{
    const int piece = (data >> 4) & 0x07;
    m_pieces[piece] = data & 0x0F;

    // the way the pieces go: the one after the last one (up) or the one
    // before it (down); anything else is a gap
    int way = 0;
    if (m_last >= 0)
        way = piece == (m_last + 1) % 8 ? 1 : (piece == (m_last + 7) % 8 ? -1 : 0);
    m_last = piece;
    if (way == 0)
    {
        // the first piece, or a gap: a new time starts
        m_way = 0;
        m_run = 1;
        m_valid = false;
        return false;
    }
    if (way != m_way)
    {
        // the second piece (both are in a row that way), or a source that
        // turned (the last piece was of a time the other way: this one starts)
        m_run = m_way == 0 ? 2 : 1;
        m_way = way;
        m_valid = false;
    }
    else if (m_run < 8)
    {
        m_run++;
    }

    // the last piece of a time (7 going up, 0 going down) after 8 in a row:
    // a whole time, and its first piece came 7 quarter frames ago
    if (piece == (m_way > 0 ? 7 : 0) && m_run >= 8)
    {
        m_valid = assemble();
        if (m_valid == false)
            return false;
        m_quarters = 7;
    }
    else if (m_valid)
    {
        m_quarters++;
    }
    else
    {
        return false;
    }

    m_ms = quartersToMs(qMax(qint64(0), m_base + m_way * m_quarters), m_fps);
    return true;
}

bool MtcDecoder::feedFullFrame(const QByteArray &message)
{
    // F0 7F <device> 01 01 hh mm ss ff F7 (any device)
    if (message.size() < 10 || uchar(message.at(0)) != 0xF0 || uchar(message.at(1)) != 0x7F ||
        uchar(message.at(3)) != 0x01 || uchar(message.at(4)) != 0x01 || uchar(message.at(9)) != 0xF7)
        return false;

    const uchar hours = uchar(message.at(5));
    const int fps = rateToFps(hours >> 5);
    const qint64 frames = labelToFrames(hours & 0x1F, message.at(6) & 0x3F, message.at(7) & 0x3F,
                                        message.at(8) & 0x1F, fps);
    if (frames < 0)
        return false;

    // a locate: the quarter frames that come next start a new time
    m_last = -1;
    m_way = 0;
    m_run = 0;
    m_valid = false;
    m_fps = fps;
    m_base = frames * 4;
    m_ms = quartersToMs(m_base, m_fps);
    return true;
}

qint64 MtcDecoder::labelToFrames(int hours, int minutes, int seconds, int frames, int fps)
{
    if (fps != Fps24 && fps != Fps25 && fps != Fps2997 && fps != Fps30)
        return -1;
    const int nominal = fps == Fps2997 ? 30 : fps;
    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59 || seconds < 0 || seconds > 59 ||
        frames < 0 || frames >= nominal)
        return -1;

    qint64 count = (qint64(hours * 60 + minutes) * 60 + seconds) * nominal + frames;
    // drop-frame: 2 labels less in each minute, but every tenth one
    if (fps == Fps2997)
    {
        const int total = hours * 60 + minutes;
        count -= 2 * (total - total / 10);
    }
    return count;
}

qint64 MtcDecoder::quartersToMs(qint64 quarters, int fps)
{
    // (29.97: a frame lasts 1001/30 ms)
    if (fps == Fps2997)
        return qRound64(qreal(quarters) * 1001.0 / 120.0);
    return qRound64(qreal(quarters) * 1000.0 / qreal(fps * 4));
}

bool MtcDecoder::assemble()
{
    const int frames = m_pieces[0] | ((m_pieces[1] & 0x01) << 4);
    const int seconds = m_pieces[2] | ((m_pieces[3] & 0x03) << 4);
    const int minutes = m_pieces[4] | ((m_pieces[5] & 0x03) << 4);
    const int hours = m_pieces[6] | ((m_pieces[7] & 0x01) << 4);
    const int fps = rateToFps(m_pieces[7] >> 1);
    const qint64 count = labelToFrames(hours, minutes, seconds, frames, fps);
    if (count < 0)
        return false;
    m_fps = fps;
    m_base = count * 4;
    return true;
}

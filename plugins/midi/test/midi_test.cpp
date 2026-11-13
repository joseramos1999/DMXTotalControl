/*
  Q Light Controller Plus
  midi_test.cpp

  Copyright (c) Jano Svitok

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

#include <QTest>

#define private public
#include "midi_test.h"
#include "midiprotocol.h"
#include "mtcdecoder.h"

#undef private

/****************************************************************************
 * MIDI tests
 ****************************************************************************/

void Midi_Test::midiToInput()
{
    quint32 channel = 0;
    uchar value = 0;

    uchar midiChannel = 7;
    uchar cmd = MIDI_NOTE_ON | midiChannel;
    uchar data1 = 10;
    uchar data2 = 127;

    QLCMIDIProtocol::midiToInput(cmd, data1, data2, midiChannel, &channel, &value);

    QCOMPARE(channel, 138U);
    QCOMPARE(value, uchar(255U));
}

/****************************************************************************
 * DMX Total Control: MIDI timecode
 ****************************************************************************/

namespace
{

/** The 8 quarter frames of hh:mm:ss:ff at $rate (0: 24, 1: 25, 2: 29.97
 *  drop-frame, 3: 30), pieces 0 to 7 */
QList<uchar> quarterFrames(int hh, int mm, int ss, int ff, int rate)
{
    return { uchar(0x00 | (ff & 0x0F)), uchar(0x10 | (ff >> 4)),
             uchar(0x20 | (ss & 0x0F)), uchar(0x30 | (ss >> 4)),
             uchar(0x40 | (mm & 0x0F)), uchar(0x50 | (mm >> 4)),
             uchar(0x60 | (hh & 0x0F)), uchar(0x70 | (rate << 1) | (hh >> 4)) };
}

/** A full frame: F0 7F 7F 01 01 hh mm ss ff F7 */
QByteArray fullFrame(int hh, int mm, int ss, int ff, int rate)
{
    const uchar bytes[10] = { 0xF0, 0x7F, 0x7F, 0x01, 0x01, uchar((rate << 5) | hh),
                              uchar(mm), uchar(ss), uchar(ff), 0xF7 };
    return QByteArray(reinterpret_cast<const char *>(bytes), 10);
}

}

void Midi_Test::mtcDecoder()
{
    // 01:02:03:04 at 25 fps: no time until the 8 pieces came in; then the
    // time of now, a quarter of a frame (10 ms) for each piece since the
    // first one
    MtcDecoder mtc;
    const QList<uchar> first = quarterFrames(1, 2, 3, 4, 1);
    for (int i = 0; i < 7; i++)
        QVERIFY(mtc.feedQuarterFrame(first.at(i)) == false);
    QVERIFY(mtc.feedQuarterFrame(first.at(7)));
    const qint64 time = ((1 * 60 + 2) * 60 + 3) * 1000 + 4 * 40;
    QCOMPARE(mtc.ms(), time + 70);
    QCOMPARE(mtc.fps(), 25);

    // the next time, 2 frames later: each piece goes on 10 ms, in a row
    const QList<uchar> next = quarterFrames(1, 2, 3, 6, 1);
    for (int i = 0; i < 8; i++)
    {
        QVERIFY(mtc.feedQuarterFrame(next.at(i)));
        QCOMPARE(mtc.ms(), time + 80 + 10 * i);
    }

    // a gap (a piece lost): nothing until a whole time came in again
    QVERIFY(mtc.feedQuarterFrame(next.at(1)) == false);
    QVERIFY(mtc.feedQuarterFrame(next.at(2)) == false);
    const QList<uchar> later = quarterFrames(1, 2, 3, 10, 1);
    for (int i = 0; i < 7; i++)
        QVERIFY(mtc.feedQuarterFrame(later.at(i)) == false);
    QVERIFY(mtc.feedQuarterFrame(later.at(7)));
    QCOMPARE(mtc.ms(), time + 6 * 40 + 70);

    // an incomplete time (the source stops after 5 pieces) gives none; a
    // new one from its first piece does
    MtcDecoder fresh;
    for (int i = 0; i < 5; i++)
        QVERIFY(fresh.feedQuarterFrame(first.at(i)) == false);
    for (int i = 0; i < 7; i++)
        QVERIFY(fresh.feedQuarterFrame(later.at(i)) == false);
    QVERIFY(fresh.feedQuarterFrame(later.at(7)));
    QCOMPARE(fresh.ms(), time + 6 * 40 + 70);

    // a time that is not one (minute 61) gives nothing
    MtcDecoder wrong;
    for (uchar piece : quarterFrames(1, 61, 3, 4, 1))
        QVERIFY(wrong.feedQuarterFrame(piece) == false);

    // 24 and 30 fps
    MtcDecoder film;
    for (uchar piece : quarterFrames(0, 0, 1, 12, 0))
        film.feedQuarterFrame(piece);
    QCOMPARE(film.fps(), 24);
    QCOMPARE(film.ms(), qRound64((24 + 12 + 7 / 4.0) * 1000.0 / 24));
    MtcDecoder video;
    for (uchar piece : quarterFrames(0, 0, 1, 15, 3))
        video.feedQuarterFrame(piece);
    QCOMPARE(video.fps(), 30);
    QCOMPARE(video.ms(), qRound64((30 + 15 + 7 / 4.0) * 1000.0 / 30));

    // reset: it starts again
    mtc.reset();
    QVERIFY(mtc.feedQuarterFrame(next.at(7)) == false);
}

void Midi_Test::mtcBackwards()
{
    // a source that runs backwards sends the pieces from 7 down to 0: the
    // time of its first piece (7), then 10 ms less for each piece
    MtcDecoder mtc;
    const QList<uchar> first = quarterFrames(1, 2, 3, 4, 1);
    for (int i = 7; i > 0; i--)
        QVERIFY(mtc.feedQuarterFrame(first.at(i)) == false);
    QVERIFY(mtc.feedQuarterFrame(first.at(0)));
    const qint64 time = ((1 * 60 + 2) * 60 + 3) * 1000 + 4 * 40;
    QCOMPARE(mtc.ms(), time - 70);
    QCOMPARE(mtc.fps(), 25);

    const QList<uchar> next = quarterFrames(1, 2, 3, 2, 1);
    for (int i = 7; i >= 0; i--)
    {
        QVERIFY(mtc.feedQuarterFrame(next.at(i)));
        QCOMPARE(mtc.ms(), time - 80 - 10 * (7 - i));
    }

    // it turns: nothing until a whole time came in forwards
    const QList<uchar> forwards = quarterFrames(1, 2, 3, 2, 1);
    for (int i = 1; i < 8; i++)
        QVERIFY(mtc.feedQuarterFrame(forwards.at(i)) == false);
    const QList<uchar> after = quarterFrames(1, 2, 3, 4, 1);
    for (int i = 0; i < 7; i++)
        QVERIFY(mtc.feedQuarterFrame(after.at(i)) == false);
    QVERIFY(mtc.feedQuarterFrame(after.at(7)));
    QCOMPARE(mtc.ms(), time + 70);
}

void Midi_Test::mtcDropFrame()
{
    // 29.97 drop-frame: the labels 00 and 01 of each minute but every tenth
    // one don't exist, and the times are in real time (a frame lasts 1001/30
    // ms): 01:00:00;00 is 107892 frames, an hour less 3.6 ms
    QCOMPARE(MtcDecoder::labelToFrames(1, 0, 0, 0, MtcDecoder::Fps2997), qint64(107892));
    QCOMPARE(MtcDecoder::quartersToMs(107892 * 4, MtcDecoder::Fps2997), qint64(3599996));
    QCOMPARE(MtcDecoder::labelToFrames(0, 10, 0, 0, MtcDecoder::Fps2997), qint64(17982));
    // 00:01:00;02 is the frame after 00:00:59;29
    QCOMPARE(MtcDecoder::labelToFrames(0, 0, 59, 29, MtcDecoder::Fps2997), qint64(1799));
    QCOMPARE(MtcDecoder::labelToFrames(0, 1, 0, 2, MtcDecoder::Fps2997), qint64(1800));
    QCOMPARE(MtcDecoder::labelToFrames(0, 0, 0, 30, MtcDecoder::Fps2997), qint64(-1));
    QCOMPARE(MtcDecoder::labelToFrames(0, 0, 0, 25, MtcDecoder::Fps25), qint64(-1));
    QCOMPARE(MtcDecoder::labelToFrames(24, 0, 0, 0, MtcDecoder::Fps25), qint64(-1));

    MtcDecoder mtc;
    for (uchar piece : quarterFrames(0, 1, 0, 2, 2))
        mtc.feedQuarterFrame(piece);
    QCOMPARE(mtc.fps(), int(MtcDecoder::Fps2997));
    QCOMPARE(mtc.ms(), qRound64((1800 * 4 + 7) * 1001.0 / 120.0));
    // the next piece: a quarter of a frame later
    QVERIFY(mtc.feedQuarterFrame(quarterFrames(0, 1, 0, 4, 2).at(0)));
    QCOMPARE(mtc.ms(), qRound64((1800 * 4 + 8) * 1001.0 / 120.0));
}

void Midi_Test::mtcFullFrame()
{
    // a full frame (a locate) gives its time at once, at its rate; any device
    MtcDecoder mtc;
    QVERIFY(mtc.feedFullFrame(fullFrame(1, 2, 3, 4, 1)));
    QCOMPARE(mtc.ms(), qint64(((1 * 60 + 2) * 60 + 3) * 1000 + 4 * 40));
    QCOMPARE(mtc.fps(), 25);
    QByteArray device = fullFrame(1, 0, 0, 0, 2);
    device[2] = 0x05;
    QVERIFY(mtc.feedFullFrame(device));
    QCOMPARE(mtc.ms(), qint64(3599996));
    QCOMPARE(mtc.fps(), int(MtcDecoder::Fps2997));

    // other SysEx messages, a short one, a time that is not one: nothing
    QByteArray other = fullFrame(1, 2, 3, 4, 1);
    other[3] = 0x06;
    QVERIFY(mtc.feedFullFrame(other) == false);
    QVERIFY(mtc.feedFullFrame(fullFrame(1, 2, 3, 4, 1).left(9)) == false);
    QVERIFY(mtc.feedFullFrame(fullFrame(1, 2, 3, 30, 1)) == false);
    QVERIFY(mtc.feedFullFrame(QByteArray::fromHex("F07E7F0601F7")) == false);
    QCOMPARE(mtc.ms(), qint64(3599996));

    // the quarter frames after a locate start a new time: none from the
    // pieces that came before it
    const QList<uchar> pieces = quarterFrames(1, 2, 3, 4, 1);
    for (int i = 0; i < 4; i++)
        mtc.feedQuarterFrame(pieces.at(i));
    QVERIFY(mtc.feedFullFrame(fullFrame(0, 0, 10, 0, 1)));
    for (int i = 4; i < 8; i++)
        QVERIFY(mtc.feedQuarterFrame(pieces.at(i)) == false);
    QCOMPARE(mtc.ms(), qint64(10000));
}

QTEST_MAIN(Midi_Test)

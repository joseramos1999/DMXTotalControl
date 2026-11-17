/*
  DMX Total Control
  fixturelook.h

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

#ifndef FIXTURELOOK_H
#define FIXTURELOOK_H

#include <QStringList>
#include <QList>
#include <QMap>

class QLCChannel;
class Fixture;

/**
 * How a fixture looks, in the DMX values of its channels: its open white
 * (Locate, Highlight), the light of the show stop, its shutters open, the
 * channels that move it. One copy for the programmer, the identify of the
 * patch and the show stop (SafetyManager)
 */
namespace FixtureLook
{
    /** The middle of the first capability of $ch whose name has one of
     *  $words (the first word before the others, the case ignored), -1 when
     *  none */
    int capabilityValue(const QLCChannel *ch, const QStringList &words);

    /** The open white: the dimmers full, white from the colours (red,
     *  green, blue and white full; CMY, amber, UV... at 0), the shutter
     *  open, the colour wheel open or white */
    QMap<quint32, uchar> openWhite(const Fixture *fixture);

    /** The light of the show stop at $level (0-255): the dimmers a dimmer
     *  level sets (FXScene::levelDimmerChannels) at $level, its other
     *  intensities without a colour at 0; red, green, blue and white full
     *  with a dimmer ($level without one: their mix is its light), CMY,
     *  amber, UV... at 0; its shutters open (shutterOpen), its colour wheels
     *  open or white, its programs (Effect channels) at "no function" */
    QMap<quint32, uchar> showStopLight(const Fixture *fixture, uchar level);

    /** Its shutters open (not an iris): the range its definition marks open
     *  (ShutterOpen), of several the one named "open" or "no strobe"; a
     *  channel without one: the "open" of the channel, a strobe channel
     *  without it its "no strobe", and a Strobe slow to fast without any, 0 */
    QMap<quint32, uchar> shutterOpen(const Fixture *fixture);

    /** Its pan and tilt channels, the fine ones too */
    QList<quint32> motionChannels(const Fixture *fixture);

    /** Its intensity channels: dimmers and colours (the Intensity group) */
    QList<quint32> intensityChannels(const Fixture *fixture);
}

#endif

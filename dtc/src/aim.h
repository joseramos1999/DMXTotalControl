/*
  DMX Total Control
  aim.h

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

#ifndef AIM_H
#define AIM_H

#include <QVector3D>
#include <QPointF>

/**
 * Where a moving head is, for Follow: on the stage plan (mm: x across, y
 * from the back to the audience) at a height (mm over the stage floor),
 * facing a way (degrees, clockwise on the plan; 0: the audience), hung or
 * standing, with the travel of its pan and tilt (degrees).
 *
 * The head points along its own axis (down when hung, up when standing)
 * with the tilt in the middle of its travel, and with the pan in the middle
 * a tilt above that leans it the way it faces. A head that does it the
 * other way round has its pan or tilt inverted.
 */
struct AimMount
{
    QVector3D position;
    qreal facing = 0.0;
    bool hanging = true;
    qreal panRange = 540.0;
    qreal tiltRange = 270.0;
    bool panInverted = false;
    bool tiltInverted = false;
    /** A scanner: a mirror moves the beam, not the head (Follow leaves it) */
    bool mirror = false;
};

namespace Aim
{
    /** The pan and tilt (degrees, 0..range) of $mount aiming at $target
     *  (mm): of the ways to get there, the one nearest $previous when $hasPrevious
     *  (a target dragged: no swings the long way round), else the one with
     *  the pan nearest the middle. @return false when the head can't get
     *  there: the nearest it can, clamped */
    bool solve(const AimMount &mount, const QVector3D &target, const QPointF &previous, bool hasPrevious,
               QPointF *panTilt);
}

#endif

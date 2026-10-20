/*
  DMX Total Control
  aim.cpp

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

#include <QtMath>
#include <QList>
#include <limits>

#include "aim.h"

bool Aim::solve(const AimMount &mount, const QVector3D &target, const QPointF &previous, bool hasPrevious,
                QPointF *panTilt)
{
    const qreal panRange = qMax(1.0, mount.panRange);
    const qreal tiltRange = qMax(1.0, mount.tiltRange);
    const QVector3D d = target - mount.position;

    // on the plan: how far ahead of the head and to its side (the plan's y
    // goes down to the audience, the angles clockwise)
    const qreal r = qDegreesToRadians(mount.facing);
    const qreal ahead = -d.x() * qSin(r) + d.y() * qCos(r);
    const qreal side = d.x() * qCos(r) + d.y() * qSin(r);
    const qreal across = qSqrt(ahead * ahead + side * side);

    // the lean from its own axis, and the turn from where it faces; within
    // half a degree of its axis the turn means nothing (a pixel under the
    // head would swing the pan): it stays
    const qreal lean = qRadiansToDegrees(qAtan2(across, mount.hanging ? -d.z() : d.z()));
    const bool onAxis = across < 1.0 || qMin(lean, 180.0 - lean) < 0.5;
    const qreal turn = onAxis ? 0.0 : qRadiansToDegrees(qAtan2(side, ahead));

    // seen from above, a hung head turns the other way
    const qreal panSign = (mount.hanging ? -1.0 : 1.0) * (mount.panInverted ? -1.0 : 1.0);
    const qreal tiltSign = mount.tiltInverted ? -1.0 : 1.0;

    // every way: the turn (or the turn round and the lean back), with the
    // whole turns a long pan has
    struct Way { qreal pan; qreal tilt; bool back; };
    QList<Way> ways;
    for (int k = -2; k <= 2; k++)
    {
        ways.append({ panRange / 2 + panSign * (turn + 360.0 * k), tiltRange / 2 + tiltSign * lean, false });
        ways.append({ panRange / 2 + panSign * (turn + 180.0 + 360.0 * k), tiltRange / 2 - tiltSign * lean, true });
    }
    // right on its axis: no turn, where it is
    if (onAxis && hasPrevious)
    {
        for (Way &way : ways)
            way.pan += previous.x() - panRange / 2;
    }

    auto outside = [&](const Way &way) {
        auto beyond = [](qreal value, qreal range) {
            return value < 0 ? -value : (value > range ? value - range : 0.0);
        };
        return beyond(way.pan, panRange) + beyond(way.tilt, tiltRange);
    };
    // (as near one way as the other: not turned round, the same on every head)
    auto cost = [&](const Way &way) {
        const qreal back = way.back ? 1e-3 : 0.0;
        if (hasPrevious)
            return qAbs(way.pan - previous.x()) + qAbs(way.tilt - previous.y()) + back;
        return qAbs(way.pan - panRange / 2) + back;
    };

    // the nearest one it can reach; none: the least out of reach
    const Way *best = nullptr;
    qreal bestOutside = std::numeric_limits<qreal>::max();
    qreal bestCost = std::numeric_limits<qreal>::max();
    for (const Way &way : std::as_const(ways))
    {
        const qreal out = outside(way);
        const qreal c = cost(way);
        if (out < bestOutside - 1e-6 || (qAbs(out - bestOutside) <= 1e-6 && c < bestCost - 1e-9))
        {
            best = &way;
            bestOutside = out;
            bestCost = c;
        }
    }

    if (panTilt != nullptr)
        *panTilt = QPointF(qBound(0.0, best->pan, panRange), qBound(0.0, best->tilt, tiltRange));
    return bestOutside <= 1e-6;
}

/*
  DMX Total Control
  effectexchange.h

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

#ifndef EFFECTEXCHANGE_H
#define EFFECTEXCHANGE_H

#include <QByteArray>
#include <QString>
#include <QList>
#include <QHash>

class Doc;

/**
 * Effect racks (and any scene) in and out of a show: export, import and
 * convert to steps.
 *
 * An export holds the function, everything it plays (rack members, the
 * channels of steps, the fixture groups of colour effects) and the fixtures
 * it uses. An import matches those fixtures with the fixtures of the show
 * (same address and model first, then the same name, then the same model).
 */
namespace EffectExchange
{
    /** The XML of an export of function $id */
    QByteArray exportXml(Doc *doc, quint32 id);

    bool exportToFile(Doc *doc, quint32 id, const QString &path, QString *error);

    /**
     * Import the file into $doc.
     * @param missing set to the number of exported fixtures with no match
     * @return the ID of the imported function, Function::invalidId() on error
     */
    quint32 importFromFile(Doc *doc, const QString &path, QString *error, int *missing);
    quint32 importXml(Doc *doc, const QByteArray &xml, QString *error, int *missing);

    /**
     * Copy functions $roots of $from, and what they play, into $doc, on the
     * fixtures of $doc that match the ones they use (like an import).
     * @param missing set to the number of used fixtures with no match
     * @param knownFixtures fixtures of $from already copied into $doc (the
     *        patch imported with them), by their ID in $from: they go there
     * @return the new ID of each copied function, by its ID in $from
     */
    QHash<quint32, quint32> importFunctions(Doc *doc, Doc *from, const QList<quint32> &roots, int *missing,
                                            const QHash<quint32, quint32> &knownFixtures = QHash<quint32, quint32>());

    /** A group of scenes of another show */
    struct ShowZone
    {
        QString name;
        QString color;
        bool exclusive = true;
        QList<quint32> functions;
    };

    /**
     * Read another show (DMX Total Control or QLC+) into a new Doc: its
     * fixtures (with the definitions of $source), fixture groups, functions
     * and 2D plan. $zones gets its groups of scenes.
     * @return the Doc, to delete, or nullptr on error
     */
    Doc *loadShow(Doc *source, const QString &path, QList<ShowZone> *zones, QString *error);

    /** The longest cycle renderToSteps records (ms), and its most steps */
    constexpr uint MaxRenderCycle = 300000;
    constexpr uint MaxRenderSteps = 256;

    /**
     * Play function $id offline for one cycle and record what it outputs on
     * its fixtures, every $stepMs, into a new Steps scene. A cycle longer
     * than MaxRenderSteps steps gets longer steps; one longer than
     * MaxRenderCycle is recorded up to there.
     * @return the ID of the new scene, Function::invalidId() on error
     */
    quint32 renderToSteps(Doc *doc, quint32 id, uint stepMs, QString *error);

    /** The length of one cycle of a function, in ms (0 for static scenes) */
    uint cycleDuration(Doc *doc, quint32 id);
}

#endif

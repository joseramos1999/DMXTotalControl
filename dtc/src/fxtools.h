/*
  DMX Total Control
  fxtools.h

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

#ifndef FXTOOLS_H
#define FXTOOLS_H

#include <QObject>
#include <QVariant>
#include <QMap>

#include "fxscene.h"

class Programmer;
class Doc;

/** The FX layers as the UI sees them */
namespace FxTools
{
    QVariantMap layerToMap(const FXScene::Layer &layer);
    /** $base with the keys of $map */
    FXScene::Layer layerFromMap(const QVariantMap &map, const FXScene::Layer &base = FXScene::Layer());
    /** A new FX on $param: a circle for pan/tilt, a sine wave for the rest */
    FXScene::Layer defaultLayer(const QString &param);
    /** What linked layers share: the speed and the timing */
    QStringList timingKeys();
    /** What a fixture can do its own way in an FX: the shape, the size, the
     *  direction, the phase. The speed and the timing are the FX's, so its
     *  fixtures stay together */
    QStringList ownKeys();
    /** The $fixtures of $layer take the ownKeys() of $changes as their own */
    void setOwn(FXScene::Layer &layer, const QList<quint32> &fixtures, const QVariantMap &changes);
    /** The fixtures of $layer (only $fixtures, when given) do what the layer
     *  does on $keys again */
    void clearOwn(FXScene::Layer &layer, const QStringList &keys, const QList<quint32> &fixtures = {});

    /** What each fixture of an FX has by hand: its shift (degrees behind) and
     *  its delay (ms behind). Not in ownKeys(): changing the shape, the phase
     *  or the timing, or "Like the rest", keeps them */
    QStringList offsetKeys();
    /** The $fixtures of $layer get $value (bounded) as their $key of offsetKeys().
     *  The fixtures are not checked against the layer (one with no fixtures of
     *  its own has all the ones of its scene): the callers do */
    void setOffset(FXScene::Layer &layer, const QList<quint32> &fixtures, const QString &key, int value);
    /** $key spread over $inEffectOrder: 0 on the first to $last on the last */
    void fanOffset(FXScene::Layer &layer, const QList<quint32> &inEffectOrder, const QString &key, int last);
    /** How many fixtures of $layer do the shape, size, direction or phase their own way */
    int ownShapeCount(const FXScene::Layer &layer);
    /** How many fixtures of $layer have a shift or a delay */
    int offsetCount(const FXScene::Layer &layer);
    /** The fixtures of $layer (or $sceneFixtures when it has none of its own)
     *  in the order of the effect: [{ id, name, selected, place (degrees
     *  behind in the wave, -1: none), step (1..., 0: none), shift, delay,
     *  virtual (a dimmer FX dims its colour: it has no dimmer) }] */
    QVariantList fixtureRows(const Doc *doc, const FXScene::Layer &layer, const QList<quint32> &sceneFixtures,
                             const QList<quint32> &selected);
    /** DMX Total Control: the virtual dimmer of $layer on $fixtures (its own
     *  ones, or these when it has none): { virtualCount (the fixtures without
     *  a dimmer a dimmer FX dims), virtualBase (-1: auto), virtualLevel (the
     *  level it dims from, 0-255, auto from the bases in $values). The other
     *  FX dim none: their level is their base, or 255 */
    QVariantMap virtualDimmer(const Doc *doc, const FXScene::Layer &layer, const QList<quint32> &fixtures,
                              const QHash<QPair<quint32, quint32>, uchar> &values);

    /** DMX Total Control: what one fixture of an FX did, for a macro saved
     *  fixture by fixture: its base values by parameter (0-255) and what it
     *  did its own way (its shift and delay included) */
    struct FixtureLook
    {
        QMap<QString, int> bases;
        FXScene::Own own;

        bool operator==(const FixtureLook &other) const { return bases == other.bases && own == other.own; }
    };
    /** The values $own has (ownKeys() and offsetKeys()), and back, bounded */
    QVariantMap ownToMap(const FXScene::Own &own);
    FXScene::Own ownFromMap(const QVariantMap &map);
    /** Whether the bases of $param spread between two places (a fan); not
     *  the wheels and the shutter, whose values are slots */
    bool interpolates(const QString &param);
    /** The place (0..m-1) of $m looks the fixture $i of $n takes: the first
     *  the first, the last the last, the ones in between spread */
    int nearestPlace(int i, int n, int m);
    /** What the fixture $i of $n takes from $looks: the shape and the slots
     *  from its nearest place, the other bases, the shift and the delay in
     *  between the two places around it */
    FixtureLook lookAt(const QList<FixtureLook> &looks, int i, int n);

    /** $samples values (-1..1) of a mode, to draw it */
    QVariantList modeCurve(int mode, int samples);
    /** $samples points {x, y} (-1..1) of a figure, to draw it */
    QVariantList figurePath(int figure, int samples);

    /** Old effects as FX: a Move FX or a Curve FX. Nullptr for anything else */
    FXScene *convert(Doc *doc, const Function *function);
}

/**
 * The FX macros: ready made effects that go on the selected fixtures
 * of the programmer, the factory ones and the ones the user saves. The
 * user's are global: a file in the user folder, for every show.
 */
class FxMacros : public QObject
{
    Q_OBJECT

    /** [{ id, name, category, factory, panTilt, figure, mode, params,
     *  perFixture (saved fixture by fixture), fixtureCount (how many) }] */
    Q_PROPERTY(QVariantList macros READ macros NOTIFY macrosChanged)

public:
    FxMacros(Programmer *programmer, QObject *parent = nullptr);

    QVariantList macros() const;

    /** Put a macro on the selection of the programmer. @return false if none of
     *  its parameters is there */
    Q_INVOKABLE bool apply(const QString &id);
    /** The FX of the programmer as a new macro: one layer by parameter, with
     *  what each of its fixtures did by its place in the FX (none when they
     *  all did the same). @return its id, or "" */
    Q_INVOKABLE QString save(const QString &name);
    Q_INVOKABLE void rename(const QString &id, const QString &name);
    Q_INVOKABLE void remove(const QString &id);

    /** Where the user's macros are (tests use another file) */
    QString filePath() const;
    void setFilePath(const QString &path);

signals:
    void macrosChanged();

private:
    struct Macro
    {
        QString id;
        QString name;
        QString category;
        bool factory = false;
        QList<FXScene::Layer> layers;
        /** base values by parameter, 0-255 */
        QMap<QString, int> bases;
        /** DMX Total Control: by layer, what each fixture of its FX did, in
         *  their order. Empty: the same for all (for any selection) */
        QList<QList<FxTools::FixtureLook>> looks;
    };

    void buildFactory();
    void load();
    bool write() const;
    const Macro *find(const QString &id) const;

    Programmer *m_programmer;
    QList<Macro> m_factory;
    QList<Macro> m_user;
    QString m_path;
};

#endif

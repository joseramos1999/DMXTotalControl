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

    /** [{ id, name, category, factory, kind, figure, mode }] */
    Q_PROPERTY(QVariantList macros READ macros NOTIFY macrosChanged)

public:
    FxMacros(Programmer *programmer, QObject *parent = nullptr);

    QVariantList macros() const;

    /** Put a macro on the selection of the programmer. @return false if none of
     *  its parameters is there */
    Q_INVOKABLE bool apply(const QString &id);
    /** The FX of the programmer as a new macro. @return its id, or "" */
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

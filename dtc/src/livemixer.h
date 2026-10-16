/*
  DMX Total Control
  livemixer.h

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

#ifndef LIVEMIXER_H
#define LIVEMIXER_H

#include <QObject>
#include <QVariant>
#include <QMap>

class Doc;

/**
 * The live mixer: one mixer per fixture group and one for all
 * the fixtures, working on what leaves the software, on top of every scene
 * and live edit. Its settings are live: they are not saved in the show.
 *
 * Mixer IDs: -1 for all the fixtures, else the ID of a fixture group.
 * Keys: dimmer (0..1), hue (0..1), saturation (0..1) — hue and saturation
 * replace the colour until resetColour —, strobe (bool), strobeHz,
 * full, blackout, flash (bool), fullExclusive, flashExclusive (bool: the
 * other fixtures go dark).
 */
class LiveMixer : public QObject
{
    Q_OBJECT

    /** [{ id, name, fixtures, dimmer, colour, hue, saturation, strobe, strobeHz,
     *     full, blackout, flash, fullExclusive, flashExclusive }] */
    Q_PROPERTY(QVariantList mixers READ mixers NOTIFY mixersChanged)
    /** Whether some mixer changes the output */
    Q_PROPERTY(bool active READ isActive NOTIFY mixersChanged)

public:
    LiveMixer(Doc *doc, QObject *parent = nullptr);
    ~LiveMixer();

    QVariantList mixers() const;
    bool isActive() const;

    Q_INVOKABLE void set(int mixerId, const QString &key, const QVariant &value);
    Q_INVOKABLE void resetColour(int mixerId);
    /** Every mixer back to neutral */
    Q_INVOKABLE void reset();

signals:
    void mixersChanged();

private slots:
    void slotFixturesChanged();
    void slotDocCleared();

private:
    struct State
    {
        qreal dimmer = 1.0;
        bool colour = false;
        qreal hue = 0.0;
        qreal saturation = 1.0;
        bool strobe = false;
        qreal strobeHz = 8.0;
        bool full = false;
        bool blackout = false;
        bool flash = false;
        bool fullExclusive = false;
        bool flashExclusive = false;
    };

    bool isNeutral(const State &s) const;
    /** The fixtures of a mixer */
    QList<quint32> fixturesOf(int mixerId) const;
    /** Rebuild the mixer layers of the universes */
    void apply();

private:
    Doc *m_doc;
    QMap<int, State> m_states;
};

#endif

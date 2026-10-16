/*
  DMX Total Control
  curvefx.h

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

#ifndef CURVEFX_H
#define CURVEFX_H

#include <QSharedPointer>
#include <QStringList>
#include <QMutex>
#include <QList>
#include <QMap>

#include "function.h"

class GenericFader;
class Doc;

/** @addtogroup engine_functions Functions
 * @{
 */

#define KXMLDTCCurve        QStringLiteral("Curve")
#define KXMLDTCCurveFixture QStringLiteral("Fixture")
#define KXMLDTCCurveFeature QStringLiteral("Feature")

/**
 * Curve FX: a waveform drives any channel of the fixtures, such
 * as the dimmer, the zoom or the focus. Each fixture can be offset along the
 * wave by its index. In relative mode the wave moves the channel around the
 * value set by other scenes instead of setting it.
 */
class CurveFX final : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(CurveFX)

public:
    /** Strobe: a short flash each cycle; RandomStrobe: each cycle, flashes on a
     *  random third of the fixtures */
    enum Waveform { Sine = 0, Triangle, RampUp, RampDown, Square, Random, Strobe, RandomStrobe };
    Q_ENUM(Waveform)

    /** Live attributes: Size scales the swing around its middle, Phase adds degrees */
    enum CurveAttr
    {
        Intensity = Function::Intensity,
        Size,
        Phase
    };

    CurveFX(Doc *doc);
    ~CurveFX();

    QIcon getIcon() const override;
    quint32 totalDuration() override;

    Function *createCopy(Doc *doc, bool addToDoc = true) override;
    bool copyFrom(const Function *function) override;

    /*********************************************************************
     * Contents
     *********************************************************************/
public:
    /** The fixtures of the effect; their order is their index along the wave */
    void setFixtures(const QList<quint32> &fixtures);
    QList<quint32> fixtures() const;

    /** The channels driven, by lowercase channel name ("dimmer", "zoom"...) */
    void setFeatures(const QStringList &features);
    QStringList features() const;

    void setWaveform(Waveform wave);
    Waveform waveform() const;
    static QString waveformToString(Waveform wave);
    static Waveform stringToWaveform(const QString &str);
    static QStringList waveformNames();

    /** The wave swings between low and high (DMX values) */
    void setLow(int value);
    int low() const;
    void setHigh(int value);
    int high() const;

    /** Phase of the first fixture, in degrees */
    void setPhaseOffset(int degrees);
    int phaseOffset() const;

    /** Phase difference from the first to the last fixture, in degrees (360 = a whole wave) */
    void setSpread(int degrees);
    int spread() const;

    /** How long a square or strobe wave is up, in % of the cycle (1-99) */
    void setDuty(int percent);
    int duty() const;

    /** Move the channels around the value set by other scenes */
    void setRelative(bool relative);
    bool isRelative() const;

    /** The wave at position $pos (0..1), between 0 and 1. $seed varies the Random wave */
    static qreal waveValue(Waveform wave, qreal pos, quint32 seed = 0, qreal duty = 0.5);

    /** The value of fixture $index (of $count) at the cycle position $pos, for previews */
    qreal valueAt(qreal pos, int index, int count) const;

public slots:
    void slotFixtureRemoved(quint32 fxi_id) override;

private:
    QList<quint32> m_fixtures;
    QStringList m_features;
    Waveform m_waveform;
    int m_low;
    int m_high;
    int m_phaseOffset;
    int m_spread;
    int m_duty;
    bool m_relative;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool saveXML(QXmlStreamWriter *doc) const override;
    bool loadXML(QXmlStreamReader &root) override;
    /** DMX Total Control: the fixtures that are not in the show leave it (a
     *  fixture patched later would get their IDs) */
    void postLoad() override;

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    void preRun(MasterTimer *timer) override;
    void write(MasterTimer *timer, QList<Universe *> universes) override;
    void postRun(MasterTimer *timer, QList<Universe *> universes) override;

    int adjustAttribute(qreal fraction, int attributeId) override;

private:
    struct Target
    {
        quint32 fixture;
        quint32 channel;
        quint32 universe;
        int index;
    };

    QSharedPointer<GenericFader> getFader(Universe *universe);
    /** The channels of the features, fixture by fixture ($count: the
     *  fixtures with any). Called with the targets mutex held */
    QList<Target> buildTargets(int *count) const;
    /** Fixtures or features changed: a running curve drives the new
     *  channels and lets the others go, without a restart */
    void updateTargets();

    QList<Target> m_targets;
    /** DMX Total Control: the channels of the fixtures removed while it
     *  plays, to take out of its faders. The targets change under the mutex
     *  (fixtures removed in the GUI thread, written in the timer thread) */
    QList<Target> m_droppedTargets;
    QMutex m_targetsMutex;
    int m_targetCount;
    uint m_cycleElapsed;
    quint32 m_cycleNumber;
};

/** @} */

#endif

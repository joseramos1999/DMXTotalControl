/*
  DMX Total Control
  safetyfade.cpp

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

#include <QTimer>

#include "safetymanager.h"
#include "inputoutputmap.h"
#include "doc.h"

/*********************************************************************
 * The fade to black: the intensities go down at the output in its time
 * (OutputLayer::rampId, above the grand master and the parks that don't
 * stay in blackout; never the special effects), then Blackout. The light
 * is full again under the Blackout: Blackout off brings it back
 *********************************************************************/

int SafetyManager::fadeToBlackTime(const QVariantMap &m)
{
    return qBound(fadeToBlackMinMs(), m.value(QStringLiteral("time"), fadeToBlackDefaultMs()).toInt(),
                  fadeToBlackMaxMs());
}

void SafetyManager::fadeToBlack(int ms)
{
    // pressed again during it: Blackout at once
    if (m_rampId != 0)
    {
        m_doc->inputOutputMap()->setBlackout(true);
        endRamp();
        return;
    }
    // (dark already; the program closes)
    if (m_closing || m_doc->inputOutputMap()->blackout())
        return;

    // the special effects first (they never fade: the gate gives them 0)
    disarm(QStringLiteral("fadeToBlack"));
    m_rampMs = quint32(qBound(fadeToBlackMinMs(), ms, fadeToBlackMaxMs()));
    // (a new one each time: the universes start it when its layer comes)
    m_rampId = ++m_ramps;
    if (m_rampId == 0)
        m_rampId = ++m_ramps;
    push();
    m_rampTimer->start(int(m_rampMs));
    emit fadeToBlackChanged();
}

void SafetyManager::slotRampDone()
{
    // Blackout first (it ends the ramp: slotBlackoutChanged), then the light
    // full under it
    m_doc->inputOutputMap()->setBlackout(true);
    endRamp();
}

void SafetyManager::endRamp(bool push)
{
    m_rampTimer->stop();
    if (m_rampId == 0)
        return;
    m_rampId = 0;
    m_rampMs = 0;
    if (push)
        this->push();
    emit fadeToBlackChanged();
}

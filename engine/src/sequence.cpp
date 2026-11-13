/*
  Q Light Controller Plus
  sequence.cpp

  Copyright (C) Massimo Callegari

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>
#include <QSet>
#include <algorithm>

#include "sequence.h"

#define KXMLQLCSequenceBoundScene QStringLiteral("BoundScene")

Sequence::Sequence(Doc* doc)
    : Chaser(doc)
    , m_boundSceneID(Function::invalidId())
    , m_needFixup(true)
{
    m_type = Function::SequenceType;
    setName(tr("New Sequence"));
}

Sequence::~Sequence()
{
}

QIcon Sequence::getIcon() const
{
    return QIcon(":/sequence.png");
}

Function *Sequence::createCopy(Doc *doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Sequence(doc);
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    if (addToDoc == true && doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }

    return copy;
}

bool Sequence::copyFrom(const Function *function)
{
    const Sequence* sequence = qobject_cast<const Sequence*> (function);
    if (sequence == NULL)
        return false;

    // Copy sequence stuff
    m_steps = sequence->m_steps;
    m_fadeInMode = sequence->m_fadeInMode;
    m_fadeOutMode = sequence->m_fadeOutMode;
    m_holdMode = sequence->m_holdMode;
    m_boundSceneID = sequence->m_boundSceneID;

    // Copy common function stuff
    return Function::copyFrom(function);
}

void Sequence::setBoundSceneID(quint32 sceneID)
{
    m_boundSceneID = sceneID;
}

quint32 Sequence::boundSceneID() const
{
    return m_boundSceneID;
}

QList<quint32> Sequence::components() const
{
    QList<quint32> ids;
    if (m_boundSceneID != Function::invalidId())
        ids.append(m_boundSceneID);
    return ids;
}

void Sequence::applyDumpValues(const QList<SceneValue> &dumpedValues, int targetStepIndex)
{
    Scene *scene = qobject_cast<Scene *>(doc()->function(boundSceneID()));
    if (scene == nullptr)
        return;

    QList<SceneValue> sceneValues = scene->values();
    std::sort(sceneValues.begin(), sceneValues.end());

    QList<SceneValue> zeroSceneValues = sceneValues;
    for (SceneValue &scv : zeroSceneValues)
        scv.value = 0;

    // Keep step values aligned with the bound Scene channels, preserving
    // existing step values where channels already exist.
    for (int i = 0; i < stepsCount(); i++)
    {
        ChaserStep step = steps().at(i);
        QList<SceneValue> oldValues = step.values;

        step.values = zeroSceneValues;
        for (const SceneValue &oldScv : oldValues)
        {
            int index = step.values.indexOf(oldScv);
            if (index != -1)
                step.values.replace(index, oldScv);
        }

        replaceStep(step, i);
    }

    if (targetStepIndex < 0 || targetStepIndex >= stepsCount())
    {
        ChaserStep newStep(boundSceneID());
        newStep.values = zeroSceneValues;
        for (const SceneValue &scv : dumpedValues)
            newStep.setValue(scv);
        addStep(newStep);
    }
    else
    {
        ChaserStep step = steps().at(targetStepIndex);
        for (const SceneValue &scv : dumpedValues)
            step.setValue(scv);
        replaceStep(step, targetStepIndex);
    }
}

/*****************************************************************************
 * DMX Total Control: palettes
 *****************************************************************************/

void Sequence::applyPalette(int paletteId, const std::function<int(quint32, quint32)> &valueOf)
{
    if (!valueOf)
        return;

    // the channels that use it, then their values outside the lock (the
    // caller reads its own palettes)
    QSet<QPair<quint32, quint32>> keys;
    {
        QMutexLocker locker(&m_stepListMutex);
        for (const ChaserStep &step : std::as_const(m_steps))
            for (auto it = step.palettes.cbegin(); it != step.palettes.cend(); ++it)
                if (it.value() == paletteId)
                    keys.insert(it.key());
    }
    if (keys.isEmpty())
        return;

    QHash<QPair<quint32, quint32>, int> newValues;
    for (const QPair<quint32, quint32> &key : std::as_const(keys))
        newValues.insert(key, valueOf(key.first, key.second));

    // what changed in each step
    QList<QList<SceneValue>> changedValues;
    bool modified = false;
    {
        QMutexLocker locker(&m_stepListMutex);
        for (ChaserStep &step : m_steps)
        {
            QList<SceneValue> changedHere;
            for (auto it = step.palettes.begin(); it != step.palettes.end();)
            {
                if (it.value() != paletteId)
                {
                    ++it;
                    continue;
                }
                const int value = newValues.value(it.key(), -1);
                // the palette doesn't have this channel any more: a plain value
                if (value < 0)
                {
                    it = step.palettes.erase(it);
                    modified = true;
                    continue;
                }
                const SceneValue scv(it.key().first, it.key().second, uchar(qBound(0, value, 255)));
                const int index = int(step.values.indexOf(scv));
                if (index >= 0 && step.values.at(index).value != scv.value)
                {
                    step.values[index].value = scv.value;
                    changedHere.append(scv);
                    modified = true;
                }
                ++it;
            }
            changedValues.append(changedHere);
        }
    }
    if (modified == false)
        return;
    emit changed(id());

    // the step on stage plays them at once, without a fade (as a cuelist).
    // Not blind: the running scene puts them in its faders, in place of what
    // they had (no HTP check: a palette can also lower a value)
    if (isRunning() == false)
        return;
    const int current = currentStepIndex();
    Scene *bound = qobject_cast<Scene *>(doc()->function(boundSceneID()));
    if (bound == nullptr || current < 0 || current >= changedValues.count())
        return;
    for (const SceneValue &scv : changedValues.at(current))
        bound->setValue(scv, false, false);
}

void Sequence::forgetPalette(int paletteId)
{
    bool modified = false;
    {
        QMutexLocker locker(&m_stepListMutex);
        for (ChaserStep &step : m_steps)
        {
            for (auto it = step.palettes.begin(); it != step.palettes.end();)
            {
                if (it.value() == paletteId)
                {
                    it = step.palettes.erase(it);
                    modified = true;
                }
                else
                {
                    ++it;
                }
            }
        }
    }
    if (modified)
        emit changed(id());
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

void Sequence::slotFixtureRemoved(quint32 fxID)
{
    // DMX Total Control: its palettes go with its values
    bool palettesRemoved = false;
    for (ChaserStep& step : m_steps)
    {
        QMutableListIterator<SceneValue> it(step.values);
        while (it.hasNext())
        {
            SceneValue& value(it.next());
            if (value.fxi == fxID)
                it.remove();
        }
        for (auto ref = step.palettes.begin(); ref != step.palettes.end();)
        {
            if (ref.key().first == fxID)
            {
                ref = step.palettes.erase(ref);
                palettesRemoved = true;
            }
            else
            {
                ++ref;
            }
        }
    }
    // (the palettes it uses may have changed)
    if (palettesRemoved)
        emit changed(id());
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Sequence::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    doc->writeAttribute(KXMLQLCSequenceBoundScene, QString::number(boundSceneID()));

    /* Tempo type */
    saveXMLTempoType(doc);

    /* Speed */
    saveXMLSpeed(doc);

    /* Direction */
    saveXMLDirection(doc);

    /* Run order */
    saveXMLRunOrder(doc);

    /* Speed modes */
    doc->writeStartElement(KXMLQLCChaserSpeedModes);
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeIn, speedModeToString(fadeInMode()));
    doc->writeAttribute(KXMLQLCFunctionSpeedFadeOut, speedModeToString(fadeOutMode()));
    doc->writeAttribute(KXMLQLCFunctionSpeedDuration, speedModeToString(durationMode()));
    doc->writeEndElement();

    /* Steps */
    for (int i = 0; i < m_steps.count(); i++)
        m_steps.at(i).saveXML(doc, i, true);

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

bool Sequence::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    QXmlStreamAttributes funcAttrs = root.attributes();

    if (funcAttrs.value(KXMLQLCFunctionType).toString() != typeToString(Function::SequenceType))
    {
        qWarning() << Q_FUNC_INFO << funcAttrs.value(KXMLQLCFunctionType).toString()
                   << "is not a Sequence";
        return false;
    }

    if (funcAttrs.hasAttribute(KXMLQLCSequenceBoundScene) == false)
    {
        qWarning() << Q_FUNC_INFO << "Sequence doesn't have a bound Scene ID";
        return false;
    }

    setBoundSceneID(funcAttrs.value(KXMLQLCSequenceBoundScene).toString().toUInt());

    Scene *scene = qobject_cast<Scene *>(doc()->function(boundSceneID()));
    QList<SceneValue> sceneValues;
    if (scene != NULL)
    {
        sceneValues = scene->values();
        std::sort(sceneValues.begin(), sceneValues.end());
        m_needFixup = false;
    }

    /* Load Sequence contents */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCFunctionSpeed)
        {
            loadXMLSpeed(root);
        }
        else if (root.name() == KXMLQLCFunctionDirection)
        {
            loadXMLDirection(root);
        }
        else if (root.name() == KXMLQLCFunctionRunOrder)
        {
            loadXMLRunOrder(root);
        }
        else if (root.name() == KXMLQLCFunctionTempoType)
        {
            loadXMLTempoType(root);
        }
        else if (root.name() == KXMLQLCChaserSpeedModes)
        {
            loadXMLSpeedModes(root);
        }
        else if (root.name() == KXMLQLCFunctionStep)
        {
            //! @todo stepNumber is useless if the steps are in the wrong order
            ChaserStep step;
            int stepNumber = -1;

            if (sceneValues.isEmpty() == false)
                step.values = sceneValues;

            if (step.loadXML(root, stepNumber, doc()) == true)
            {
                step.fid = boundSceneID();

                if (stepNumber >= m_steps.size())
                    m_steps.append(step);
                else
                    m_steps.insert(stepNumber, step);
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Sequence tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

void Sequence::postLoad()
{
    if (m_needFixup == false)
        return;

    Doc* doc = this->doc();
    Q_ASSERT(doc != NULL);

    Scene *scene = qobject_cast<Scene *>(doc->function(boundSceneID()));
    QList<SceneValue> sceneValues;
    if (scene != NULL)
    {
        sceneValues = scene->values();

        if (sceneValues.count() == 0)
        {
            qDebug() << "The bound Scene is empty ! This should never happen. Trying to fix it...";
            if (stepsCount())
            {
                foreach (SceneValue value, m_steps.at(0).values)
                {
                    value.value = 0;
                    if (doc->fixture(value.fxi) != NULL)
                        scene->setValue(value);
                }
            }
            m_needFixup = false;
            return;
        }

        std::sort(sceneValues.begin(), sceneValues.end());
    }

    int stepIndex = 0;

    QMutableListIterator <ChaserStep> it(m_steps);
    while (it.hasNext() == true)
    {
        ChaserStep step(it.next());
        if (sceneValues.count() == step.values.count())
        {
            stepIndex++;
            continue;
        }

        QList <SceneValue> tmpList = step.values;
        step.values = sceneValues;
        for (int i = 0; i < tmpList.count(); i++)
        {
            int tmpIndex = step.values.indexOf(tmpList.at(i));
            if (tmpIndex == -1)
                continue;
            step.values.replace(tmpIndex, tmpList.at(i));
        }

        replaceStep(step, stepIndex);
        stepIndex++;
    }
    m_needFixup = false;

    qDebug() << "Sequence" << name() << "steps fixed. Values:" << sceneValues.count();
}

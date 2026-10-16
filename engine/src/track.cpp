/*
  Q Light Controller Plus
  track.cpp

  Copyright (c) Massimo Callegari

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
#include <qmath.h>
#include <algorithm>

#include "sequence.h"
#include "track.h"
#include "scene.h"
#include "show.h"
#include "doc.h"

#define KXMLQLCTrackName      QStringLiteral("Name")
#define KXMLQLCTrackSceneID   QStringLiteral("SceneID")
#define KXMLQLCTrackIsMute    QStringLiteral("isMute")

#define KXMLQLCTrackFunctions QStringLiteral("Functions")
// DMX Total Control
#define KXMLDTCTrackLocked     QStringLiteral("Locked")
#define KXMLDTCTrackHidden     QStringLiteral("Hidden")
#define KXMLDTCTrackAutomation QStringLiteral("Automation")

Track::Track(quint32 sceneID, QObject *parent)
    : QObject(parent)
    , m_id(Track::invalidId())
    , m_showId(Function::invalidId())
    , m_sceneID(sceneID)
    , m_isMute(false)
    , m_locked(false)
    , m_hidden(false)
{
    setName(tr("New Track"));
}

Track::~Track()
{

}

/** DMX Total Control: the runner mutex of the show of $track (null without
 *  a show): the running show reads the tracks and holds their blocks */
static QRecursiveMutex *runnerMutexOf(const Track *track)
{
    Show *show = qobject_cast<Show *>(track->parent());
    return show != NULL ? show->runnerMutex() : NULL;
}

/****************************************************************************
 * ID
 ****************************************************************************/

void Track::setId(quint32 id)
{
    m_id = id;
}

quint32 Track::id() const
{
    return m_id;
}

quint32 Track::invalidId()
{
    return UINT_MAX;
}

quint32 Track::showId() const
{
    return m_showId;
}

void Track::setShowId(quint32 id)
{
    m_showId = id;
}

/****************************************************************************
 * Name
 ****************************************************************************/

void Track::setName(const QString& name)
{
    m_name = name;
    emit changed(this->id());
}

QString Track::name() const
{
    return m_name;
}

/*********************************************************************
 * Scene
 *********************************************************************/
void Track::setSceneID(quint32 id)
{
    m_sceneID = id;
}

quint32 Track::getSceneID() const
{
    return m_sceneID;
}

/*********************************************************************
 * Mute state
 *********************************************************************/
void Track::setMute(bool state)
{
    if (m_isMute == state)
        return;

    m_isMute = state;

    emit muteChanged(state);
}

bool Track::isMute() const
{
    return m_isMute;
}

/*********************************************************************
 * DMX Total Control: lock, hide, automation
 *********************************************************************/

void Track::setLocked(bool locked)
{
    m_locked = locked;
}

bool Track::isLocked() const
{
    return m_locked;
}

void Track::setHidden(bool hidden)
{
    m_hidden = hidden;
}

bool Track::isHidden() const
{
    return m_hidden;
}

void Track::setAutomation(const QString &lane, const QList<AutomationPoint> &points)
{
    QList<AutomationPoint> sorted = points;
    std::sort(sorted.begin(), sorted.end(), [](const AutomationPoint &a, const AutomationPoint &b) {
        return a.time < b.time;
    });
    {
        // DMX Total Control: not while a running show reads it
        QMutexLocker locker(runnerMutexOf(this));
        if (sorted.isEmpty())
            m_automation.remove(lane);
        else
            m_automation.insert(lane, sorted);
    }
    emit changed(id());
}

QList<Track::AutomationPoint> Track::automation(const QString &lane) const
{
    return m_automation.value(lane);
}

bool Track::hasAutomation(const QString &lane) const
{
    return m_automation.value(lane).isEmpty() == false;
}

qreal Track::automationValue(const QString &lane, quint32 time, qreal fallback) const
{
    const QList<AutomationPoint> points = m_automation.value(lane);
    if (points.isEmpty())
        return fallback;
    if (time <= points.first().time)
        return points.first().value;
    if (time >= points.last().time)
        return points.last().value;

    for (int i = 0; i < points.count() - 1; i++)
    {
        const AutomationPoint &a = points.at(i);
        const AutomationPoint &b = points.at(i + 1);
        if (time >= b.time)
            continue;
        qreal f = b.time > a.time ? qreal(time - a.time) / qreal(b.time - a.time) : 1.0;
        // a smooth point eases in and out to the next one
        if (a.smooth)
            f = (1.0 - qCos(f * M_PI)) / 2.0;
        return a.value + (b.value - a.value) * f;
    }
    return points.last().value;
}

/*********************************************************************
 * Sequences
 *********************************************************************/

ShowFunction *Track::createShowFunction(quint32 functionID)
{
    Show *show = qobject_cast<Show *>(parent());
    quint32 uId = show == NULL ? 0 : show->getLatestShowFunctionId();
    ShowFunction *func = new ShowFunction(uId);
    func->setFunctionID(functionID);
    // DMX Total Control: not while a running show reads the blocks
    QMutexLocker locker(runnerMutexOf(this));
    m_functions.append(func);

    return func;
}

bool Track::addShowFunction(ShowFunction *func)
{
    if (func == NULL || func->functionID() == Function::invalidId())
        return false;

    // DMX Total Control: not while a running show reads the blocks
    QMutexLocker locker(runnerMutexOf(this));
    m_functions.append(func);

    return true;
}

ShowFunction *Track::showFunction(quint32 id) const
{
    foreach (ShowFunction *sf, m_functions)
        if (sf->id() == id)
            return sf;

    return NULL;
}

bool Track::removeShowFunction(ShowFunction *function, bool performDelete)
{
    // DMX Total Control: not while a running show reads the blocks; a block
    // deleted is forgotten first (what it plays stops)
    QMutexLocker locker(runnerMutexOf(this));
    if (m_functions.contains(function) == false)
        return false;

    ShowFunction *func = m_functions.takeAt(m_functions.indexOf(function));
    if (performDelete && func)
    {
        if (Show *show = qobject_cast<Show *>(parent()))
            show->forgetBlock(func);
        delete func;
    }

    return true;
}

QList <ShowFunction *> Track::showFunctions() const
{
    return m_functions;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/
bool Track::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* Track entry */
    doc->writeStartElement(KXMLQLCTrack);
    doc->writeAttribute(KXMLQLCTrackID, QString::number(this->id()));
    doc->writeAttribute(KXMLQLCTrackName, this->name());
    if (m_sceneID != Scene::invalidId())
        doc->writeAttribute(KXMLQLCTrackSceneID, QString::number(m_sceneID));
    doc->writeAttribute(KXMLQLCTrackIsMute, QString::number(m_isMute));
    // DMX Total Control
    if (m_locked)
        doc->writeAttribute(KXMLDTCTrackLocked, "1");
    if (m_hidden)
        doc->writeAttribute(KXMLDTCTrackHidden, "1");
    for (auto it = m_automation.cbegin(); it != m_automation.cend(); ++it)
    {
        doc->writeStartElement(KXMLDTCTrackAutomation);
        doc->writeAttribute("Lane", it.key());
        for (const AutomationPoint &point : it.value())
        {
            doc->writeStartElement("Point");
            doc->writeAttribute("Time", QString::number(point.time));
            doc->writeAttribute("Value", QString::number(point.value));
            if (point.smooth)
                doc->writeAttribute("Smooth", "1");
            doc->writeEndElement();
        }
        doc->writeEndElement();
    }

    /* Save the list of Functions if any is present */
    if (m_functions.isEmpty() == false)
    {
        foreach (ShowFunction *func, showFunctions())
            func->saveXML(doc);
    }

    doc->writeEndElement();

    return true;
}

bool Track::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCTrack)
    {
        qWarning() << Q_FUNC_INFO << "Track node not found";
        return false;
    }

    bool ok = false;
    QXmlStreamAttributes attrs = root.attributes();
    quint32 id = attrs.value(KXMLQLCTrackID).toString().toUInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid Track ID:" << attrs.value(KXMLQLCTrackID).toString();
        return false;
    }
    // Assign the ID to myself
    m_id = id;

    if (attrs.hasAttribute(KXMLQLCTrackName) == true)
        m_name = attrs.value(KXMLQLCTrackName).toString();

    if (attrs.hasAttribute(KXMLQLCTrackSceneID))
    {
        ok = false;
        id = attrs.value(KXMLQLCTrackSceneID).toString().toUInt(&ok);
        if (ok == false)
        {
            qWarning() << "Invalid Scene ID:" << attrs.value(KXMLQLCTrackSceneID).toString();
            return false;
        }
        m_sceneID = id;
    }

    ok = false;
    bool mute = attrs.value(KXMLQLCTrackIsMute).toString().toInt(&ok);
    if (ok == false)
    {
        qWarning() << "Invalid Mute flag:" << root.attributes().value(KXMLQLCTrackIsMute).toString();
        return false;
    }
    m_isMute = mute;
    // DMX Total Control
    m_locked = attrs.value(KXMLDTCTrackLocked).toString() == "1";
    m_hidden = attrs.value(KXMLDTCTrackHidden).toString() == "1";

    /* look for show functions */
    while (root.readNextStartElement())
    {
        if (root.name() == KXMLDTCTrackAutomation)
        {
            QString lane = root.attributes().value("Lane").toString();
            QList<AutomationPoint> points;
            while (root.readNextStartElement())
            {
                if (root.name() == QLatin1String("Point"))
                {
                    QXmlStreamAttributes p = root.attributes();
                    points.append({ p.value("Time").toString().toUInt(), p.value("Value").toString().toDouble(),
                                    p.value("Smooth").toString() == "1" });
                }
                root.skipCurrentElement();
            }
            m_automation.insert(lane, points);
            continue;
        }
        if (root.name() == KXMLShowFunction)
        {
            Show *show = qobject_cast<Show *>(parent());
            quint32 uId = show == NULL ? 0 : show->getLatestShowFunctionId();
            ShowFunction *newFunc = new ShowFunction(uId);
            newFunc->loadXML(root);
            if (addShowFunction(newFunc) == false)
                delete newFunc;
        }
        /* LEGACY code: to be removed */
        else if (root.name() == KXMLQLCTrackFunctions)
        {
            QString strvals = root.readElementText();
            if (strvals.isEmpty() == false)
            {
                QStringList varray = strvals.split(",");
                for (int i = 0; i < varray.count(); i++)
                    createShowFunction(QString(varray.at(i)).toUInt());
            }
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Track tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

bool Track::postLoad(Doc* doc)
{
    bool modified = false;
    QMutableListIterator<ShowFunction*> it(m_functions);
    while (it.hasNext())
    {
        ShowFunction* showFunction = it.next();

        Function* function = doc->function(showFunction->functionID());
        if (function == NULL
                || (m_showId != Function::invalidId()
                    && function->contains(m_showId)))
        {
            it.remove();
            delete showFunction;
            modified = true;
            continue;
        }

        //if (showFunction->duration() == 0)
        //    showFunction->setDuration(function->totalDuration());
        // DMX Total Control: a block without a colour keeps none: the timeline
        // colours it by the type of its function, as before it was saved (a
        // QLC+ default here changed its colour, and the file, on every open).
        // The default QLC+ gave it (shows saved before) is no colour either
        if (showFunction->color() == ShowFunction::defaultColor(function->type()))
            showFunction->setColor(QColor());

        if (function->type() == Function::SequenceType)
        {
            Sequence* sequence = qobject_cast<Sequence*>(function);
            if (sequence == NULL || getSceneID() == sequence->boundSceneID())
                continue;
#ifndef QMLUI
            if (getSceneID() == Function::invalidId())
            {
                // No scene ID, use the one from this sequence
                setSceneID(sequence->boundSceneID());
            }
            else
            {
                // Conflicting scene IDs, we have to remove this sequence
                it.remove();
                delete showFunction;
            }
#endif
            modified = true;
        }
    }
    return modified;
}

bool Track::contains(Doc* doc, quint32 functionId) const
{
    if (m_sceneID == functionId)
        return true;

    QListIterator<ShowFunction*> it(m_functions);
    while (it.hasNext())
    {
        ShowFunction* showFunction = it.next();

        Function* function = doc->function(showFunction->functionID());
        // contains() can be called during init, function may be NULL
        if (function == NULL)
            continue;

        if (function->id() == functionId)
            return true;
        if (function->contains(functionId))
            return true;
    }

    return false;
}

QList<quint32> Track::components() const
{
    QList<quint32> ids;

    QListIterator<ShowFunction*> it(m_functions);
    while (it.hasNext())
    {
        ShowFunction* showFunction = it.next();
        ids.append(showFunction->functionID());
    }

    return ids;
}

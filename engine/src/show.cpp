/*
  Q Light Controller Plus
  show.cpp

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
#include <QString>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QList>

#include "showrunner.h"
#include "function.h"
#include "show.h"
#include "doc.h"

#define KXMLQLCShowTimeDivision QStringLiteral("TimeDivision")
#define KXMLQLCShowTimeType     QStringLiteral("Type")
#define KXMLQLCShowTimeBPM      QStringLiteral("BPM")
// DMX Total Control
#define KXMLDTCShowTimeline     QStringLiteral("DMXTotalControl")

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Show::Show(Doc* doc) : Function(doc, Function::ShowType)
    , m_timeDivisionType(Time)
    , m_timeDivisionBPM(120)
    , m_endTime(0)
    , m_loop(false)
    , m_gridOffset(0)
    , m_msTimeline(false)
    , m_staticScene(Function::invalidId())
    , m_latestTrackId(0)
    , m_latestShowFunctionID(0)
    , m_runner(NULL)
{
    setName(tr("New Show"));

    // Clear attributes here. I want attributes to be mapped
    // exactly like the Show tracks
    unregisterAttribute(tr("Intensity"));

    // DMX Total Control: the functions of the running blocks deleted or
    // replaced (the runner must not keep them)
    connect(doc, SIGNAL(functionRemoved(quint32)),
            this, SLOT(slotFunctionRemoved(quint32)));
    connect(doc, SIGNAL(functionReplaced(quint32)),
            this, SLOT(slotFunctionReplaced(quint32)));
}

Show::~Show()
{
    m_tracks.clear();
}

QIcon Show::getIcon() const
{
    return QIcon(":/show.png");
}

quint32 Show::totalDuration()
{
    // DMX Total Control: the end marker
    if (m_endTime > 0)
        return m_endTime;

    quint32 totalDuration = 0;

    foreach (Track *track, m_tracks)
    {
        foreach (ShowFunction *sf, track->showFunctions())
        {
            if (sf->startTime() + sf->duration(doc()) > totalDuration)
                totalDuration = sf->startTime() + sf->duration(doc());
        }
    }

    return totalDuration;
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Show::createCopy(Doc* doc, bool addToDoc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Show(doc);
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

bool Show::copyFrom(const Function* function)
{
    const Show* show = qobject_cast<const Show*> (function);
    if (show == NULL)
        return false;

    m_timeDivisionType = show->m_timeDivisionType;
    m_timeDivisionBPM = show->m_timeDivisionBPM;
    // DMX Total Control
    m_endTime = show->m_endTime;
    m_loop = show->m_loop;
    m_gridOffset = show->m_gridOffset;
    m_msTimeline = show->m_msTimeline;
    if (Function *values = doc()->function(show->m_staticScene))
    {
        Function *copy = values->createCopy(doc());
        m_staticScene = copy != NULL ? copy->id() : Function::invalidId();
    }
    m_latestTrackId = show->m_latestTrackId;
    m_latestShowFunctionID = show->m_latestShowFunctionID;

    // DMX Total Control: the functions copied for the blocks
    QHash<quint32, quint32> copies;

    // create a copy of each track
    foreach (Track *track, show->tracks())
    {
        quint32 sceneID = track->getSceneID();
        Track* newTrack = new Track(sceneID, this);
        newTrack->setName(track->name());
        addTrack(newTrack);
        // DMX Total Control
        newTrack->setLocked(track->isLocked());
        newTrack->setHidden(track->isHidden());
        for (const QString &lane : { QString("dimmer"), QString("phase") })
            newTrack->setAutomation(lane, track->automation(lane));

        // create a copy of each sequence/audio in a track
        foreach (ShowFunction *sfunc, track->showFunctions())
        {
            Function* function = doc()->function(sfunc->functionID());
            if (function == NULL)
                continue;

            // DMX Total Control: the blocks of a super scene play the scenes
            // of the user: the copy plays the same ones. Only what belongs to
            // the show (hidden: its audio) is copied, once
            quint32 fid = Function::invalidId();
            if (m_msTimeline && function->isVisible())
            {
                fid = function->id();
            }
            else if (m_msTimeline && copies.contains(function->id()))
            {
                fid = copies.value(function->id());
            }
            else
            {
                /* Attempt to create a copy of the function to Doc */
                Function* copy = function->createCopy(doc());
                if (copy == NULL)
                    continue;
                if (m_msTimeline == false)
                    copy->setName(tr("Copy of %1").arg(function->name()));
                copies.insert(function->id(), copy->id());
                fid = copy->id();
            }

            ShowFunction *showFunc = newTrack->createShowFunction(fid);
            showFunc->setStartTime(sfunc->startTime());
            showFunc->setDuration(sfunc->duration());
            showFunc->setColor(sfunc->color());
            showFunc->setLocked(sfunc->isLocked());
            showFunc->setFadeIn(sfunc->fadeIn());
            showFunc->setFadeOut(sfunc->fadeOut());
            showFunc->setStretch(sfunc->stretch());
        }
    }

    return Function::copyFrom(function);
}

/*********************************************************************
 * Time division
 *********************************************************************/

void Show::setTimeDivision(Show::TimeDivision type, int BPM)
{
    qDebug() << "[setTimeDivision] type:" << type << ", BPM:" << BPM;
    m_timeDivisionType = type;
    m_timeDivisionBPM = BPM;
}

Show::TimeDivision Show::timeDivisionType() const
{
    return m_timeDivisionType;
}

int Show::beatsDivision() const
{
    switch(m_timeDivisionType)
    {
        case BPM_2_4: return 2;
        case BPM_3_4: return 3;
        case BPM_4_4: return 4;
        default: return 0;
    }
}

void Show::setTimeDivisionType(TimeDivision type)
{
    m_timeDivisionType = type;
}

int Show::timeDivisionBPM() const
{
    return m_timeDivisionBPM;
}

void Show::setTimeDivisionBPM(int BPM)
{
    m_timeDivisionBPM = BPM;
}

/*****************************************************************************
 * DMX Total Control: timeline settings
 *****************************************************************************/

void Show::setEndTime(quint32 ms)
{
    m_endTime = ms;
    emit changed(id());
}

quint32 Show::endTime() const
{
    return m_endTime;
}

void Show::setLoop(bool loop)
{
    m_loop = loop;
    emit changed(id());
}

bool Show::loop() const
{
    return m_loop;
}

void Show::setGridOffset(quint32 ms)
{
    m_gridOffset = ms;
    emit changed(id());
}

quint32 Show::gridOffset() const
{
    return m_gridOffset;
}

void Show::setMsTimeline(bool ms)
{
    m_msTimeline = ms;
}

bool Show::msTimeline() const
{
    return m_msTimeline;
}

void Show::setStaticScene(quint32 id)
{
    m_staticScene = id;
    emit changed(this->id());
}

quint32 Show::staticScene() const
{
    return m_staticScene;
}

QString Show::tempoToString(Show::TimeDivision type)
{
    switch(type)
    {
        case Time: return QString("Time"); break;
        case BPM_4_4: return QString("BPM_4_4"); break;
        case BPM_3_4: return QString("BPM_3_4"); break;
        case BPM_2_4: return QString("BPM_2_4"); break;
        case Invalid:
        default:
            return QString("Invalid"); break;
    }
    return QString();
}

Show::TimeDivision Show::stringToTempo(const QString& tempo)
{
    if (tempo == "Time")
        return Time;
    else if (tempo == "BPM_4_4")
        return BPM_4_4;
    else if (tempo == "BPM_3_4")
        return BPM_3_4;
    else if (tempo == "BPM_2_4")
        return BPM_2_4;
    else
        return Invalid;
}

/*****************************************************************************
 * Tracks
 *****************************************************************************/

bool Show::addTrack(Track *track, quint32 id)
{
    Q_ASSERT(track != NULL);
    // DMX Total Control: not while the runner reads the tracks
    QMutexLocker locker(&m_runnerMutex);

    // No ID given, this method can assign one
    if (id == Track::invalidId())
        id = createTrackId();

     track->setId(id);
     track->setShowId(this->id());
     m_tracks[id] = track;

     registerAttribute(QString("%1-%2").arg(track->name()).arg(track->id()));

     return true;
}

bool Show::removeTrack(quint32 id)
{
    // DMX Total Control: a running show forgets the track and its blocks
    // (what they play stops) before they are deleted
    QMutexLocker locker(&m_runnerMutex);
    if (m_tracks.contains(id) == true)
    {
        Track* track = m_tracks.take(id);
        Q_ASSERT(track != NULL);

        if (m_runner != NULL)
            m_runner->forgetTrack(track);

        unregisterAttribute(QString("%1-%2").arg(track->name()).arg(track->id()));

        //emit trackRemoved(id);
        // DMX Total Control: its blocks go with it (they were leaked)
        qDeleteAll(track->showFunctions());
        delete track;

        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "No track found with id" << id;
        return false;
    }
}

Track* Show::track(quint32 id) const
{
    return m_tracks.value(id, NULL);
}

Track* Show::getTrackFromSceneID(quint32 id) const
{
    foreach (Track *track, m_tracks)
    {
        if (track->getSceneID() == id)
            return track;
    }
    return NULL;
}

Track *Show::getTrackFromShowFunctionID(quint32 id) const
{
    foreach (Track *track, m_tracks)
        if (track->showFunction(id) != NULL)
            return track;

    return NULL;
}

int Show::getTracksCount() const
{
    return m_tracks.size();
}

void Show::moveTrack(Track *track, int direction)
{
    if (track == NULL)
        return;
    // DMX Total Control: not while the runner reads the tracks
    QMutexLocker locker(&m_runnerMutex);

    qint32 trkID = track->id();
    if (trkID == 0 && direction == -1)
        return;
    qint32 maxID = -1;
    Track *swapTrack = NULL;
    qint32 swapID = -1;
    if (direction > 0) swapID = INT_MAX;

    foreach (quint32 id, m_tracks.keys())
    {
        qint32 signedID = (qint32)id;
        if (signedID > maxID) maxID = signedID;
        if (direction == -1 && signedID > swapID && signedID < trkID)
            swapID = signedID;
        else if (direction == 1 && signedID < swapID && signedID > trkID)
            swapID = signedID;
    }

    qDebug() << Q_FUNC_INFO << "Direction:" << direction << ", trackID:" << trkID << ", swapID:" << swapID;
    if (swapID == trkID || (direction > 0 && trkID == maxID))
        return;

    swapTrack = m_tracks[swapID];
    m_tracks[swapID] = track;
    m_tracks[trkID] = swapTrack;
    track->setId(swapID);
    swapTrack->setId(trkID);
}

QList <Track*> Show::tracks() const
{
    return m_tracks.values();
}

quint32 Show::createTrackId()
{
    while (m_tracks.contains(m_latestTrackId) == true ||
           m_latestTrackId == Track::invalidId())
    {
        m_latestTrackId++;
    }

    return m_latestTrackId;
}

/*********************************************************************
 * Show Functions
 *********************************************************************/

quint32 Show::getLatestShowFunctionId()
{
    return m_latestShowFunctionID++;
}

ShowFunction *Show::showFunction(quint32 id) const
{
    foreach (Track *track, m_tracks)
    {
        ShowFunction *sf = track->showFunction(id);
        if (sf != NULL)
            return sf;
    }

    return NULL;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Show::saveXML(QXmlStreamWriter *doc) const
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    doc->writeStartElement(KXMLQLCFunction);

    /* Common attributes */
    saveXMLCommon(doc);

    doc->writeStartElement(KXMLQLCShowTimeDivision);
    doc->writeAttribute(KXMLQLCShowTimeType, tempoToString(m_timeDivisionType));
    doc->writeAttribute(KXMLQLCShowTimeBPM, QString::number(m_timeDivisionBPM));
    doc->writeEndElement();

    // DMX Total Control: the timeline settings (blocks in ms, unless Ms="0":
    // a show of QLC+ edited here keeps its end, loop and static values too)
    if (m_msTimeline || m_endTime > 0 || m_loop || m_gridOffset > 0 || m_staticScene != Function::invalidId())
    {
        doc->writeStartElement(KXMLDTCShowTimeline);
        if (m_msTimeline == false)
            doc->writeAttribute("Ms", "0");
        if (m_endTime > 0)
            doc->writeAttribute("End", QString::number(m_endTime));
        if (m_loop)
            doc->writeAttribute("Loop", "1");
        if (m_gridOffset > 0)
            doc->writeAttribute("GridOffset", QString::number(m_gridOffset));
        if (m_staticScene != Function::invalidId())
            doc->writeAttribute("Static", QString::number(m_staticScene));
        doc->writeEndElement();
    }

    foreach (Track *track, m_tracks)
        track->saveXML(doc);

    /* End the <Function> tag */
    doc->writeEndElement();

    return true;
}

bool Show::loadXML(QXmlStreamReader &root)
{
    if (root.name() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root.attributes().value(KXMLQLCFunctionType).toString() != typeToString(Function::ShowType))
    {
        qWarning() << Q_FUNC_INFO << root.attributes().value(KXMLQLCFunctionType).toString()
                   << "is not a show";
        return false;
    }

    while (root.readNextStartElement())
    {
        if (root.name() == KXMLQLCShowTimeDivision)
        {
            QString type = root.attributes().value(KXMLQLCShowTimeType).toString();
            int bpm = root.attributes().value(KXMLQLCShowTimeBPM).toString().toInt();
            setTimeDivision(stringToTempo(type), bpm);
            root.skipCurrentElement();
        }
        else if (root.name() == KXMLQLCTrack)
        {
            Track *trk = new Track(Function::invalidId(), this);
            if (trk->loadXML(root) == true)
                addTrack(trk, trk->id());
        }
        else if (root.name() == KXMLDTCShowTimeline)
        {
            // DMX Total Control
            QXmlStreamAttributes attrs = root.attributes();
            m_msTimeline = attrs.value("Ms").toString() != "0";
            m_endTime = attrs.value("End").toString().toUInt();
            m_loop = attrs.value("Loop").toString() == "1";
            m_gridOffset = attrs.value("GridOffset").toString().toUInt();
            if (attrs.hasAttribute("Static"))
                m_staticScene = attrs.value("Static").toString().toUInt();
            root.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Show tag:" << root.name();
            root.skipCurrentElement();
        }
    }

    return true;
}

void Show::postLoad()
{
    foreach (Track* track, m_tracks)
    {
        if (track->postLoad(doc()))
            doc()->setModified();
    }
}

bool Show::contains(quint32 functionId) const
{
    Doc *doc = this->doc();
    Q_ASSERT(doc != NULL);

    if (functionId == id())
        return true;
    if (functionId == m_staticScene)
        return true;

    foreach (Track* track, m_tracks)
    {
        if (track->contains(doc, functionId))
            return true;
    }

    return false;
}

QList<quint32> Show::components() const
{
    QList<quint32> ids;

    foreach (Track* track, m_tracks)
        ids.append(track->components());
    if (m_staticScene != Function::invalidId())
        ids.append(m_staticScene);

    return ids;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Show::preRun(MasterTimer* timer)
{
    Function::preRun(timer);
    m_runningChildren.clear();
    {
        // DMX Total Control: the tracks and blocks don't change meanwhile
        QMutexLocker locker(&m_runnerMutex);
        if (m_runner != NULL)
        {
            m_runner->stop();
            delete m_runner;
        }

        m_runner = new ShowRunner(doc(), this->id(), elapsed());
        int i = 0;
        foreach (Track *track, m_tracks)
            m_runner->adjustIntensity(getAttributeValue(i++), track);

        connect(m_runner, SIGNAL(timeChanged(quint32)), this, SIGNAL(timeChanged(quint32)));
        connect(m_runner, SIGNAL(showFinished()), this, SIGNAL(showFinished()));
        m_runner->start();
    }

    // DMX Total Control: the static values of the show play with it (not
    // when it was stopped before its first frame: then it plays nothing)
    if (Function *values = stopped() ? nullptr : doc()->function(m_staticScene))
        values->start(timer, FunctionParent(FunctionParent::Function, id()));
}

void Show::setPause(bool enable)
{
    {
        QMutexLocker locker(&m_runnerMutex);
        if (m_runner != NULL)
            m_runner->setPause(enable);
    }
    Function::setPause(enable);
}

void Show::write(MasterTimer* timer, QList<Universe *> universes)
{
    Q_UNUSED(universes);

    if (isPaused())
        return;

    // DMX Total Control: the blocks and tracks it plays stay meanwhile
    QMutexLocker locker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->write(timer);
}

void Show::postRun(MasterTimer* timer, QList<Universe *> universes)
{
    {
        QMutexLocker locker(&m_runnerMutex);
        if (m_runner != NULL)
        {
            m_runner->stop();
            delete m_runner;
            m_runner = NULL;
        }
    }
    // DMX Total Control
    if (Function *values = doc()->function(m_staticScene))
        values->stop(FunctionParent(FunctionParent::Function, id()));
    Function::postRun(timer, universes);
}

QRecursiveMutex *Show::runnerMutex()
{
    return &m_runnerMutex;
}

void Show::forgetBlock(ShowFunction *sf)
{
    // DMX Total Control
    QMutexLocker locker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->forgetBlock(sf);
}

void Show::slotChildStopped(quint32 fid)
{
    Q_UNUSED(fid);
}

void Show::slotFunctionRemoved(quint32 fid)
{
    // DMX Total Control
    QMutexLocker locker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->forgetFunction(fid, NULL);
}

void Show::slotFunctionReplaced(quint32 fid)
{
    // DMX Total Control: the Doc already has the new one
    QMutexLocker locker(&m_runnerMutex);
    if (m_runner != NULL)
        m_runner->forgetFunction(fid, doc()->function(fid));
}

/*****************************************************************************
 * Attributes
 *****************************************************************************/

int Show::adjustAttribute(qreal fraction, int attributeId)
{
    int attrIndex = Function::adjustAttribute(fraction, attributeId);

    // DMX Total Control: the runner may be going away in the timer thread
    QMutexLocker locker(&m_runnerMutex);
    if (m_runner != NULL)
    {
        QList<Track*> trkList = m_tracks.values();
        if (trkList.isEmpty() == false &&
            attrIndex >= 0 && attrIndex < trkList.count())
        {
            Track *track = trkList.at(attrIndex);
            if (track != NULL)
                m_runner->adjustIntensity(getAttributeValue(attrIndex), track);
        }
    }

    return attrIndex;
}


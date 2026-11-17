/*
  DMX Total Control
  liverecall.h

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

#ifndef LIVERECALL_H
#define LIVERECALL_H

#include <QObject>
#include <QVariant>
#include <QElapsedTimer>
#include <QList>

class DtcCore;
class QTimer;

/**
 * Stop all undone: what played when Stop all (or «Release groups») stopped
 * it can be brought back for some seconds (the toast, Ctrl+Z, a mapping, a
 * SHOW button, the phone): the scene each group played, the cuelists on
 * their cue, the scenes started by a button or a playback, at the levels
 * their playbacks have. Not what a flash held (it plays only while held),
 * nor what a fader of the SHOW holds up (never stopped), nor the
 * programmer, the mixer, the arming of the special effects, the parks or
 * the show stop. Brought back with a short fade; FX and steps start their
 * cycle again. Once. It never fires a protected special effect: armed, they
 * are disarmed first, and what comes back with one waits for a release.
 */
class LiveRecall : public QObject
{
    Q_OBJECT

    /** What was stopped can be brought back now */
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    /** What it would bring back: «4 scenes and 1 cuelist» */
    Q_PROPERTY(QString summary READ summary NOTIFY availableChanged)
    /** What stopped it: "stopAll" or "releaseGroups" */
    Q_PROPERTY(QString reason READ reason NOTIFY availableChanged)
    /** How long it can be brought back after each stop, ms */
    Q_PROPERTY(int windowMs READ windowMs CONSTANT)

public:
    /** The fade of what comes back, ms */
    static constexpr uint kRecallFadeMs = 500;

    /** What played when it was stopped */
    struct Snapshot
    {
        qint64 takenAt = 0;
        QString reason;
        /** a scene (not a cuelist) a group played */
        struct ZoneScene
        {
            quint32 zone;
            quint32 fid;
        };
        QList<ZoneScene> zoneScenes;
        /** a function of no group (not a cuelist): from a button, the
         *  editors (master), or a playback at its level */
        struct Run
        {
            quint32 fid;
            bool playback;
            qreal level;
        };
        QList<Run> functions;
        /** a cuelist on its cue (zone -1: of no group) */
        struct List
        {
            quint32 fid;
            quint32 cueUid;
            bool paused;
            int zone;
            bool playback;
            qreal level;
        };
        QList<List> cuelists;

        bool isEmpty() const { return zoneScenes.isEmpty() && functions.isEmpty() && cuelists.isEmpty(); }
        /** Another one taken in the window joins it (each function once) */
        void merge(const Snapshot &later);
        /** As a map (the tests) */
        QVariantMap toVariant() const;
    };

    explicit LiveRecall(DtcCore *core, QObject *parent = nullptr);
    ~LiveRecall();

    bool available() const { return m_available; }
    QString summary() const;
    QString reason() const { return m_snapshot.reason; }
    int windowMs() const { return m_windowMs; }
    /** What is left of the window, ms (0 when nothing can be brought back) */
    Q_INVOKABLE int msLeft() const;
    /** One more at each take (a second Stop all in the window too) */
    int serial() const { return m_serial; }

    /** Before Stop all ($zonesOnly false) or «Release groups» ($zonesOnly:
     *  only the groups): what plays. A second one in the window joins the
     *  first one; the window starts again */
    void take(const QString &reason, bool zonesOnly);
    /** What was stopped plays again (once): false when there is nothing.
     *  The special effects armed: disarmed first ("recall") */
    Q_INVOKABLE bool recall();
    /** Nothing to bring back any more (a new show, the window over) */
    void clear();

    /** The snapshot it would bring back (tests) */
    QVariantMap snapshot() const { return m_snapshot.toVariant(); }
    /** (tests) the window, ms */
    void setWindowMsForTest(int ms) { m_windowMs = ms; }

    /** «4 scenes and 1 cuelist» */
    static QString describe(int scenes, int cuelists);

signals:
    void availableChanged();
    /** A snapshot was taken (or joined): the countdown starts again */
    void taken();
    /** Something to tell the user */
    void message(const QString &text);
    /** What was stopped was brought back (recall()) */
    void recalled();

private slots:
    void slotDocCleared();

private:
    DtcCore *m_core;
    Snapshot m_snapshot;
    bool m_available = false;
    int m_serial = 0;
    int m_windowMs = 10000;
    QTimer *m_window;
    QElapsedTimer m_since;
};

#endif

/*
  DMX Total Control
  dtccore.h

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

#ifndef DTCCORE_H
#define DTCCORE_H

#include <QObject>
#include <QVariant>
#include <QUrl>
#include <QElapsedTimer>

// complete types are needed by the QML properties below
#include "patchmanager.h"
#include "zonemanager.h"
#include "programmer.h"
#include "iomanager.h"
#include "appsettings.h"
#include "fixturelibrary.h"
#include "effecteditor.h"
#include "timelineeditor.h"
#include "fxtools.h"
#include "remoteserver.h"
#include "updatechecker.h"
#include "showsurface.h"
#include "drivecontrol.h"
#include "livemixer.h"
#include "controlmapper.h"
#include "palettemanager.h"
#include "commandline.h"
#include "cuelisteditor.h"
#include "playbackmanager.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QTimer;
class Doc;

#define KXMLDTCWorkspace QStringLiteral("Workspace")
#define KXMLDTCSection   QStringLiteral("DMXTotalControl")
#define KXMLDTCFolders   QStringLiteral("Folders")
#define KXMLDTCFolder    QStringLiteral("Folder")

/**
 * The application core: owns the engine (Doc) and the managers the
 * QML UI talks to. Shows are saved as QLC+ workspaces (.qxw) with an extra
 * section for DMX Total Control specific data, so the engine part stays
 * readable by QLC+.
 */
class DtcCore : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(QString showTitle READ showTitle NOTIFY fileNameChanged)
    Q_PROPERTY(bool modified READ modified NOTIFY modifiedChanged)
    Q_PROPERTY(bool blackout READ blackout WRITE setBlackout NOTIFY blackoutChanged)
    Q_PROPERTY(int grandMaster READ grandMaster WRITE setGrandMaster NOTIFY grandMasterChanged)
    Q_PROPERTY(int runningFunctions READ runningFunctions NOTIFY runningFunctionsChanged)
    Q_PROPERTY(int bpm READ bpm WRITE setBpm NOTIFY bpmChanged)
    /** How the buttons of the scenes act: "toggle" (as always), "go" or "stop" */
    Q_PROPERTY(QString pressMode READ pressMode WRITE setPressMode NOTIFY pressModeChanged)
    /** Changes when a scene with steps that plays moves to another step (stepOf) */
    Q_PROPERTY(int stepsRevision READ stepsRevision NOTIFY stepsChanged)
    /** The processor this application uses, in percent of the whole machine
     *  (all of its cores), looked at every two seconds. -1: unknown */
    Q_PROPERTY(int cpuLoad READ cpuLoad NOTIFY cpuLoadChanged)
    Q_PROPERTY(QVariantList functions READ functions NOTIFY functionsChanged)
    Q_PROPERTY(QStringList sceneFolders READ sceneFolders NOTIFY functionsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

    Q_PROPERTY(ZoneManager *zones READ zones CONSTANT)
    Q_PROPERTY(PatchManager *patch READ patch CONSTANT)
    Q_PROPERTY(Programmer *programmer READ programmer CONSTANT)
    Q_PROPERTY(IOManager *io READ io CONSTANT)
    Q_PROPERTY(AppSettings *settings READ settings CONSTANT)
    Q_PROPERTY(FixtureLibrary *library READ library CONSTANT)
    Q_PROPERTY(EffectEditor *effects READ effects CONSTANT)
    Q_PROPERTY(TimelineEditor *timeline READ timeline CONSTANT)
    Q_PROPERTY(FxMacros *fxMacros READ fxMacros CONSTANT)
    Q_PROPERTY(RemoteServer *remote READ remote CONSTANT)
    Q_PROPERTY(UpdateChecker *updates READ updates CONSTANT)
    Q_PROPERTY(ShowSurface *surface READ surface CONSTANT)
    Q_PROPERTY(DriveControl *drive READ drive CONSTANT)
    Q_PROPERTY(LiveMixer *mixer READ mixer CONSTANT)
    Q_PROPERTY(ControlMapper *mapping READ mapping CONSTANT)
    Q_PROPERTY(PaletteManager *palettes READ palettes CONSTANT)
    Q_PROPERTY(CommandLine *commands READ commands CONSTANT)
    Q_PROPERTY(CuelistEditor *cuelists READ cuelists CONSTANT)
    Q_PROPERTY(PlaybackManager *playbacks READ playbacks CONSTANT)
    /** An autosaved, never saved show from a session that crashed can be recovered */
    Q_PROPERTY(bool hasRecovery READ hasRecovery NOTIFY hasRecoveryChanged)
    Q_PROPERTY(QString lastAutosave READ lastAutosave NOTIFY lastAutosaveChanged)
    /** The files the open show names that are not there: its sounds, videos
     *  and pictures (paths of this machine) */
    Q_PROPERTY(QStringList missingFiles READ missingFiles NOTIFY missingFilesChanged)

public:
    explicit DtcCore(QObject *parent = nullptr);
    ~DtcCore();

    Doc *doc() const;
    ZoneManager *zones() const;
    PatchManager *patch() const;
    Programmer *programmer() const;
    IOManager *io() const;
    AppSettings *settings() const;
    FixtureLibrary *library() const;
    EffectEditor *effects() const;
    TimelineEditor *timeline() const;
    FxMacros *fxMacros() const;
    RemoteServer *remote() const;
    UpdateChecker *updates() const;
    ShowSurface *surface() const;
    DriveControl *drive() const;
    LiveMixer *mixer() const;
    ControlMapper *mapping() const;
    PaletteManager *palettes() const;
    CommandLine *commands() const;
    CuelistEditor *cuelists() const;
    PlaybackManager *playbacks() const;

    QString appName() const;
    QString appVersion() const;

    QString fileName() const;
    QString showTitle() const;
    bool modified() const;
    QString lastError() const;

    bool blackout() const;
    void setBlackout(bool enable);

    /** Grand master, 0 - 255 */
    int grandMaster() const;
    void setGrandMaster(int value);

    int runningFunctions() const;

    /** Project tempo, used by the scenes that follow the BPM */
    int bpm() const;
    void setBpm(int bpm);
    /** Tap tempo: the BPM follows the spacing of the taps */
    Q_INVOKABLE void tapTempo();

    /** The buttons of the scenes (Live, SHOW, mappings, the phone): "toggle"
     *  (a press starts or stops it; presses in a row of one with steps mark
     *  its rhythm), "go" (a press starts it, the next ones mark its rhythm)
     *  or "stop" (a press stops it). Not saved: "toggle" when it opens */
    QString pressMode() const;
    void setPressMode(const QString &mode);
    /** The step (or cue) each scene with steps that plays is on: the rhythm
     *  bar of its buttons alternates when it moves. -1: it doesn't play, or
     *  has no steps. stepsChanged when one moves, starts or stops */
    Q_INVOKABLE int stepOf(quint32 id) const;
    int stepsRevision() const;
    int cpuLoad() const;
    /** The same for every one: ID -> step (the remote) */
    QHash<quint32, int> steps() const;

    /** [{ id, name, type }] */
    QVariantList functions() const;

    Q_INVOKABLE void newShow();
    Q_INVOKABLE bool openShow(const QUrl &url);
    Q_INVOKABLE bool saveShow(const QUrl &url = QUrl());
    /** Bring the patch and/or the scenes (with their groups) of another show
     *  into this one. The scenes go on the fixtures of this show that match
     *  theirs. @return what was imported, empty on error (see lastError) */
    Q_INVOKABLE QString importFromShow(const QUrl &url, bool patch, bool scenes);

    /** Open the last show of the previous session, if the settings ask for it */
    void openStartupShow();

    bool hasRecovery() const;
    Q_INVOKABLE bool recoverShow();
    Q_INVOKABLE void discardRecovery();
    QString lastAutosave() const;

    QStringList missingFiles() const;
    /** Look for the missing files by their name in $folder and below: the
     *  show uses the ones found. @return how many were found */
    Q_INVOKABLE int locateMissingFiles(const QUrl &folder);

    /** Stop every running function (panic) */
    Q_INVOKABLE void stopAll();
    /** Before the program closes: everything stops and lets its channels
     *  go, and the outputs get to send the stage at rest */
    void releaseOutput();

    Q_INVOKABLE void renameFunction(quint32 id, const QString &name);
    Q_INVOKABLE void deleteFunction(quint32 id);
    /** Set the fade in/out times, in ms, of a function */
    Q_INVOKABLE void setFunctionFades(quint32 id, int fadeIn, int fadeOut);
    /** Playback priority, 0 - 10 */
    Q_INVOKABLE void setFunctionPriority(quint32 id, int priority);
    Q_INVOKABLE QVariantMap functionInfo(quint32 id) const;

    /** Folders to sort the scenes: the ones created here (even empty)
     *  and the ones the scenes use (the function path), by name */
    QStringList sceneFolders() const;
    /** Create an empty folder. False if the name is empty or taken */
    Q_INVOKABLE bool addSceneFolder(const QString &name);
    /** Rename a folder, moving its scenes. Into an existing one, they merge */
    Q_INVOKABLE bool renameSceneFolder(const QString &name, const QString &newName);
    /** Remove a folder: its scenes stay, out of any folder */
    Q_INVOKABLE void removeSceneFolder(const QString &name);
    /** Put scenes in a folder ("" = no folder) */
    Q_INVOKABLE void setFunctionFolder(const QVariantList &ids, const QString &folder);

    /** Put a text on the clipboard */
    Q_INVOKABLE void copyToClipboard(const QString &text);

    /** A page of the manual (docs/manual, built in): its Markdown text, or
     *  an empty string. $name: "programar", "index"... */
    Q_INVOKABLE QString manualPage(const QString &name) const;
    /** The same page as HTML for the help panel: its pictures $width wide */
    Q_INVOKABLE QString manualHtml(const QString &name, int width) const;
    /** The language the manual is shown in ("en", "de"...): the one of the app
     *  where it is translated; empty: the original, in Spanish */
    Q_INVOKABLE QString manualLanguage() const;
    /** The language of the translation of the interface that was loaded
     *  ("es", "de_DE"...; empty: none, the interface is in English). The
     *  manual follows it. Not set: the language of the locale */
    void setUiLanguage(const QString &language);
    /** The example shows installed with the program: [{ name, url }] */
    Q_INVOKABLE QVariantList examples() const;

    /** The URL of a local file (a path with '#' or '%' stays a path) */
    Q_INVOKABLE QUrl localFileUrl(const QString &path) const;

    /** Write a control property from C++, so its QML binding is kept (sliders: keys, double click) */
    Q_INVOKABLE void setControlValue(QObject *control, qreal value);

signals:
    void fileNameChanged();
    void modifiedChanged();
    void blackoutChanged();
    void grandMasterChanged();
    void runningFunctionsChanged();
    void functionsChanged();
    void lastErrorChanged();
    void bpmChanged();
    void pressModeChanged();
    void stepsChanged();
    void cpuLoadChanged();
    /** Emitted on every beat, to flash the BPM indicator */
    void beat();
    void hasRecoveryChanged();
    void lastAutosaveChanged();
    void missingFilesChanged();
    /** An autosave couldn't write the show: nothing protects it */
    void autosaveFailed(const QString &error);

private slots:
    void slotStepsTick();
    void slotCpuTick();
    void slotAutosave();
    void slotAutosaveSettingsChanged();
    void slotGrandMasterModeChanged();
    void slotLiveEditChanged();
    void slotFunctionRemoved(quint32 id);

private:
    void initDoc();
    void clearShow();
    /** Look at which files of the show are missing (missingFiles) */
    void checkFiles();
    bool loadWorkspace(QXmlStreamReader &reader);
    void loadSection(QXmlStreamReader &reader);
    /** The current element of $reader as it is (QLC+ sections kept) */
    static QByteArray copyElement(QXmlStreamReader &reader);
    static void copyToken(QXmlStreamReader &reader, QXmlStreamWriter &writer);
    void setFileName(const QString &fileName);
    void setLastError(const QString &error);
    bool writeWorkspace(const QString &path);
    QString recoveryFilePath() const;

private:
    Doc *m_doc;
    ZoneManager *m_zones;
    PatchManager *m_patch;
    Programmer *m_programmer;
    IOManager *m_io;
    AppSettings *m_settings;
    FixtureLibrary *m_library;
    EffectEditor *m_effects;
    TimelineEditor *m_timeline;
    FxMacros *m_fxMacros;
    RemoteServer *m_remote;
    UpdateChecker *m_updates;
    ShowSurface *m_surface;
    DriveControl *m_drive;
    LiveMixer *m_mixer;
    ControlMapper *m_mapping;
    PaletteManager *m_palettes;
    CommandLine *m_commands;
    CuelistEditor *m_cuelists;
    PlaybackManager *m_playbacks;
    QTimer *m_autosaveTimer;
    /** The steps of the scenes with steps that play, looked at every frame or so */
    QTimer *m_stepsTimer = nullptr;
    QHash<quint32, int> m_steps;
    int m_stepsRevision = 0;
    QTimer *m_cpuTimer = nullptr;
    QElapsedTimer m_cpuClock;
    qint64 m_cpuTime = -1;
    int m_cpuLoad = -1;
    bool m_hasRecovery;
    QStringList m_missingFiles;
    /** See setUiLanguage(): null until it is set */
    QString m_uiLanguage;
    bool m_backupDone;
    /** The show is being cleared (its functions all go) */
    bool m_clearing;
    QString m_lastAutosave;
    /** The sections of the workspace this application doesn't use (QLC+
     *  Virtual Console, Simple Desk...), written back as they were read */
    QList<QByteArray> m_foreignSections;
    QElapsedTimer m_tapTimer;
    QList<qint64> m_tapIntervals;
    /** Folders created by the user, so the empty ones are kept */
    QStringList m_folders;

    QString m_fileName;
    QString m_lastError;
};

#endif

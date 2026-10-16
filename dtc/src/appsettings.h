/*
  DMX Total Control
  appsettings.h

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

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QStringList>

/**
 * User preferences, persisted with QSettings
 */
class AppSettings : public QObject
{
    Q_OBJECT

    /** UI language, e.g. "es_ES". Empty follows the system. Needs a restart */
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(bool autosaveEnabled READ autosaveEnabled WRITE setAutosaveEnabled NOTIFY autosaveChanged)
    Q_PROPERTY(int autosaveMinutes READ autosaveMinutes WRITE setAutosaveMinutes NOTIFY autosaveChanged)
    Q_PROPERTY(bool openLastShow READ openLastShow WRITE setOpenLastShow NOTIFY openLastShowChanged)
    /** Grand master acting on intensity channels only (true) or on every channel */
    Q_PROPERTY(bool grandMasterIntensityOnly READ grandMasterIntensityOnly WRITE setGrandMasterIntensityOnly NOTIFY grandMasterModeChanged)
    Q_PROPERTY(int defaultPriority READ defaultPriority WRITE setDefaultPriority NOTIFY defaultPriorityChanged)
    Q_PROPERTY(QStringList recentShows READ recentShows NOTIFY recentShowsChanged)
    /** Live edits (the programmer) beat the scenes of this priority and below */
    Q_PROPERTY(int liveEditPriority READ liveEditPriority WRITE setLiveEditPriority NOTIFY liveEditChanged)
    /** When a scene starts: "keep" the live edits, release the "channels" it uses, or reset "all" */
    Q_PROPERTY(QString liveEditRelease READ liveEditRelease WRITE setLiveEditRelease NOTIFY liveEditChanged)
    /** Favourite colours of the colour picker */
    Q_PROPERTY(QStringList favoriteColors READ favoriteColors WRITE setFavoriteColors NOTIFY favoriteColorsChanged)
    /** Open the SHOW (Touch) full screen at startup, and locked (kiosk) */
    Q_PROPERTY(bool startShowFullScreen READ startShowFullScreen WRITE setStartShowFullScreen NOTIFY startupChanged)
    Q_PROPERTY(bool startShowLocked READ startShowLocked WRITE setStartShowLocked NOTIFY startupChanged)
    /** Lock: the screens that need the password ("setup", "program", "show") */
    Q_PROPERTY(QStringList lockedScreens READ lockedScreens WRITE setLockedScreens NOTIFY lockChanged)
    Q_PROPERTY(bool hasLockPassword READ hasLockPassword NOTIFY lockChanged)
    /** The programmer is cleared after recording a cue or a new cuelist */
    Q_PROPERTY(bool clearAfterRecord READ clearAfterRecord WRITE setClearAfterRecord NOTIFY clearAfterRecordChanged)

public:
    explicit AppSettings(QObject *parent = nullptr);

    QString language() const;
    void setLanguage(const QString &language);

    bool autosaveEnabled() const;
    void setAutosaveEnabled(bool enable);

    int autosaveMinutes() const;
    void setAutosaveMinutes(int minutes);

    bool openLastShow() const;
    void setOpenLastShow(bool enable);

    bool grandMasterIntensityOnly() const;
    void setGrandMasterIntensityOnly(bool enable);

    int defaultPriority() const;
    void setDefaultPriority(int priority);

    int liveEditPriority() const;
    void setLiveEditPriority(int priority);
    QString liveEditRelease() const;
    void setLiveEditRelease(const QString &mode);
    QStringList favoriteColors() const;
    void setFavoriteColors(const QStringList &colors);
    Q_INVOKABLE void addFavoriteColor(const QString &color);
    Q_INVOKABLE void removeFavoriteColor(const QString &color);

    QStringList recentShows() const;
    Q_INVOKABLE void addRecentShow(const QString &path);
    Q_INVOKABLE void clearRecentShows();

    bool startShowFullScreen() const;
    void setStartShowFullScreen(bool enable);
    bool startShowLocked() const;
    void setStartShowLocked(bool enable);

    QStringList lockedScreens() const;
    void setLockedScreens(const QStringList &screens);
    bool hasLockPassword() const;
    /** An empty password removes it */
    Q_INVOKABLE void setLockPassword(const QString &password);
    /** True without a password, or when it is the right one */
    Q_INVOKABLE bool checkLockPassword(const QString &password) const;

    bool clearAfterRecord() const;
    void setClearAfterRecord(bool enable);

    /** Set while the application runs: still set at startup means a crash */
    bool sessionRunning() const;
    void setSessionRunning(bool running);

signals:
    void languageChanged();
    void autosaveChanged();
    void openLastShowChanged();
    void grandMasterModeChanged();
    void defaultPriorityChanged();
    void liveEditChanged();
    void favoriteColorsChanged();
    void recentShowsChanged();
    void startupChanged();
    void lockChanged();
    void clearAfterRecordChanged();

private:
    QSettings m_settings;
};

#endif

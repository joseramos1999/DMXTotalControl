/*
  DMX Total Control
  appsettings.cpp

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

#include <QColor>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QRandomGenerator>

#include "appsettings.h"
#include "qlcconfig.h"

#define KSettingLanguage        QStringLiteral("ui/language")
#define KSettingAutosave        QStringLiteral("autosave/enabled")
#define KSettingAutosaveMinutes QStringLiteral("autosave/minutes")
#define KSettingOpenLast        QStringLiteral("startup/openLastShow")
#define KSettingGMIntensity     QStringLiteral("output/grandMasterIntensityOnly")
#define KSettingPriority        QStringLiteral("playback/defaultPriority")
#define KSettingLivePriority    QStringLiteral("playback/liveEditPriority")
#define KSettingLiveRelease     QStringLiteral("playback/liveEditRelease")
#define KSettingFavoriteColors  QStringLiteral("programmer/favoriteColors")
#define KSettingRecent          QStringLiteral("files/recent")
#define KSettingSession         QStringLiteral("session/running")
#define KSettingStartShowFull   QStringLiteral("startup/showFullScreen")
#define KSettingStartShowLocked QStringLiteral("startup/showLocked")
#define KSettingLockedScreens   QStringLiteral("lock/screens")
#define KSettingLockPassword    QStringLiteral("lock/password")
#define KSettingClearAfterRecord QStringLiteral("programmer/clearAfterRecord")
// (the key the audio capture of the engine reads)
#define KSettingAudioInput      QStringLiteral("audio/input")
#define KSettingTheme           QStringLiteral("ui/theme")
#define KSettingUiScale         QStringLiteral("ui/scale")
#define KSettingGuideShown      QStringLiteral("ui/guideShown")

#define MAX_RECENT_SHOWS 8

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
{
}

QString AppSettings::language() const
{
    return m_settings.value(KSettingLanguage).toString();
}

void AppSettings::setLanguage(const QString &language)
{
    if (language == this->language())
        return;
    m_settings.setValue(KSettingLanguage, language);
    emit languageChanged();
}

bool AppSettings::autosaveEnabled() const
{
    return m_settings.value(KSettingAutosave, true).toBool();
}

void AppSettings::setAutosaveEnabled(bool enable)
{
    if (enable == autosaveEnabled())
        return;
    m_settings.setValue(KSettingAutosave, enable);
    emit autosaveChanged();
}

int AppSettings::autosaveMinutes() const
{
    return qBound(1, m_settings.value(KSettingAutosaveMinutes, 2).toInt(), 60);
}

void AppSettings::setAutosaveMinutes(int minutes)
{
    minutes = qBound(1, minutes, 60);
    if (minutes == autosaveMinutes())
        return;
    m_settings.setValue(KSettingAutosaveMinutes, minutes);
    emit autosaveChanged();
}

bool AppSettings::openLastShow() const
{
    return m_settings.value(KSettingOpenLast, true).toBool();
}

void AppSettings::setOpenLastShow(bool enable)
{
    if (enable == openLastShow())
        return;
    m_settings.setValue(KSettingOpenLast, enable);
    emit openLastShowChanged();
}

bool AppSettings::grandMasterIntensityOnly() const
{
    return m_settings.value(KSettingGMIntensity, true).toBool();
}

void AppSettings::setGrandMasterIntensityOnly(bool enable)
{
    if (enable == grandMasterIntensityOnly())
        return;
    m_settings.setValue(KSettingGMIntensity, enable);
    emit grandMasterModeChanged();
}

int AppSettings::defaultPriority() const
{
    return qBound(0, m_settings.value(KSettingPriority, 5).toInt(), 10);
}

void AppSettings::setDefaultPriority(int priority)
{
    priority = qBound(0, priority, 10);
    if (priority == defaultPriority())
        return;
    m_settings.setValue(KSettingPriority, priority);
    emit defaultPriorityChanged();
}

QStringList AppSettings::recentShows() const
{
    QStringList list;
    // forget the shows that have been moved or deleted
    for (const QString &path : m_settings.value(KSettingRecent).toStringList())
    {
        if (QFileInfo::exists(path))
            list.append(path);
    }
    return list;
}

void AppSettings::addRecentShow(const QString &path)
{
    if (path.isEmpty())
        return;

    QStringList list = m_settings.value(KSettingRecent).toStringList();
    list.removeAll(path);
    list.prepend(path);
    while (list.count() > MAX_RECENT_SHOWS)
        list.removeLast();

    m_settings.setValue(KSettingRecent, list);
    emit recentShowsChanged();
}

void AppSettings::clearRecentShows()
{
    m_settings.remove(KSettingRecent);
    emit recentShowsChanged();
}

bool AppSettings::sessionRunning() const
{
    return m_settings.value(KSettingSession, false).toBool();
}

void AppSettings::setSessionRunning(bool running)
{
    m_settings.setValue(KSettingSession, running);
    m_settings.sync();
}

int AppSettings::liveEditPriority() const
{
    return qBound(0, m_settings.value(KSettingLivePriority, 10).toInt(), 10);
}

void AppSettings::setLiveEditPriority(int priority)
{
    priority = qBound(0, priority, 10);
    if (priority == liveEditPriority())
        return;
    m_settings.setValue(KSettingLivePriority, priority);
    emit liveEditChanged();
}

QString AppSettings::liveEditRelease() const
{
    QString mode = m_settings.value(KSettingLiveRelease, "channels").toString();
    return mode == "keep" || mode == "all" ? mode : QString("channels");
}

void AppSettings::setLiveEditRelease(const QString &mode)
{
    if (mode == liveEditRelease())
        return;
    m_settings.setValue(KSettingLiveRelease, mode);
    emit liveEditChanged();
}

QStringList AppSettings::favoriteColors() const
{
    return m_settings.value(KSettingFavoriteColors).toStringList();
}

void AppSettings::setFavoriteColors(const QStringList &colors)
{
    if (colors == favoriteColors())
        return;
    m_settings.setValue(KSettingFavoriteColors, colors);
    emit favoriteColorsChanged();
}

void AppSettings::addFavoriteColor(const QString &color)
{
    QStringList colors = favoriteColors();
    QString name = QColor(color).name();
    colors.removeAll(name);
    colors.prepend(name);
    while (colors.count() > 24)
        colors.removeLast();
    setFavoriteColors(colors);
}

void AppSettings::removeFavoriteColor(const QString &color)
{
    QStringList colors = favoriteColors();
    colors.removeAll(QColor(color).name());
    setFavoriteColors(colors);
}

/*********************************************************************
 * Startup and lock
 *********************************************************************/

bool AppSettings::startShowFullScreen() const
{
    return m_settings.value(KSettingStartShowFull, false).toBool();
}

void AppSettings::setStartShowFullScreen(bool enable)
{
    if (enable == startShowFullScreen())
        return;
    m_settings.setValue(KSettingStartShowFull, enable);
    emit startupChanged();
}

bool AppSettings::startShowLocked() const
{
    return m_settings.value(KSettingStartShowLocked, false).toBool();
}

void AppSettings::setStartShowLocked(bool enable)
{
    if (enable == startShowLocked())
        return;
    m_settings.setValue(KSettingStartShowLocked, enable);
    emit startupChanged();
}

QStringList AppSettings::lockedScreens() const
{
    return m_settings.value(KSettingLockedScreens).toStringList();
}

void AppSettings::setLockedScreens(const QStringList &screens)
{
    if (screens == lockedScreens())
        return;
    m_settings.setValue(KSettingLockedScreens, screens);
    emit lockChanged();
}

bool AppSettings::hasLockPassword() const
{
    return m_settings.value(KSettingLockPassword).toString().isEmpty() == false;
}

static QString passwordHash(const QString &salt, const QString &password)
{
    return QString::fromLatin1(QCryptographicHash::hash((salt + password).toUtf8(), QCryptographicHash::Sha256).toHex());
}

void AppSettings::setLockPassword(const QString &password)
{
    if (password.isEmpty())
    {
        m_settings.remove(KSettingLockPassword);
    }
    else
    {
        // salt:hash, never the password itself
        QString salt = QString::number(QRandomGenerator::global()->generate64(), 36);
        m_settings.setValue(KSettingLockPassword, salt + ":" + passwordHash(salt, password));
    }
    emit lockChanged();
}

bool AppSettings::checkLockPassword(const QString &password) const
{
    const QString stored = m_settings.value(KSettingLockPassword).toString();
    if (stored.isEmpty())
        return true;
    const QString salt = stored.section(':', 0, 0);
    return stored.section(':', 1) == passwordHash(salt, password);
}

bool AppSettings::clearAfterRecord() const
{
    return m_settings.value(KSettingClearAfterRecord, true).toBool();
}

void AppSettings::setClearAfterRecord(bool enable)
{
    if (enable == clearAfterRecord())
        return;
    m_settings.setValue(KSettingClearAfterRecord, enable);
    emit clearAfterRecordChanged();
}

QString AppSettings::audioInput() const
{
    return m_settings.value(KSettingAudioInput).toString();
}

void AppSettings::setAudioInput(const QString &name)
{
    if (name == audioInput())
        return;
    if (name.isEmpty())
        m_settings.remove(KSettingAudioInput);
    else
        m_settings.setValue(KSettingAudioInput, name);
    emit audioInputChanged();
}

/*********************************************************************
 * Look
 *********************************************************************/

QStringList AppSettings::themes()
{
    return { QStringLiteral("dark"), QStringLiteral("light"), QStringLiteral("contrast"), QStringLiteral("night") };
}

QString AppSettings::theme() const
{
    if (m_themeOverride.isEmpty() == false)
        return m_themeOverride;
    const QString theme = m_settings.value(KSettingTheme).toString();
    return themes().contains(theme) ? theme : QStringLiteral("dark");
}

void AppSettings::setTheme(const QString &theme)
{
    if (themes().contains(theme) == false)
        return;
    const QString before = this->theme();
    m_themeOverride.clear();
    m_settings.setValue(KSettingTheme, theme);
    if (theme != before)
        emit themeChanged();
}

void AppSettings::setThemeOverride(const QString &theme)
{
    if (themes().contains(theme) == false)
        return;
    const QString before = this->theme();
    m_themeOverride = theme;
    if (theme != before)
        emit themeChanged();
}

int AppSettings::uiScale() const
{
    return qBound(80, m_settings.value(KSettingUiScale, 100).toInt(), 150);
}

void AppSettings::setUiScale(int percent)
{
    percent = qBound(80, percent, 150);
    if (percent == uiScale())
        return;
    m_settings.setValue(KSettingUiScale, percent);
    emit uiScaleChanged();
}

int AppSettings::savedUiScale()
{
    // (main reads it before the application, and its names, exist)
    QSettings settings(QStringLiteral("DMXTotalControl"), QStringLiteral(APPNAME));
    return qBound(80, settings.value(KSettingUiScale, 100).toInt(), 150);
}

bool AppSettings::guideShown() const
{
    return m_settings.value(KSettingGuideShown, false).toBool();
}

void AppSettings::setGuideShown(bool shown)
{
    if (shown == guideShown())
        return;
    m_settings.setValue(KSettingGuideShown, shown);
    emit guideShownChanged();
}

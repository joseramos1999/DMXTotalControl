/*
  DMX Total Control
  updatechecker.h

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

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QCryptographicHash>
#include <QObject>
#include <QPointer>
#include <QFile>
#include <optional>

class QNetworkAccessManager;
class QNetworkReply;
class QJsonDocument;

/**
 * Updates from the GitHub releases of the project: compares the newest
 * release with this build, downloads its installer, checks it against the
 * SHA-256 digest GitHub publishes for it and runs it silently (now, opening
 * the program again, or when the program closes).
 *
 * It only downloads from the releases of its own repository, and never a
 * file without a well-formed digest: such a release is announced, with its
 * page, but not installed. For tests, setApiUrl() points it at another
 * server (and lifts the rule of the repository).
 */
class UpdateChecker : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int state READ state NOTIFY changed)
    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(QString latestVersion READ latestVersion NOTIFY changed)
    Q_PROPERTY(QString notes READ notes NOTIFY changed)
    Q_PROPERTY(QString pageUrl READ pageUrl NOTIFY changed)
    Q_PROPERTY(bool prerelease READ prerelease NOTIFY changed)
    /** The release can be installed from here: its installer has a digest */
    Q_PROPERTY(bool installable READ installable NOTIFY changed)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QString lastCheck READ lastCheck NOTIFY changed)
    /** The downloaded installer runs when the program closes */
    Q_PROPERTY(bool installOnExit READ installOnExit NOTIFY changed)
    /** The user said "later" to this version: no banner for it */
    Q_PROPERTY(bool dismissed READ dismissed NOTIFY changed)
    Q_PROPERTY(bool autoCheck READ autoCheck WRITE setAutoCheck NOTIFY settingsChanged)
    Q_PROPERTY(bool prereleases READ prereleases WRITE setPrereleases NOTIFY settingsChanged)

public:
    enum State
    {
        Idle = 0,
        Checking,
        UpToDate,
        Available,
        Downloading,
        Ready,
        Failed
    };
    Q_ENUM(State)

    /** A release of GitHub, as far as an update cares */
    struct Release
    {
        QString version;
        QString tag;
        QString notes;
        QString pageUrl;
        QString assetName;
        QString assetUrl;
        qint64 assetSize = 0;
        /** lower case hex, empty when GitHub gives none */
        QString sha256;
        bool prerelease = false;
    };

    UpdateChecker(QObject *parent = nullptr);
    ~UpdateChecker();

    int state() const;
    QString currentVersion() const;
    QString latestVersion() const;
    QString notes() const;
    QString pageUrl() const;
    bool prerelease() const;
    bool installable() const;
    qreal progress() const;
    QString error() const;
    QString lastCheck() const;
    bool installOnExit() const;
    bool dismissed() const;

    bool autoCheck() const;
    void setAutoCheck(bool on);
    bool prereleases() const;
    void setPrereleases(bool on);

    /** Ask GitHub for the newest release. $manual: the user asked (an error
     *  shows, and a dismissed version shows again) */
    Q_INVOKABLE void check(bool manual = true);
    /** Download the installer of the release found, and verify it */
    Q_INVOKABLE void download();
    /** Run the downloaded installer now: it opens the program again when it
     *  ends. The caller closes the program. @return false if it can't start */
    Q_INVOKABLE bool installNow();
    /** Run the downloaded installer when the program closes */
    Q_INVOKABLE void setInstallOnExit(bool on);
    /** No banner for this version (Options still shows it) */
    Q_INVOKABLE void dismiss();
    /** The page of the release on GitHub, in the browser */
    Q_INVOKABLE void openPage();

    /** At startup: a check when automatic checks are on and the last one is
     *  older than a day, a few seconds after start */
    void checkOnStartup();
    /** After the event loop: the installer the user left for the exit */
    void runPendingInstall();

    /** The downloaded installer, once verified */
    QString downloadedFile() const;

    /*********************************************************************
     * For tests and other repositories
     *********************************************************************/
    /** The release API to ask instead of GitHub's (a test server): the rule
     *  that only the releases of the repository are downloaded is lifted */
    void setApiUrl(const QString &url);
    /** Where the installer is downloaded (default: a temporary folder) */
    void setDownloadDir(const QString &dir);
    /** The command line the installer runs with (for tests) */
    QStringList installerArguments(bool relaunch) const;

    /** "v0.13.2", "0.13.0 GIT" -> {0, 13, 2}; empty when it isn't a version */
    static QList<int> parseVersion(const QString &text);
    /** < 0, 0, > 0 like strcmp; a missing number counts as 0 */
    static int compareVersions(const QString &a, const QString &b);
    /** The newest release of $doc (the latest release, or the list of them)
     *  newer than $current, with its installer; nullopt if there is none.
     *  Drafts never count, pre-releases only with $prereleases. $repo: the
     *  only download location allowed ("owner/name"), empty for any */
    static std::optional<Release> pickRelease(const QJsonDocument &doc, const QString &current,
                                              bool prereleases, const QString &repo);
    /** The repository the updates come from: "owner/name" */
    static QString repository();

signals:
    void changed();
    void progressChanged();
    void settingsChanged();

private:
    void setState(State state);
    void fail(const QString &message);
    void finishCheck(QNetworkReply *reply);
    void finishDownload(QNetworkReply *reply);
    QString apiUrl() const;
    QString partialPath() const;
    QString targetPath() const;

    QNetworkAccessManager *m_network;
    QPointer<QNetworkReply> m_reply;
    State m_state;
    bool m_manual;
    std::optional<Release> m_release;
    qreal m_progress;
    QString m_error;
    QString m_downloaded;
    bool m_installOnExit;
    QString m_apiUrl;
    QString m_downloadDir;

    QFile m_file;
    QCryptographicHash m_hash;
    qint64 m_received;
};

#endif

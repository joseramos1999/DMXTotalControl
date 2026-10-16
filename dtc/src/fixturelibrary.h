/*
  DMX Total Control
  fixturelibrary.h

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

#ifndef FIXTURELIBRARY_H
#define FIXTURELIBRARY_H

#include <QObject>
#include <QVariant>
#include <QUrl>
#include <QDir>
#include <QPointer>
#include <QStringList>

#include <atomic>

class QNetworkAccessManager;
class QLCFixtureDef;
class QThread;
class QNetworkReply;
class Doc;

/**
 * Everything about fixture definitions:
 * - the libraries: QLC+ (system), DMX Total Control generics, the user's
 *   own fixtures and the Open Fixture Library download
 * - importing definition files (.qxf, or .zip of .qxf) and GDTF files
 *   (.gdtf, folders of them, or the .zip GDTF-Share downloads), converted
 *   to .qxf in a thread of their own
 * - the fixture editor: definitions are exchanged with QML as plain maps
 */
class FixtureLibrary : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVariantList userFixtures READ userFixtures NOTIFY libraryChanged)
    Q_PROPERTY(int definitionsCount READ definitionsCount NOTIFY libraryChanged)
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadChanged)
    Q_PROPERTY(qreal downloadProgress READ downloadProgress NOTIFY downloadChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool openFixtureLibraryInstalled READ openFixtureLibraryInstalled NOTIFY libraryChanged)
    /** A GDTF import is running, and how far it is (0..1) */
    Q_PROPERTY(bool importing READ importing NOTIFY importChanged)
    Q_PROPERTY(qreal importProgress READ importProgress NOTIFY importChanged)
    /** The GDTF files of the last import that could not be read: "file: why" */
    Q_PROPERTY(QStringList importErrors READ importErrors NOTIFY importChanged)
    /** How many GDTF fixtures are in the library */
    Q_PROPERTY(int gdtfCount READ gdtfCount NOTIFY libraryChanged)

public:
    FixtureLibrary(Doc *doc, QObject *parent = nullptr);
    ~FixtureLibrary();

    /** Load the definition folders DMX Total Control adds to the QLC+ ones.
     *  To be called after the QLC+ system/user definitions are loaded */
    void loadExtraDefinitions();

    static QDir genericDefinitionDirectory();
    static QDir openFixtureLibraryDirectory();
    /** Where the GDTF files go, converted to .qxf, with the pictures of their wheels */
    static QDir gdtfDirectory();

    /** [{ manufacturer, model, file }] of the definitions in the user folder */
    QVariantList userFixtures() const;
    int definitionsCount() const;

    bool downloading() const;
    qreal downloadProgress() const;
    QString status() const;
    bool openFixtureLibraryInstalled() const;
    bool importing() const;
    qreal importProgress() const;
    QStringList importErrors() const;
    int gdtfCount() const;

    /** Copy .qxf files (or the .qxf inside .zip files) to the user folder, and
     *  convert the GDTF files (.gdtf, also in folders and .zip) in the background.
     *  @return the number of definitions imported or being converted */
    Q_INVOKABLE int importFiles(const QList<QUrl> &urls);
    /** Import the user fixtures of a QLC+ installation, if there is one */
    Q_INVOKABLE int importFromQlcPlus();
    /** Download the whole Open Fixture Library (MIT licensed) */
    Q_INVOKABLE void downloadOpenFixtureLibrary();
    Q_INVOKABLE void cancelDownload();
    Q_INVOKABLE void cancelImport();
    Q_INVOKABLE void openUserFolder() const;

    /*********************************************************************
     * Fixture editor
     *********************************************************************/
    /** { name, group, colour } for every channel preset */
    Q_INVOKABLE QVariantList channelPresets() const;
    Q_INVOKABLE QStringList channelGroups() const;
    Q_INVOKABLE QStringList fixtureTypes() const;

    /** A new, empty definition with one RGB mode */
    Q_INVOKABLE QVariantMap newDefinition() const;
    /** Copy of an existing definition, ready to be edited. What the editor
     *  doesn't show comes along, for saveDefinition to keep: the heads and the
     *  physical data of the modes (by the original names of the channels:
     *  "sourceName"), the physical data of the fixture, the presets and
     *  pictures/colours of the capabilities ("xml" of each one) */
    Q_INVOKABLE QVariantMap loadDefinition(const QString &manufacturer, const QString &model) const;
    /** Validate and save $definition in the user folder (in its own file when
     *  it is a user definition already), adding it to the library: the fixtures
     *  patched with it get the new version.
     *  @return an empty string on success, otherwise the error */
    Q_INVOKABLE QString saveDefinition(const QVariantMap &definition);
    /** Delete a user definition. System definitions can't be deleted */
    Q_INVOKABLE bool deleteUserDefinition(const QString &manufacturer, const QString &model);

signals:
    void libraryChanged();
    void downloadChanged();
    void statusChanged();
    void importChanged();

private slots:
    void slotDownloadProgress(qint64 received, qint64 total);
    void slotDownloadFinished();

private:
    void setStatus(const QString &status);
    int importQxf(const QString &path, const QDir &target, const QString &prefix = QString());
    int importZip(const QString &path, const QDir &target);
    int loadNewDefinitions(const QDir &dir);
    QString userFileName(const QString &manufacturer, const QString &model) const;
    /** $def (the library takes it) in the place of the definition with the
     *  same manufacturer and model, or added. The fixtures patched with that
     *  one get the new one (their mode by name), unless its mode doesn't fit
     *  where they are: those keep the old one until they are patched again.
     *  @return how many keep the old one */
    int installDefinition(QLCFixtureDef *def);
    struct GdtfJob
    {
        QString path;
        /** Inside a .zip: its name there */
        QString entry;
    };
    void startGdtf(const QList<GdtfJob> &jobs, int copied);
    void finishGdtf(const QStringList &written, int failed, int warnings, const QStringList &errors, int copied);

private:
    Doc *m_doc;
    QNetworkAccessManager *m_network;
    QNetworkReply *m_reply;
    qreal m_progress;
    QString m_status;
    QPointer<QThread> m_importThread;
    std::atomic<bool> m_cancelImport;
    qreal m_importProgress;
    QStringList m_importErrors;
};

#endif

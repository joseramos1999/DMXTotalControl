/*
  DMX Total Control
  gdtfshare.h

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

#ifndef GDTFSHARE_H
#define GDTFSHARE_H

#include <QObject>
#include <QPointer>
#include <QUrlQuery>
#include <QDateTime>
#include <QVariant>
#include <QLocale>
#include <QList>
#include <QHash>
#include <QPair>
#include <QFile>
#include <QSet>
#include <QDir>

#include <functional>
#include <memory>

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QThread;

/**
 * GDTF-Share, through its public API (login.php, getList.php,
 * downloadFile.php), with the user's own free account.
 *
 * The session is a cookie GDTF-Share gives at sign-in, kept in memory only;
 * it expires after 2 h without use. A request that finds it expired signs in
 * again with the password the program has (typed in this session, or saved)
 * and is made once more; without one, the user is asked to sign in, and the
 * request waits for it. One request at a time: the others wait their turn.
 *
 * The account: the user name is always remembered (the settings, in plain
 * text). The password only when the user asks: encrypted by Windows for the
 * Windows user (DPAPI), never in plain text. Without it, it stays in memory
 * until the program closes.
 *
 * Errors are told in plain words, except for what the program does on its
 * own ($manual false), which never shows one.
 *
 * The list of its fixtures is saved as GDTF-Share sent it (list.json in
 * dataDirectory()), and shown from there, also without internet. It is
 * asked for again when it is more than 20 h old, or when the user asks: an
 * empty one never replaces it. It is read in a thread of its own (it is
 * large), and searched by manufacturer and fixture name.
 *
 * Its fixtures are downloaded one after the other, each one written to disk
 * as it comes (some have large 3D models), and given to the library to be
 * converted as a whole once they are all there. The .gdtf of each one is
 * kept (originals in dataDirectory()), to convert it again with a newer
 * version of the program; and a record says where each fixture of the
 * library came from (see indexFile()).
 *
 * A fixture of the library from GDTF-Share whose fixture type (the same
 * uuid) has a newer revision there is an update (see updates()), known
 * once the list is read. Once a day, at the start, with a saved password,
 * the program signs in by itself for a new list, and tells the updates it
 * hasn't told before (see checkOnStartup()).
 */
class GdtfShare : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int state READ state NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool signedIn READ signedIn NOTIFY changed)
    /** The user of the session, else the one saved (to fill the field) */
    Q_PROPERTY(QString user READ user NOTIFY changed)
    /** The password is saved on this computer */
    Q_PROPERTY(bool remembered READ remembered NOTIFY changed)
    /** See sessionPossible() (the browser shows the account, or Sign in) */
    Q_PROPERTY(bool sessionPossible READ sessionPossible NOTIFY changed)
    /** It can be saved here (Windows) */
    Q_PROPERTY(bool canRemember READ canRemember CONSTANT)
    Q_PROPERTY(QString error READ error NOTIFY changed)
    Q_PROPERTY(QString notice READ notice NOTIFY changed)
    /** The download running (0..1) */
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    /** The fixture types of the list (see search()) */
    Q_PROPERTY(int listCount READ listCount NOTIFY listChanged)
    /** The date of the list shown: invalid without one */
    Q_PROPERTY(QDateTime listDate READ listDate NOTIFY listChanged)
    /** A list is being downloaded or read */
    Q_PROPERTY(bool listLoading READ listLoading NOTIFY listChanged)
    /** One more each time the list changes (for the bindings of QML) */
    Q_PROPERTY(int listRevision READ listRevision NOTIFY listChanged)
    /** The fixtures downloaded whose .gdtf is kept (see originals()) */
    Q_PROPERTY(int originalsCount READ originalsCount NOTIFY indexChanged)
    /** The fixtures of the library with a newer revision on GDTF-Share
     *  (see updates()) */
    Q_PROPERTY(int updatesCount READ updatesCount NOTIFY updatesChanged)

public:
    enum State
    {
        Idle = 0,
        SigningIn,
        Fetching,
        Downloading
    };
    Q_ENUM(State)

    /** What a successful answer is given to: its body. It returns an empty
     *  string when it could use it, else the error to tell */
    using Handler = std::function<QString(const QByteArray &body)>;
    /** What is told when a request ends without its answer: an error, a
     *  cancel, or no sign-in to wait for. $unreachable: GDTF-Share could not
     *  be reached */
    using Failure = std::function<void(bool unreachable)>;

    /** An upload of GDTF-Share: one revision of a fixture type */
    struct Entry
    {
        int rid = 0;
        QString fixture;
        QString manufacturer;
        QString revision;
        /** As GDTF-Share says it: "Manuf." for the manufacturer, else a user */
        QString uploader;
        /** Of the GDTF format */
        QString version;
        QString creator;
        /** Of the fixture type: without braces, in capitals; empty when it
         *  has none (all zeros) */
        QString uuid;
        /** "manufacturer fixture", folded for the search (see fold()) */
        QString key;
        /** The manufacturer, folded */
        QString makerKey;
        /** Seconds since 1970 (0: unknown) */
        qint64 created = 0;
        qint64 modified = 0;
        qint64 filesize = 0;
        double rating = 0;
        /** The name and the DMX channels of each mode */
        QList<QPair<QString, int>> modes;
    };

    /** A fixture downloaded, ready to be converted: its .gdtf, and the
     *  upload it is (see Entry) */
    struct Download
    {
        QString path;
        QString uuid;
        int rid = 0;
        QString revision;
        qint64 modified = 0;
        QString manufacturer;
        QString fixture;
    };

    /** A fixture converted into the GDTF folder, for the record (see
     *  recordImports()) */
    struct Imported
    {
        /** The .qxf written */
        QString file;
        QString manufacturer;
        QString model;
        /** Of the fixture type: that of GDTF-Share, else that of the file */
        QString uuid;
        /** The upload of GDTF-Share (0: imported from a file) */
        int rid = 0;
        /** Of the upload, as GDTF-Share says it */
        QString revision;
        /** Of the last revision in the file */
        QString revisionText;
        qint64 modified = 0;
        /** The .gdtf kept (a download), else empty */
        QString original;
    };

    GdtfShare(QObject *parent = nullptr);
    ~GdtfShare();

    int state() const;
    bool busy() const;
    bool signedIn() const;
    QString user() const;
    QString error() const;
    QString notice() const;
    qreal progress() const;
    bool remembered() const;
    /** A session without asking the user: signed in, or a password to sign
     *  in with (that of the session, or the saved one) */
    bool sessionPossible() const;

    /** Sign in with the user's own account. Then what waited for it runs.
     *  $remember: its password is saved on this computer (canRemember()),
     *  else it stays in memory until the program closes, and a saved one
     *  is forgotten */
    Q_INVOKABLE void signIn(const QString &user, const QString &password, bool remember = false);
    /** End the session: GDTF-Share has no request for it, its cookie is
     *  dropped. The password of the session is forgotten, and what waited
     *  for a sign-in too; until the user signs in again, the saved password
     *  isn't used. $forget: the saved password is forgotten too (the user
     *  name stays) */
    Q_INVOKABLE void signOut(bool forget = false);
    /** Stop the request running, those waiting their turn and those
     *  waiting for a sign-in (the error that sign-in told goes too) */
    Q_INVOKABLE void cancel();

    /** Ask GDTF-Share for its list of fixtures: listReceived() with it */
    Q_INVOKABLE void fetchList(bool manual = true);

    /** A request to $endpoint of the API with the session ($kind: Fetching,
     *  or Downloading for a file). $done gets the body of a successful
     *  answer, else $failed is told. $manual: the user asked (errors are
     *  told, and a sign-in is asked for when needed) */
    void call(State kind, const QString &endpoint, const QUrlQuery &query, bool manual, const Handler &done,
              const Failure &failed = Failure());

    /*********************************************************************
     * The list
     *********************************************************************/
    int listCount() const;
    QDateTime listDate() const;
    bool listLoading() const;
    int listRevision() const;

    /** The browser opens (and after a sign-in): the saved list, read in the
     *  background the first time, and a new one when it is more than 20 h
     *  old and a session is possible without asking */
    Q_INVOKABLE void load();
    /** Ask GDTF-Share for its list now, signed in if needed. $manual: the
     *  user asked; else only with a session that needs nobody, and not
     *  within 20 h of the last one */
    Q_INVOKABLE void refresh(bool manual = true);
    /** [{name, count}]: each manufacturer and its fixture types. With $text
     *  or $filter (as search() has them), only the manufacturers with fixture
     *  types they find, and how many */
    Q_INVOKABLE QVariantList manufacturers(const QString &text = QString(), const QString &filter = QString()) const;
    /** {rows, total}: the fixture types whose manufacturer and name have
     *  every word of $text (in any order, case and accents ignored), of
     *  $manufacturer ("" for all), and of $filter: "" all, "manufacturer"
     *  uploaded by it, "library" in the library, "updates" with a newer
     *  revision. A row is the newest revision of the type (see typeRow());
     *  at most $limit of them, and total counts them all */
    Q_INVOKABLE QVariantMap search(const QString &text, const QString &manufacturer = QString(),
                                   const QString &filter = QString(), int limit = 400) const;
    /** Every revision of the fixture type $uuid (that of a row), newest first */
    Q_INVOKABLE QVariantList revisions(const QString &uuid) const;

    /*********************************************************************
     * What was downloaded
     *********************************************************************/
    /** Download the uploads $rids of the list (signed in if needed), one
     *  after the other, each into originals/ in dataDirectory(). Once they
     *  are all there (or the rest was cancelled), downloaded() gives those
     *  that arrived whole. A rid already on its way, or not in the list, is
     *  left out. @return how many are added */
    Q_INVOKABLE int download(const QVariantList &rids);

    /** The record of the fixtures of the GDTF folder (fixtures.json in
     *  dataDirectory()): {"version": 1, "fixtures": {"<.qxf in the GDTF
     *  folder>": {uuid, rid, revision, revisionText, modified, manufacturer,
     *  model, original, imported}}}, with "original" the .gdtf kept,
     *  relative to dataDirectory(), and "imported" the time (UTC, ISO 8601) */
    static QString indexFile();
    /** $imported into the record, each in the place of what it had for the
     *  same .qxf (a download, or a file: rid 0). A .gdtf kept that nothing
     *  has any more is deleted, and so are the fixtures that are no longer
     *  in the GDTF folder */
    void recordImports(const QList<Imported> &imported);
    /** Delete the downloads $paths that weren't converted, unless the
     *  record keeps them */
    void discardDownloads(const QStringList &paths);
    /** The .gdtf kept of each fixture downloaded that is still in the GDTF
     *  folder, to convert them again with a newer version of the program */
    QStringList originals() const;
    int originalsCount() const;

    /*********************************************************************
     * Newer revisions
     *********************************************************************/
    /** The fixtures of the library from GDTF-Share whose fixture type has a
     *  newer revision there, one for each fixture type (its row of search()
     *  is "older"), in the order of the rows: [{file, manufacturer, model,
     *  fixture, from, to, rid, modified}], with file the .qxf, model its
     *  name in the library, from and to the revisions, and rid and modified
     *  those of the newest one. Known once the list is read, and made again
     *  when the list or the library changes.
     *  A fixture imported from a file is one of GDTF-Share when the last
     *  revision of the file has the text of exactly one revision of its
     *  fixture type (matched again with each list): else it is "from a
     *  file", and never has one */
    Q_INVOKABLE QVariantList updates() const;
    int updatesCount() const;
    /** Download the newest revision of each one (see download()): the
     *  fixtures patched with them get it. @return how many */
    Q_INVOKABLE int updateAll();
    /** At the start, with fixtures of GDTF-Share in the library: the saved
     *  list is read, for the updates it knows (no request). And once a day,
     *  with $autoCheck (UpdateChecker::autoCheck), a saved password and
     *  fixtures downloaded from GDTF-Share, after the other checks of the
     *  start: a sign-in without asking, a new list, and updatesFound() when
     *  it has an update not told before. Nothing is shown when it fails.
     *  First, the downloads a crash left behind (a day old, not in the
     *  record) are deleted */
    void checkOnStartup(bool autoCheck);
    /** The time before the check of the start (tests) */
    void setStartupDelay(int ms);

    /** Where the list, and what is downloaded from GDTF-Share, are kept:
     *  GDTF/gdtf-share in the user fixture folder */
    static QDir dataDirectory();
    /** The uploads in a body of getList.php: an object with "result" true
     *  and a "list" array, each value read as a number or a text. An item
     *  without rid or fixture is skipped. $error: why it can't be read, else
     *  empty. $timestamp: the time GDTF-Share gave (seconds), or 0 */
    static QList<Entry> parseList(const QByteArray &json, QString *error, qint64 *timestamp);
    /** $text for the search: in lower case, without accents */
    static QString fold(const QString &text);

    /** Only https addresses get the password and the session, unless a
     *  test lifted the rule ($testOverride) */
    static bool allowedUrl(const QString &url, bool testOverride);

    /** A password can be saved: Windows encrypts it for the Windows user */
    static bool canRemember();
    /** $plain encrypted for the Windows user (DPAPI): empty when it can't be */
    static QByteArray protectSecret(const QByteArray &plain);
    /** What protectSecret() gave, decrypted: empty when it can't be (made
     *  on another computer or for another Windows user, or not one) */
    static QByteArray unprotectSecret(const QByteArray &blob);

    /*********************************************************************
     * For tests
     *********************************************************************/
    /** The API at another address (a local server): lifts the https rule */
    void setBaseUrl(const QString &url);
    QString baseUrl() const;

signals:
    void changed();
    void progressChanged();
    /** The user has to sign in for what was asked: it waits for it */
    void loginNeeded();
    /** The list of GDTF-Share, as it sent it */
    void listReceived(const QByteArray &body);
    /** The list shown changed, or is being downloaded or read */
    void listChanged();
    /** What was downloaded changed, or the fixtures of the library */
    void indexChanged();
    /** The fixtures of download() that arrived whole, to be converted */
    void downloaded(const QList<GdtfShare::Download> &downloads);
    /** updates() changed */
    void updatesChanged();
    /** The check of the start found updates not told before: $count
     *  fixtures have a newer revision */
    void updatesFound(int count);

private:
    /** A request, waiting its turn or for a sign-in */
    struct Call
    {
        State kind = Idle;
        QString endpoint;
        QUrlQuery query;
        bool manual = false;
        Handler done;
        Failure failed;
        /** Shown while it is on its way (asked by the user) */
        QString notice;
        /** Signed in for it just before: a refusal now is no expired session */
        bool retried = false;
        /** A sign-in the user asked for (kind SigningIn) */
        QString user;
        QString password;
        bool remember = false;
        /** A download written to this file as it comes (as .part until it
         *  is whole): $done gets only its first bytes */
        QString file;
        /** Not the first of a download(): the errors of the ones before stay */
        bool follows = false;
    };

    /** A fixture of the record (see indexFile()) */
    struct Record
    {
        /** Its .qxf, as the record writes it */
        QString file;
        QString uuid;
        int rid = 0;
        qint64 modified = 0;
        QString manufacturer;
        QString model;
        QString revision;
        QString revisionText;
        /** Imported from a file, and found to be the upload rid of the list */
        bool matched = false;
    };
    /** A newer revision of a fixture of the library (see updates()) */
    struct Update
    {
        QString file;
        QString manufacturer;
        QString model;
        QString fixture;
        QString from;
        QString to;
        int rid = 0;
        qint64 modified = 0;
    };
    /** What the check of the start waits for: nothing, the next list read
     *  (a saved one new enough), or the next one GDTF-Share sends */
    enum class Check { None, AnyList, FetchedList };
    /** The record and the .qxf files of the GDTF folder, as read last
     *  (again once indexChanged() is told, or in another user folder) */
    struct Library
    {
        QString folder;
        bool read = false;
        QList<Record> records;
        /** The records of each uuid */
        QHash<QString, QList<int>> ofUuid;
        /** And of each upload (from GDTF-Share) */
        QHash<int, int> ofRid;
        /** The record of each .qxf, and the .qxf files (in lower case) */
        QHash<QString, int> ofFile;
        QSet<QString> files;
    };

    /** A list, ready to be shown */
    struct List
    {
        QList<Entry> entries;
        /** The fixture types (their uploads in entries, newest first), in
         *  the order of the rows */
        QList<QList<int>> types;
        /** The fixture type of each uuid ("rid:<rid>" for an upload without one) */
        QHash<QString, int> typeOf;
        /** Each manufacturer and its fixture types, in order */
        QList<QPair<QString, int>> manufacturers;
        qint64 timestamp = 0;
        QString error;
    };
    /** A list to read in the background: GDTF-Share's ($fetched, written
     *  to $file when it has fixtures), else the one saved in $file */
    struct ListJob
    {
        QString file;
        QByteArray body;
        bool fetched = false;
        bool manual = false;
    };

    static List readList(const QByteArray &json, const QLocale &locale);
    static QString listFile();
    /** The list came less than 20 h ago (asked again by the program after that) */
    bool listFresh() const;
    void readListLater(const ListJob &job);
    void useList(const ListJob &job, const List &list);
    void listFailed(bool unreachable);
    QString listDateText() const;
    /** The fixture type $type of $filter (see search()) */
    bool inFilter(const QList<int> &type, const QString &filter, const Library &library) const;
    /** A row of search(): its newest upload, how many revisions it has, and
     *  how the library has it: status (see libraryStatus()), libraryModel
     *  and replaces (see libraryModel()), and libraryManufacturer and
     *  libraryRid, those of its record (for the patch) */
    QVariantMap typeRow(const QList<int> &type, const Library &library) const;
    /** How the library has the fixture type: "" not, "current" its newest
     *  revision, "older" an older one, "file" from a file */
    QString libraryStatus(const QList<int> &type, const Library &library) const;
    /** The record of the fixture type that tells its status, or -1 */
    int recordOf(const QList<int> &type, const Library &library) const;
    /** The model in the library of the fixture type: its own, else that of
     *  another upload of it, which a download replaces ($replaces) */
    QString libraryModel(const QList<int> &type, const Library &library, bool *replaces) const;
    const Library &library() const;
    /** $record, imported from a file, is the upload of the list whose
     *  revision has the text of its last revision, when exactly one of its
     *  fixture type has it: it gets its rid (see updates()) */
    void matchUpload(Record *record) const;
    /** The upload $rid of the list, or nullptr */
    const Entry *entryOf(int rid) const;

    /** updates() again, from the list and the library: told when it changed */
    void findUpdates();
    /** The check of the start, once its delay is over */
    void checkDaily();
    /** It has a new list: the time of the check, and the updates not told before */
    void checkDone();
    /** The check of the start was done less than 20 h ago */
    bool checkedRecently() const;

    /** The download of $entry into $file, the $number of $count of the batch */
    Call downloadCall(const Entry &entry, const QString &file, int number, int count);
    /** $file.part, whole and closed, becomes $file: else why not */
    QString keepDownload(const Entry &entry, const QString &file, const QByteArray &head);
    /** One download of the batch is over: the last one gives them all */
    void downloadEnded(int rid, qint64 bytes);
    void downloadProgress(qint64 received, qint64 total, bool batch);
    bool openPart(const QString &file);
    /** The file of the download running, closed: kept as it is, or deleted */
    void closePart(bool keep);

    QNetworkAccessManager *network();
    QNetworkRequest request(const QString &endpoint, const QUrlQuery &query, bool download) const;
    void enqueue(const Call &call);
    void runNext();
    void start(const Call &call);
    void send(const Call &call);
    void finishCall(QNetworkReply *reply, Call call);
    /** Sign in, then send $call (none: the sign-in the user asked for, then
     *  the requests that waited for it) */
    void login(const QString &user, const QString &password, const Call &call);
    void finishLogin(QNetworkReply *reply, const QString &user, const QString &password, Call call);
    /** Sign in for $call without asking, with the password the program
     *  has; without one, $call waits for the user */
    void signInAgain(const Call &call);
    /** A sign-in is needed (no password, or a wrong one): $call waits for
     *  the user to give one, with $message */
    void waitForLogin(const Call &call, const QString &message);

    /** The password for a sign-in without asking: that of the session,
     *  else the saved one. A saved one that can't be read is forgotten,
     *  with the notice ($unreadable) */
    QString passwordForSession(bool *unreadable);
    /** The account a sign-in of the user worked with, for the next times */
    void saveAccount(const QString &user, const QString &password, bool remember);
    void forgetPassword();
    void answerError(QNetworkReply *reply, const QByteArray &body);
    /** The request running ends with $message ($unreachable: GDTF-Share
     *  could not be reached) */
    void fail(const QString &message, bool unreachable = false);
    /** What the requests running, waiting their turn and waiting for a
     *  sign-in tell when they end: they are all dropped */
    QList<Failure> dropCalls();
    void finish();
    void setState(State state);
    void dropReply();

    QNetworkAccessManager *m_network;
    QPointer<QNetworkReply> m_reply;
    State m_state;
    QString m_baseUrl;
    bool m_testUrl;
    QString m_user;
    /** The password of the session (memory only), for a silent sign-in */
    QString m_password;
    bool m_signedIn;
    /** Signed out by the user: the saved password waits for a sign-in */
    bool m_signedOut;
    bool m_manual;
    QString m_error;
    QString m_notice;
    qreal m_progress;
    QList<Call> m_queue;
    QList<Call> m_pending;
    /** What the request running tells if it ends without its answer */
    Failure m_failed;

    List m_list;
    QDateTime m_listDate;
    /** When the last answer to a list request came (this run) */
    QDateTime m_listFetched;
    int m_listRevision;
    /** A list request is on its way, or waits its turn or a sign-in */
    bool m_listFetching;
    /** GDTF-Share could not be reached for the last one: the saved list */
    bool m_listOffline;
    QPointer<QThread> m_listThread;
    /** The list waiting for the one being read */
    QList<ListJob> m_listJobs;

    /** The downloads of download() still to come (by rid), how many it has
     *  and how many are over, their bytes (as the list says) and those of
     *  the ones over, and the files whole */
    QSet<int> m_downloadRids;
    int m_downloadCount;
    int m_downloadsOver;
    qint64 m_downloadBytes;
    qint64 m_downloadedBytes;
    QList<Download> m_downloads;
    /** The list says the size of each one: else the progress counts files */
    bool m_downloadSizes;
    /** The file the download running is written to */
    std::unique_ptr<QFile> m_part;

    mutable Library m_library;

    QList<Update> m_updates;
    int m_startupDelay;
    Check m_check;
};

#endif

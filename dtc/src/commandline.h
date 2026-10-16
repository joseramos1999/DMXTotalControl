/*
  DMX Total Control
  commandline.h

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

#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QObject>
#include <QStringList>

class PlaybackManager;
class PaletteManager;
class CuelistEditor;
class PatchManager;
class Programmer;
class Doc;

/**
 * The command line of a console, on the programmer:
 *
 *   1 THRU 8          select fixtures 1 to 8 (by their number)
 *   1 THRU 8 - 4 + 12 ...without 4, with 12; "+ 5" adds to the selection
 *   GROUP 2           the fixtures of fixture group 2
 *   @ 50  @ 5  @ FULL  @ @  @ OUT   dimmer of the selection (one digit: tens, like consoles)
 *   @ 0 THRU 100      the dimmer spread over the selection
 *   @ + 10  @ - 10    the dimmer up or down
 *   POS 3  COL 2  BEAM 1  INT 4    a palette on the selection
 *   LOCATE  HIGHLIGHT  NEXT  PREV  ALL  REL  CLEAR
 *
 * and on the selected cuelist:
 *
 *   PB 3              select playback 3 of the current page (and its cuelist)
 *   GO  GO CUE 3  BACK   the next cue, cue 3, the cue before
 *   CUE 3             cue 3 (what the stage shows on it) in the programmer
 *   RECORD  RECORD CUE 2.5   the programmer as the next whole cue, or as cue 2.5
 *   UPDATE  UPDATE CUE 3     the programmer into the cue loaded (else the cue
 *                     that plays), or into cue 3
 *   DELETE CUE 4      delete a cue; the next cues look the same
 *
 * They can be chained: "1 THRU 4 @ FULL COL 2". Spanish words work too
 * (HASTA, GRUPO, HAZ, TODO, IR, GRABAR...), and ">" or "/" mean THRU. Cue
 * numbers may have decimals ("2.5" or "2,5"); anywhere else "," is "+".
 * A line with a mistake does nothing at all, not even its first words: the
 * cue words change cues and the stage, and CLEAR doesn't undo that. A number
 * is never short for CUE ("UPDATE 3" is a mistake: UPDATE CUE 3). What the
 * show doesn't have (a cue, a palette, a dimmer...) stops a line that reads
 * well there: the message says what the words before it did, and rest() is
 * the part of the line left to correct.
 */
class CommandLine : public QObject
{
    Q_OBJECT

    /** What the last command did, or why it didn't */
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)
    Q_PROPERTY(bool error READ error NOTIFY messageChanged)
    /** The last commands, the newest last */
    Q_PROPERTY(QStringList history READ history NOTIFY historyChanged)
    /** After a line that stopped part way: the words not done, from the one
     *  that failed ("" when nothing was done, or all of it) */
    Q_PROPERTY(QString rest READ rest NOTIFY messageChanged)

public:
    CommandLine(Doc *doc, Programmer *programmer, PatchManager *patch, PaletteManager *palettes,
                QObject *parent = nullptr);

    /** The cuelist words (GO, RECORD, CUE...) work on the cuelist the
     *  editor has selected; PB selects one of the playbacks */
    void setCuelists(CuelistEditor *cuelists);
    void setPlaybacks(PlaybackManager *playbacks);

    QString message() const;
    bool error() const;
    QStringList history() const;
    QString rest() const;

    /** Run a command line. @return true if it did it all (see message) */
    Q_INVOKABLE bool execute(const QString &command);
    /** The command with its words as the console writes them ("1 THRU 8 @ FULL") */
    Q_INVOKABLE QString normalize(const QString &command) const;

signals:
    void messageChanged();
    void historyChanged();

private slots:
    void slotDocCleared();

private:
    struct Token
    {
        enum Kind { Number, Word, Symbol };
        Kind kind;
        /** Numbers: their text ("05", a cue number "2.5"); words: their
         *  command (THRU, AT...); symbols: + - @ */
        QString text;
        /** As typed ("2,5", "grabar") */
        QString raw;
    };
    QList<Token> tokenize(const QString &command, QString *error) const;
    /**
     * What is wrong in how the line $t is written (a number where a word
     * goes, "@" with no level, a number after GO...), or "" when it reads
     * well. It reads the words as execute() does, without looking at the
     * show: execute() runs nothing of a line that doesn't read well.
     */
    static QString syntaxError(const QList<Token> &t);
    void setMessage(const QString &message, bool error);
    static bool isCueWord(const Token &token);
    /**
     * Runs the cuelist word at $t[$i] (GO, BACK, CUE, RECORD, UPDATE, DELETE,
     * PLAYBACK) with what follows it, and moves $i past them.
     * @return false with the reason in $error, else what it did is in $done
     */
    bool runCueWord(const QList<Token> &t, int &i, QStringList *done, QString *error);

private:
    Doc *m_doc;
    Programmer *m_programmer;
    PatchManager *m_patch;
    PaletteManager *m_palettes;
    CuelistEditor *m_cuelists;
    PlaybackManager *m_playbacks;
    QString m_message;
    bool m_error;
    QStringList m_history;
    QString m_rest;
};

#endif

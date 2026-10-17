/*
  DMX Total Control
  commandline.cpp

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

#include <QScopeGuard>
#include <QHash>
#include <algorithm>
#include <limits>

#include "commandline.h"
#include "playbackmanager.h"
#include "palettemanager.h"
#include "cuelisteditor.h"
#include "patchmanager.h"
#include "programmer.h"

#include "cuelist.h"
#include "doc.h"

CommandLine::CommandLine(Doc *doc, Programmer *programmer, PatchManager *patch, PaletteManager *palettes,
                         QObject *parent)
    : QObject(parent)
    , m_doc(doc)
    , m_programmer(programmer)
    , m_patch(patch)
    , m_palettes(palettes)
    , m_cuelists(nullptr)
    , m_playbacks(nullptr)
    , m_error(false)
{
    // a new show: what the last command did is gone too
    connect(m_doc, SIGNAL(cleared()), this, SLOT(slotDocCleared()));
}

void CommandLine::setCuelists(CuelistEditor *cuelists)
{
    m_cuelists = cuelists;
}

void CommandLine::setPlaybacks(PlaybackManager *playbacks)
{
    m_playbacks = playbacks;
}

void CommandLine::slotDocCleared()
{
    m_rest.clear();
    setMessage(QString(), false);
}

QString CommandLine::message() const
{
    return m_message;
}

bool CommandLine::error() const
{
    return m_error;
}

QStringList CommandLine::history() const
{
    return m_history;
}

void CommandLine::setMessage(const QString &message, bool error)
{
    m_message = message;
    m_error = error;
    emit messageChanged();
}

/** The command of a word, in English or Spanish, or "" */
static QString commandOf(const QString &typed)
{
    static const QHash<QString, QString> words = {
        { "THRU", "THRU" }, { "THROUGH", "THRU" }, { "THR", "THRU" }, { "HASTA", "THRU" },
        { "AT", "AT" },
        { "FULL", "FULL" }, { "FL", "FULL" }, { "MAX", "FULL" }, { "LLENO", "FULL" }, { "TOPE", "FULL" },
        { "OUT", "OUT" }, { "ZERO", "OUT" }, { "CERO", "OUT" },
        { "GROUP", "GROUP" }, { "GRP", "GROUP" }, { "G", "GROUP" }, { "GRUPO", "GROUP" },
        { "ALL", "ALL" }, { "TODO", "ALL" }, { "TODOS", "ALL" }, { "TODAS", "ALL" },
        { "INT", "INT" }, { "INTENSITY", "INT" }, { "INTENSIDAD", "INT" }, { "DIM", "INT" },
        { "POS", "POS" }, { "POSITION", "POS" }, { "POSICION", "POS" },
        { "COL", "COL" }, { "COLOR", "COL" }, { "COLOUR", "COL" },
        { "BEAM", "BEAM" }, { "HAZ", "BEAM" },
        { "CLEAR", "CLEAR" }, { "CLR", "CLEAR" }, { "BORRAR", "CLEAR" }, { "LIMPIAR", "CLEAR" },
        { "HIGHLIGHT", "HIGHLIGHT" }, { "HI", "HIGHLIGHT" }, { "HL", "HIGHLIGHT" }, { "RESALTAR", "HIGHLIGHT" },
        { "LOCATE", "LOCATE" }, { "LOC", "LOCATE" }, { "LOCALIZAR", "LOCATE" },
        { "NEXT", "NEXT" }, { "SIG", "NEXT" }, { "SIGUIENTE", "NEXT" },
        { "PREV", "PREV" }, { "PREVIOUS", "PREV" }, { "ANT", "PREV" }, { "ANTERIOR", "PREV" },
        { "REL", "REL" }, { "RELEASE", "REL" }, { "LIBERAR", "REL" },
        // the selected cuelist ("BORRAR" is CLEAR, so DELETE is ELIMINAR)
        { "GO", "GO" }, { "IR", "GO" },
        { "BACK", "BACK" }, { "ATRAS", "BACK" }, { "VOLVER", "BACK" },
        { "CUE", "CUE" }, { "Q", "CUE" },
        { "RECORD", "RECORD" }, { "REC", "RECORD" }, { "GRABAR", "RECORD" },
        { "UPDATE", "UPDATE" }, { "UPD", "UPDATE" }, { "ACTUALIZAR", "UPDATE" },
        { "DELETE", "DELETE" }, { "DEL", "DELETE" }, { "ELIMINAR", "DELETE" },
        { "PLAYBACK", "PLAYBACK" }, { "PB", "PLAYBACK" },
    };
    // without accents: POSICIÓN is POSICION
    QString word = typed.normalized(QString::NormalizationForm_D).toUpper();
    word.erase(std::remove_if(word.begin(), word.end(), [](QChar c) { return c.category() == QChar::Mark_NonSpacing; }),
               word.end());
    return words.value(word);
}

QList<CommandLine::Token> CommandLine::tokenize(const QString &command, QString *error) const
{
    QList<Token> tokens;
    const int n = int(command.length());
    int i = 0;
    while (i < n)
    {
        const QChar c = command.at(i);
        if (c.isSpace())
        {
            i++;
            continue;
        }
        if (c.isDigit())
        {
            int start = i;
            while (i < n && command.at(i).isDigit())
                i++;
            QString number = command.mid(start, i - start);
            // a cue number may have decimals: CUE 2.5 or CUE 2,5. Anywhere
            // else "." is not understood and "," is "+" (1,3,5)
            const bool afterCue = tokens.isEmpty() == false && tokens.last().kind == Token::Word &&
                                  tokens.last().text == QLatin1String("CUE");
            if (afterCue && i + 1 < n && (command.at(i) == QLatin1Char('.') || command.at(i) == QLatin1Char(',')) &&
                command.at(i + 1).isDigit())
            {
                const int decimals = ++i;
                while (i < n && command.at(i).isDigit())
                    i++;
                number += QLatin1Char('.') + command.mid(decimals, i - decimals);
            }
            tokens.append({ Token::Number, number, command.mid(start, i - start) });
            continue;
        }
        if (c.isLetter())
        {
            int start = i;
            while (i < n && command.at(i).isLetter())
                i++;
            const QString raw = command.mid(start, i - start);
            const QString word = commandOf(raw);
            if (word.isEmpty())
            {
                *error = tr("I don't understand “%1”").arg(raw);
                return QList<Token>();
            }
            tokens.append({ Token::Word, word, raw });
            continue;
        }
        switch (c.unicode())
        {
            case '@':
                tokens.append({ Token::Word, "AT", "@" });
                break;
            // THRU on a keyboard (consoles use / on the keypad)
            case '>': case '/':
                tokens.append({ Token::Word, "THRU", QString(c) });
                break;
            case '+': case '-':
                tokens.append({ Token::Symbol, QString(c), QString(c) });
                break;
            // 1,3,5 = 1 + 3 + 5
            case ',':
                tokens.append({ Token::Symbol, "+", "," });
                break;
            default:
                *error = tr("I don't understand “%1”").arg(c);
                return QList<Token>();
        }
        i++;
    }
    return tokens;
}

QString CommandLine::normalize(const QString &command) const
{
    QString error;
    const QList<Token> tokens = tokenize(command, &error);
    if (error.isEmpty() == false)
        return command.trimmed();
    QStringList parts;
    for (const Token &t : tokens)
        parts.append(t.kind == Token::Word && t.text == QLatin1String("AT") ? QStringLiteral("@") : t.text);
    return parts.join(' ');
}

/** A whole number as typed; one too big for an int is the biggest one
 *  (toInt() would make it 0: "@ 99999999999" was a blackout) */
static int numberOf(const QString &number)
{
    bool ok = false;
    const int value = number.toInt(&ok);
    return ok ? value : std::numeric_limits<int>::max();
}

/** A level typed after @: one digit is tens, like on consoles ("@ 5" is 50 %) */
static int levelOf(const QString &number)
{
    return number.length() == 1 ? numberOf(number) * 10 : numberOf(number);
}

static uchar dmxOf(int percent)
{
    // in integers: 50 % is 128, not 127.49...
    return uchar(qBound(0, (percent * 255 + 50) / 100, 255));
}

QString CommandLine::syntaxError(const QList<Token> &t)
{
    // the same reading as execute(), with its messages, but nothing is done
    // and the show is not looked at (that is for execute)
    const int count = int(t.count());
    auto isWord = [&t, count](int k, const char *word) {
        return k < count && t.at(k).kind == Token::Word && t.at(k).text == QLatin1String(word);
    };
    auto isSymbol = [&t, count](int k, const char *symbol) {
        return k < count && t.at(k).kind == Token::Symbol && t.at(k).text == QLatin1String(symbol);
    };
    auto isNumber = [&t, count](int k) { return k < count && t.at(k).kind == Token::Number; };
    int i = 0;

    // the selection
    if (isNumber(0) || isWord(0, "GROUP") || isWord(0, "ALL") ||
        ((isSymbol(0, "+") || isSymbol(0, "-")) && (isNumber(1) || isWord(1, "GROUP"))))
    {
        while (true)
        {
            const bool add = isSymbol(i, "-") == false;
            if (isSymbol(i, "+") || isSymbol(i, "-"))
                i++;
            if (isNumber(i))
            {
                i++;
                if (isWord(i, "THRU"))
                {
                    i++;
                    if (isNumber(i))
                        i++;
                }
            }
            else if (isWord(i, "GROUP"))
            {
                i++;
                if (isNumber(i) == false)
                    return tr("Which group? GROUP 1, GROUP 2…");
                i++;
                if (isWord(i, "THRU") && isNumber(i + 1))
                    i += 2;
            }
            else if (isWord(i, "ALL"))
            {
                i++;
            }
            else
            {
                return tr("A fixture number, GROUP or ALL after “%1”").arg(add ? "+" : "-");
            }
            if (isSymbol(i, "+") == false && isSymbol(i, "-") == false)
                break;
        }
    }

    // what to do with it
    while (i < count)
    {
        const Token &token = t.at(i++);
        const QString word = token.kind == Token::Word ? token.text : QString();
        if (word == QLatin1String("AT"))
        {
            if (isWord(i, "AT") || isWord(i, "FULL") || isWord(i, "OUT"))
            {
                i++;
            }
            else if (isSymbol(i, "+") || isSymbol(i, "-"))
            {
                if (isNumber(++i) == false)
                    return tr("How much? @ + 10, @ - 10");
                if (levelOf(t.at(i++).text) > 100)
                    return tr("A level from 0 to 100");
            }
            else if (isNumber(i))
            {
                if (levelOf(t.at(i++).text) > 100)
                    return tr("A level from 0 to 100");
                if (isWord(i, "THRU"))
                {
                    i++;
                    const bool level = isNumber(i) ? levelOf(t.at(i).text) <= 100
                                                   : (isWord(i, "FULL") || isWord(i, "OUT"));
                    if (level == false)
                        return tr("A level from 0 to 100");
                    i++;
                }
            }
            else
            {
                return tr("@ what? A level from 0 to 100, FULL or OUT");
            }
        }
        else if (word == QLatin1String("INT") || word == QLatin1String("POS") || word == QLatin1String("COL") ||
                 word == QLatin1String("BEAM"))
        {
            if (isNumber(i++) == false)
                return tr("Which palette? %1 1, %1 2…").arg(word);
        }
        else if (word == QLatin1String("LOCATE") || word == QLatin1String("HIGHLIGHT") ||
                 word == QLatin1String("NEXT") || word == QLatin1String("PREV") || word == QLatin1String("ALL") ||
                 word == QLatin1String("REL") || word == QLatin1String("CLEAR"))
        {
            continue;
        }
        else if (word == QLatin1String("PLAYBACK"))
        {
            if (isNumber(i++) == false)
                return tr("Which playback? PB 1, PB 2…");
        }
        else if (word == QLatin1String("BACK"))
        {
            // one cue back, never to a cue ("BACK CUE 3" would go back and
            // load cue 3 in the programmer)
            const int n = isNumber(i) ? i : (isWord(i, "CUE") && isNumber(i + 1) ? i + 1 : -1);
            if (n >= 0)
                return tr("BACK goes one cue back; to go to cue %1: GO CUE %1").arg(t.at(n).raw);
        }
        else if (isCueWord(token))
        {
            // GO, RECORD, UPDATE and DELETE take CUE and its number (DELETE
            // must). A number right after them is not short for CUE (on some
            // consoles GO 3 is playback 3): "UPDATE 3" must not update the
            // loaded cue, nor "GO 3" fire the next one
            if (word != QLatin1String("CUE"))
            {
                if (isNumber(i))
                    return tr("Which cue? %1 CUE %2").arg(token.raw.toUpper(), t.at(i).raw);
                if (isWord(i, "CUE") == false)
                {
                    if (word == QLatin1String("DELETE"))
                        return tr("Which cue? DELETE CUE 4");
                    continue;
                }
                i++;
            }
            if (isNumber(i) == false)
                return tr("Which cue? CUE 1, CUE 2.5…");
            const Token &number = t.at(i++);
            if (Cuelist::parseNumber(number.text) <= 0)
                return tr("%1 is not a cue number").arg(number.raw);
        }
        else
        {
            return tr("I don't understand “%1” there").arg(token.raw.isEmpty() ? token.text : token.raw);
        }
    }
    return QString();
}

bool CommandLine::execute(const QString &command)
{
    m_rest.clear();
    QString error;
    const QList<Token> t = tokenize(command, &error);
    if (error.isEmpty() == false)
    {
        setMessage(error, true);
        return false;
    }
    if (t.isEmpty())
        return false;

    const QString normalized = normalize(command);
    m_history.removeAll(normalized);
    m_history.append(normalized);
    while (m_history.count() > 30)
        m_history.removeFirst();
    emit historyChanged();

    const int count = int(t.count());
    auto isWord = [&t, count](int k, const char *word) {
        return k < count && t.at(k).kind == Token::Word && t.at(k).text == QLatin1String(word);
    };
    auto isSymbol = [&t, count](int k, const char *symbol) {
        return k < count && t.at(k).kind == Token::Symbol && t.at(k).text == QLatin1String(symbol);
    };
    auto isNumber = [&t, count](int k) { return k < count && t.at(k).kind == Token::Number; };
    // what the line did, and where the word being run starts
    QStringList done;
    int wordStart = 0;
    auto fail = [this, &done, &t, &wordStart](const QString &why) {
        // what the show doesn't have stops the line there: the words before
        // it did their part (a GO, a cue recorded can't be taken back), the
        // message says so, and what is left of the line is to correct
        if (done.isEmpty() == false)
        {
            QStringList rest;
            for (int k = wordStart; k < int(t.count()); k++)
                rest.append(t.at(k).kind == Token::Word && t.at(k).text == QLatin1String("AT") ? QStringLiteral("@")
                                                                                              : t.at(k).text);
            m_rest = rest.join(' ');
            setMessage(done.join(QStringLiteral(" · ")) + QStringLiteral(" · ") + why, true);
            return false;
        }
        setMessage(why, true);
        return false;
    };

    // nothing of a line with a mistake is done, not even the words before
    // it: a cue recorded or updated, or a GO, can't be taken back
    const QString mistake = syntaxError(t);
    if (mistake.isEmpty() == false)
        return fail(mistake);

    int i = 0;

    // the selection: numbers, groups, ALL, joined with + and -
    const bool selecting = isNumber(0) || isWord(0, "GROUP") || isWord(0, "ALL") ||
                           ((isSymbol(0, "+") || isSymbol(0, "-")) && (isNumber(1) || isWord(1, "GROUP")));
    if (selecting)
    {
        // "+ 5" and "- 5" change the selection there is; else it is a new one
        QList<quint32> selection;
        if (isSymbol(0, "+") || isSymbol(0, "-"))
            selection = m_programmer->selectionIds();

        const QList<quint32> byNumber = m_patch->fixturesByNumber();
        const int lastNumber = byNumber.isEmpty() ? 0 : m_patch->fixtureNumber(byNumber.last());
        const QVariantList groups = m_patch->groups();

        bool add = true;
        while (true)
        {
            if (isSymbol(i, "+"))
            {
                add = true;
                i++;
            }
            else if (isSymbol(i, "-"))
            {
                add = false;
                i++;
            }

            QList<quint32> term;
            if (isNumber(i))
            {
                const QString fromText = t.at(i).raw;
                const int from = numberOf(t.at(i).text);
                i++;
                QString toText = fromText;
                int to = from;
                if (isWord(i, "THRU"))
                {
                    i++;
                    if (isNumber(i))
                    {
                        toText = t.at(i).raw;
                        to = numberOf(t.at(i++).text);
                    }
                    else
                    {
                        to = qMax(from, lastNumber);
                        toText = QString::number(to);
                    }
                }
                // the fixtures numbered from one to the other, in that order:
                // the patch is walked, not every number in between (1 THRU
                // 999999999 froze the window)
                for (quint32 id : byNumber)
                {
                    const int number = m_patch->fixtureNumber(id);
                    if (number >= 1 && number >= qMin(from, to) && number <= qMax(from, to))
                        term.append(id);
                }
                if (from > to)
                    std::reverse(term.begin(), term.end());
                if (term.isEmpty())
                    return fail(from == to ? tr("There is no fixture %1").arg(fromText)
                                           : tr("There are no fixtures from %1 to %2").arg(fromText, toText));
            }
            else if (isWord(i, "GROUP"))
            {
                i++;
                if (isNumber(i) == false)
                    return fail(tr("Which group? GROUP 1, GROUP 2…"));
                const int from = numberOf(t.at(i++).text);
                int to = from;
                if (isWord(i, "THRU") && isNumber(i + 1))
                {
                    to = numberOf(t.at(i + 1).text);
                    i += 2;
                }
                const int step = from <= to ? 1 : -1;
                for (int number = from; ; number += step)
                {
                    if (number < 1 || number > groups.count())
                        return fail(tr("There is no group %1").arg(number));
                    for (const QVariant &fid : groups.at(number - 1).toMap().value("fixtures").toList())
                        if (term.contains(fid.toUInt()) == false)
                            term.append(fid.toUInt());
                    if (number == to)
                        break;
                }
            }
            else if (isWord(i, "ALL"))
            {
                i++;
                // after NEXT / PREV: the whole selection again
                if (m_programmer->stepping().value("active").toBool())
                {
                    m_programmer->allFixtures();
                    term = m_programmer->selectionIds();
                }
                else
                {
                    term = byNumber;
                }
            }
            else
            {
                return fail(tr("A fixture number, GROUP or ALL after “%1”").arg(add ? "+" : "-"));
            }

            for (quint32 id : std::as_const(term))
            {
                if (add && selection.contains(id) == false)
                    selection.append(id);
                else if (add == false)
                    selection.removeAll(id);
            }
            if (isSymbol(i, "+") == false && isSymbol(i, "-") == false)
                break;
        }

        QVariantList ids;
        for (quint32 id : std::as_const(selection))
            ids.append(id);
        m_programmer->selectFixtures(ids);
        done.append(tr("%n fixture(s) selected", "", int(selection.count())));
    }

    // what to do with it
    while (i < count)
    {
        wordStart = i;
        const int selected = int(m_programmer->selectionIds().count());
        if (isWord(i, "AT"))
        {
            i++;
            if (selected == 0)
                return fail(tr("Select fixtures first"));

            // what is said is how many fixtures have a dimmer that took it
            // (RGB-only fixtures have none)
            const QString noDimmer = tr("The selected fixtures have no dimmer");
            if (isWord(i, "AT") || isWord(i, "FULL") || isWord(i, "OUT"))
            {
                const bool full = isWord(i, "OUT") == false;
                i++;
                const int dimmed = m_programmer->setDimmer(full ? 255 : 0);
                if (dimmed == 0)
                    return fail(noDimmer);
                done.append(tr("Dimmer at %1 % on %n fixture(s)", "", dimmed).arg(full ? 100 : 0));
            }
            else if (isSymbol(i, "+") || isSymbol(i, "-"))
            {
                const int sign = isSymbol(i, "+") ? 1 : -1;
                i++;
                if (isNumber(i) == false)
                    return fail(tr("How much? @ + 10, @ - 10"));
                const int percent = levelOf(t.at(i++).text);
                if (percent > 100)
                    return fail(tr("A level from 0 to 100"));
                const int dimmed = m_programmer->nudgeDimmer(sign * int(dmxOf(percent)));
                if (dimmed == 0)
                    return fail(noDimmer);
                done.append(tr("Dimmer %1%2 % on %n fixture(s)", "", dimmed).arg(sign > 0 ? "+" : "-").arg(percent));
            }
            else if (isNumber(i))
            {
                const int from = levelOf(t.at(i++).text);
                if (from > 100)
                    return fail(tr("A level from 0 to 100"));
                if (isWord(i, "THRU"))
                {
                    // spread over the selection, in its order
                    i++;
                    int to = -1;
                    if (isNumber(i))
                        to = levelOf(t.at(i++).text);
                    else if (isWord(i, "FULL"))
                        to = 100, i++;
                    else if (isWord(i, "OUT"))
                        to = 0, i++;
                    if (to < 0 || to > 100)
                        return fail(tr("A level from 0 to 100"));
                    const int dimmed = m_programmer->setChannelFan("dimmer", dmxOf(from),
                                                                   int(dmxOf(to)) - int(dmxOf(from)), false);
                    if (dimmed == 0)
                        return fail(noDimmer);
                    done.append(tr("Dimmer from %1 % to %2 % on %n fixture(s)", "", dimmed).arg(from).arg(to));
                }
                else
                {
                    const int dimmed = m_programmer->setDimmer(dmxOf(from));
                    if (dimmed == 0)
                        return fail(noDimmer);
                    done.append(tr("Dimmer at %1 % on %n fixture(s)", "", dimmed).arg(from));
                }
            }
            else
            {
                return fail(tr("@ what? A level from 0 to 100, FULL or OUT"));
            }
        }
        else if (isWord(i, "INT") || isWord(i, "POS") || isWord(i, "COL") || isWord(i, "BEAM"))
        {
            static const QHash<QString, QString> types = {
                { "INT", "intensity" }, { "POS", "position" }, { "COL", "colour" }, { "BEAM", "beam" } };
            const QString word = t.at(i++).text;
            const QString type = types.value(word);
            if (isNumber(i) == false)
                return fail(tr("Which palette? %1 1, %1 2…").arg(word));
            const QString typed = t.at(i).raw;
            const int number = numberOf(t.at(i++).text);
            const int id = m_palettes->paletteByNumber(type, number);
            if (id < 0)
                return fail(tr("There is no palette %1 %2").arg(word, typed));
            if (selected == 0)
                return fail(tr("Select fixtures first"));
            const int applied = m_palettes->apply(id);
            const QString name = m_palettes->palette(id).value("name").toString();
            if (applied == 0)
                return fail(tr("%1 doesn't apply to the selected fixtures").arg(name));
            done.append(tr("%1 on %n fixture(s)", "", applied).arg(name));
        }
        else if (isWord(i, "LOCATE"))
        {
            i++;
            if (selected == 0)
                return fail(tr("Select fixtures first"));
            m_programmer->locate();
            done.append(tr("%n fixture(s) located", "", selected));
        }
        else if (isWord(i, "HIGHLIGHT"))
        {
            i++;
            m_programmer->setHighlight(m_programmer->highlight() == false);
            done.append(m_programmer->highlight() ? tr("Highlight on") : tr("Highlight off"));
        }
        else if (isWord(i, "NEXT") || isWord(i, "PREV"))
        {
            isWord(i, "NEXT") ? m_programmer->nextFixture() : m_programmer->previousFixture();
            i++;
            const QVariantMap step = m_programmer->stepping();
            if (step.value("active").toBool())
                done.append(tr("Fixture %1 of %2").arg(step.value("index").toInt() + 1).arg(step.value("count").toInt()));
        }
        else if (isWord(i, "ALL"))
        {
            i++;
            m_programmer->allFixtures();
            done.append(tr("%n fixture(s) selected", "", int(m_programmer->selectionIds().count())));
        }
        else if (isWord(i, "REL"))
        {
            i++;
            m_programmer->resetSelected();
            done.append(tr("The edits of the selection are released"));
        }
        else if (isWord(i, "CLEAR"))
        {
            i++;
            m_programmer->clear();
            done.append(tr("The programmer is clear"));
        }
        else if (isCueWord(t.at(i)))
        {
            QString why;
            if (runCueWord(t, i, &done, &why) == false)
                return fail(why);
        }
        else
        {
            const Token &token = t.at(i);
            return fail(tr("I don't understand “%1” there").arg(token.raw.isEmpty() ? token.text : token.raw));
        }
    }

    setMessage(done.join(QStringLiteral(" · ")), false);
    return true;
}

QString CommandLine::rest() const
{
    return m_rest;
}

bool CommandLine::isCueWord(const Token &token)
{
    static const QStringList words = { "GO", "BACK", "CUE", "RECORD", "UPDATE", "DELETE", "PLAYBACK" };
    return token.kind == Token::Word && words.contains(token.text);
}

bool CommandLine::runCueWord(const QList<Token> &t, int &i, QStringList *done, QString *error)
{
    const int count = int(t.count());
    auto isWord = [&t, count](int k, const char *word) {
        return k < count && t.at(k).kind == Token::Word && t.at(k).text == QLatin1String(word);
    };
    auto fail = [error](const QString &why) {
        *error = why;
        return false;
    };
    const QString word = t.at(i++).text;

    // PB 3: playback 3 of the current page is the selected one, and so is
    // its cuelist
    if (word == QLatin1String("PLAYBACK"))
    {
        if (i >= count || t.at(i).kind != Token::Number)
            return fail(tr("Which playback? PB 1, PB 2…"));
        const QString typed = t.at(i++).raw;
        const int number = numberOf(typed);
        if (m_playbacks == nullptr || number < 1 || number > m_playbacks->slotsPerPage())
            return fail(tr("There is no playback %1 on this page").arg(typed));
        const int functionId = m_playbacks->functionAt(-1, number - 1);
        Function *f = functionId >= 0 ? m_doc->function(quint32(functionId)) : nullptr;
        if (f == nullptr)
            return fail(tr("Playback %1 is empty").arg(number));
        m_playbacks->select(-1, number - 1);
        // a scene, a chaser...: no cuelist is selected, so that the cue
        // words after it don't work on the cuelist selected before
        if (qobject_cast<Cuelist *>(f) == nullptr && m_cuelists != nullptr)
            m_cuelists->setSelectedId(-1);
        done->append(tr("Playback %1 selected: %2").arg(number).arg(f->name()));
        return true;
    }

    // the rest work on the selected cuelist (the transport through the
    // playbacks, so a release of the playback releases it too)
    const int selectedId = m_cuelists != nullptr ? m_cuelists->selectedId() : -1;
    Cuelist *cl = selectedId >= 0 ? qobject_cast<Cuelist *>(m_doc->function(quint32(selectedId))) : nullptr;
    const bool transport = word == QLatin1String("GO") || word == QLatin1String("BACK");
    if (cl == nullptr || (transport && m_playbacks == nullptr))
        return fail(tr("Select a cuelist first"));
    const quint32 id = cl->id();

    // the number after CUE (i is past the word), in thousandths
    auto cueNumber = [&](int *number) {
        if (i >= count || t.at(i).kind != Token::Number)
            return fail(tr("Which cue? CUE 1, CUE 2.5…"));
        const Token &token = t.at(i++);
        *number = Cuelist::parseNumber(token.text);
        if (*number <= 0)
            return fail(tr("%1 is not a cue number").arg(token.raw));
        return true;
    };
    // ...of a cue of the list
    auto cueIndex = [&](int *index) {
        int number = -1;
        if (cueNumber(&number) == false)
            return false;
        *index = cl->indexOfNumber(number);
        if (*index < 0)
            return fail(tr("There is no cue %1 in %2").arg(Cuelist::numberToString(number), cl->name()));
        return true;
    };
    auto cueText = [cl](int index) { return Cuelist::numberToString(cl->cueAt(index).number); };

    // what the editor says while it works: why it couldn't, or a warning
    QString said;
    const QMetaObject::Connection listening =
        connect(m_cuelists, &CuelistEditor::message, this, [&said](const QString &text) { said = text; });
    const auto stopListening = qScopeGuard([&listening] { QObject::disconnect(listening); });
    auto editorFailed = [&]() { return fail(said.isEmpty() ? tr("%1 could not be changed").arg(cl->name()) : said); };

    if (word == QLatin1String("GO"))
    {
        if (isWord(i, "CUE"))
        {
            i++;
            int index = -1;
            if (cueIndex(&index) == false)
                return false;
            m_playbacks->goToFunction(int(id), index);
            done->append(tr("GO to cue %1 of %2").arg(cueText(index), cl->name()));
            return true;
        }
        if (cl->cueCount() == 0)
            return fail(tr("%1 has no cues").arg(cl->name()));
        m_playbacks->goFunction(int(id));
        done->append(tr("GO %1").arg(cl->name()));
        return true;
    }

    if (word == QLatin1String("BACK"))
    {
        if (cl->stopped())
            return fail(tr("%1 is not playing").arg(cl->name()));
        m_playbacks->backFunction(int(id));
        done->append(tr("BACK %1").arg(cl->name()));
        return true;
    }

    // CUE 3: what the stage shows on it, in the programmer to edit it
    if (word == QLatin1String("CUE"))
    {
        int index = -1;
        if (cueIndex(&index) == false)
            return false;
        if (m_cuelists->loadCue(id, index, true) == false)
            return editorFailed();
        done->append(tr("Cue %1 of %2 in the programmer").arg(cueText(index), cl->name()));
        return true;
    }

    if (word == QLatin1String("RECORD"))
    {
        // the next whole number, or a number that is free
        QString number;
        if (isWord(i, "CUE"))
        {
            i++;
            int n = -1;
            if (cueNumber(&n) == false)
                return false;
            number = Cuelist::numberToString(n);
            if (cl->indexOfNumber(n) >= 0)
                return fail(tr("Cue %1 already exists: use UPDATE CUE %1").arg(number));
        }
        QVariantMap opts;
        opts.insert("existing", "ask");
        const int index = m_cuelists->recordCue(id, number, opts);
        if (index < 0)
            return editorFailed();
        done->append(tr("Cue %1 recorded in %2").arg(cueText(index), cl->name()));
        // (a cue with no changes is recorded, with a warning)
        if (said.isEmpty() == false)
            done->append(said);
        // like Record on a playback: the programmer is cleared when the settings say so
        if (m_playbacks != nullptr)
            m_playbacks->afterRecord();
        return true;
    }

    if (word == QLatin1String("UPDATE"))
    {
        int index = -1;
        if (isWord(i, "CUE"))
        {
            i++;
            if (cueIndex(&index) == false)
                return false;
        }
        else
        {
            // the cue loaded from this list, else the cue it plays
            const QVariantMap editing = m_cuelists->editing();
            if (editing.isEmpty() == false && editing.value("id").toUInt() == id)
                index = editing.value("index").toInt();
            else if (cl->stopped() == false)
                index = cl->indexOfUid(cl->status().currentUid);
            if (index < 0 || index >= cl->cueCount())
                return fail(tr("Load a cue of %1 (CUE 3) or play it first").arg(cl->name()));
        }
        if (m_cuelists->updateCue(id, index, QVariantMap()) == false)
            return editorFailed();
        done->append(tr("Cue %1 updated in %2").arg(cueText(index), cl->name()));
        if (m_playbacks != nullptr)
            m_playbacks->afterRecord();
        return true;
    }

    // what is left is DELETE CUE 4: what the cue changes moves into the next
    // cue, which looks the same
    if (isWord(i, "CUE") == false)
        return fail(tr("Which cue? DELETE CUE 4"));
    i++;
    int index = -1;
    if (cueIndex(&index) == false)
        return false;
    const QString deleted = cueText(index);
    if (m_cuelists->removeCues(id, QVariantList({ index }), true) == false)
        return editorFailed();
    done->append(tr("Cue %1 deleted from %2").arg(deleted, cl->name()));
    return true;
}

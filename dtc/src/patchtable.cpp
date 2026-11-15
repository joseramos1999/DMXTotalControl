/*
  DMX Total Control
  patchtable.cpp

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

#include <QRegularExpression>
#include <QStringDecoder>
#include <QHash>
#include <QSet>
#include <cmath>

#include "patchtable.h"
#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"

/* How alike the words of two model names must be to take one for the other */
static const qreal kModelSimilarity = 0.6;
/* ...and the words of two mode names */
static const qreal kModeSimilarity = 0.5;

static const QString kGdtfSuffix = QStringLiteral(" (GDTF)");

PatchTable::Table PatchTable::read(const QByteArray &data, const QString &fileName)
{
    // (one format for now: the table as CSV, whatever the file is called)
    Q_UNUSED(fileName)
    return readCsv(data);
}

/*********************************************************************
 * CSV
 *********************************************************************/

QString PatchTable::decode(const QByteArray &data)
{
    if (data.startsWith("\xEF\xBB\xBF"))
        return QString::fromUtf8(data.mid(3));
    if (data.startsWith("\xFF\xFE"))
        return QStringDecoder(QStringDecoder::Utf16LE)(data.mid(2));
    if (data.startsWith("\xFE\xFF"))
        return QStringDecoder(QStringDecoder::Utf16BE)(data.mid(2));

    // not UTF-8: an old export in the codepage of Windows
    QStringDecoder utf8(QStringDecoder::Utf8);
    const QString text = utf8(data);
    if (utf8.hasError())
        return QString::fromLatin1(data);
    return text;
}

QChar PatchTable::separator(const QString &text)
{
    int commas = 0, semicolons = 0, tabs = 0;
    bool quoted = false;
    for (const QChar c : text)
    {
        if (c == QLatin1Char('"'))
            quoted = !quoted;
        else if (quoted)
            continue;
        else if (c == QLatin1Char('\n') || c == QLatin1Char('\r'))
            break;
        else if (c == QLatin1Char(','))
            commas++;
        else if (c == QLatin1Char(';'))
            semicolons++;
        else if (c == QLatin1Char('\t'))
            tabs++;
    }
    if (semicolons > commas && semicolons >= tabs)
        return QLatin1Char(';');
    if (tabs > commas && tabs > semicolons)
        return QLatin1Char('\t');
    return QLatin1Char(',');
}

QList<QStringList> PatchTable::parseCsv(const QString &text, QChar separator, QList<int> *lines, int maxRecords)
{
    QList<QStringList> records;
    QStringList record;
    QString field;
    bool quoted = false;
    // nothing in the field yet: a quote there opens a quoted field
    bool fieldStart = true;
    bool anything = false;
    int line = 1;
    int recordLine = 1;

    auto endRecord = [&]() {
        record.append(field);
        records.append(record);
        if (lines != nullptr)
            lines->append(recordLine);
        record.clear();
        field.clear();
        fieldStart = true;
        anything = false;
    };

    auto full = [&]() { return maxRecords > 0 && records.count() >= maxRecords; };
    const qsizetype n = text.size();
    for (qsizetype i = 0; i < n && full() == false; i++)
    {
        const QChar c = text.at(i);
        if (quoted)
        {
            if (c == QLatin1Char('"'))
            {
                if (i + 1 < n && text.at(i + 1) == QLatin1Char('"'))
                {
                    field += c;
                    i++;
                }
                else
                {
                    quoted = false;
                }
            }
            else if (c == QLatin1Char('\r'))
            {
                // a line break inside the quotes, as "\n"
                line++;
                if (i + 1 < n && text.at(i + 1) == QLatin1Char('\n'))
                    i++;
                field += QLatin1Char('\n');
            }
            else
            {
                if (c == QLatin1Char('\n'))
                    line++;
                field += c;
            }
            continue;
        }

        if (c == QLatin1Char('"') && fieldStart)
        {
            quoted = true;
            fieldStart = false;
            anything = true;
        }
        else if (c == separator)
        {
            record.append(field);
            field.clear();
            fieldStart = true;
            anything = true;
        }
        else if (c == QLatin1Char('\n') || c == QLatin1Char('\r'))
        {
            if (c == QLatin1Char('\r') && i + 1 < n && text.at(i + 1) == QLatin1Char('\n'))
                i++;
            endRecord();
            line++;
            recordLine = line;
        }
        else
        {
            // (a quote in the middle of a field without quotes: as it is)
            field += c;
            fieldStart = false;
            anything = true;
        }
    }
    if (full() == false && (anything || field.isEmpty() == false || record.isEmpty() == false))
        endRecord();
    return records;
}

/*********************************************************************
 * Values with units
 *********************************************************************/

/* "1,5", "1.5", "1.234,5", "1,234.5": a decimal comma or point, the other
 * one (or a repeated one) a thousands separator */
static bool toNumber(QString digits, double *value)
{
    const qsizetype comma = digits.lastIndexOf(QLatin1Char(','));
    const qsizetype point = digits.lastIndexOf(QLatin1Char('.'));
    QChar decimal;
    if (comma >= 0 && point >= 0)
        decimal = comma > point ? QLatin1Char(',') : QLatin1Char('.');
    else if (comma >= 0)
        decimal = digits.count(QLatin1Char(',')) == 1 ? QLatin1Char(',') : QChar();
    else if (point >= 0)
        decimal = digits.count(QLatin1Char('.')) == 1 ? QLatin1Char('.') : QChar();

    QString plain;
    for (const QChar c : std::as_const(digits))
    {
        if (c == decimal)
            plain += QLatin1Char('.');
        else if (c != QLatin1Char(',') && c != QLatin1Char('.'))
            plain += c;
    }
    bool ok = false;
    const double v = plain.toDouble(&ok);
    if (ok == false || std::isfinite(v) == false)
        return false;
    *value = v;
    return true;
}

/* No spaces, no-break spaces (the thousands of some locales) or a sign of
 * another kind ("−") */
static QString compact(const QString &text)
{
    QString out;
    for (const QChar c : text)
    {
        if (c.isSpace() || c == QChar(0x00A0) || c == QChar(0x202F))
            continue;
        out += c == QChar(0x2212) ? QLatin1Char('-') : c;
    }
    return out.toLower();
}

bool PatchTable::parseNumber(const QString &text, double *value)
{
    static const QRegularExpression number(QStringLiteral("^([-+]?[0-9]+(?:[.,][0-9]+)*|[-+]?[.,][0-9]+)"));
    const QRegularExpressionMatch m = number.match(compact(text));
    if (m.hasMatch() == false)
        return false;
    return toNumber(m.captured(1), value);
}

bool PatchTable::parseLength(const QString &text, double *mm)
{
    const QString t = compact(text);
    if (t.isEmpty())
        return false;

    // feet and inches: 3'4"
    static const QRegularExpression feetInches(
        QStringLiteral("^([-+]?)([0-9]+(?:[.,][0-9]+)?)(?:'|ft)([0-9]+(?:[.,][0-9]+)?)(?:\"|in)$"));
    QRegularExpressionMatch m = feetInches.match(t);
    if (m.hasMatch())
    {
        double feet = 0, inches = 0;
        if (toNumber(m.captured(2), &feet) == false || toNumber(m.captured(3), &inches) == false)
            return false;
        const double length = feet * 304.8 + inches * 25.4;
        *mm = m.captured(1) == QLatin1String("-") ? -length : length;
        return true;
    }

    static const QRegularExpression withUnit(
        QStringLiteral("^([-+]?[0-9]+(?:[.,][0-9]+)*|[-+]?[.,][0-9]+)(mm|cm|m|ft|feet|foot|'|in|inch|inches|\"|)$"));
    m = withUnit.match(t);
    if (m.hasMatch() == false)
        return false;
    double value = 0;
    if (toNumber(m.captured(1), &value) == false)
        return false;
    const QString unit = m.captured(2);
    if (unit == QLatin1String("mm"))
        *mm = value;
    else if (unit == QLatin1String("cm"))
        *mm = value * 10.0;
    else if (unit == QLatin1String("ft") || unit == QLatin1String("feet") || unit == QLatin1String("foot") ||
             unit == QLatin1String("'"))
        *mm = value * 304.8;
    else if (unit == QLatin1String("in") || unit == QLatin1String("inch") || unit == QLatin1String("inches") ||
             unit == QLatin1String("\""))
        *mm = value * 25.4;
    else
        *mm = value * 1000.0;
    return true;
}

int PatchTable::parseYesNo(const QString &text)
{
    static const QSet<QString> yes = {
        "yes", "y", "true", "1", "x", "on", "sí", "si", "sì", "oui", "ja", "sim", "tak"
    };
    static const QSet<QString> no = {
        "no", "n", "false", "0", "off", "non", "nein", "não", "nao", "nee", "nie"
    };
    const QString t = text.trimmed().toLower();
    if (yes.contains(t))
        return 1;
    if (no.contains(t))
        return 0;
    return -1;
}

bool PatchTable::upsideDown(const QVector3D &rotation)
{
    // the height of its top after the turns about x and z, in either order
    // (a turn about y, round the vertical, doesn't move it up or down)
    const double toRadians = M_PI / 180.0;
    const double up = std::cos(double(rotation.x()) * toRadians) * std::cos(double(rotation.z()) * toRadians);
    return up < -0.01;
}

/*********************************************************************
 * The table
 *********************************************************************/

PatchTable::Table PatchTable::readCsv(const QByteArray &data)
{
    Table table;
    static const QStringList required = { QStringLiteral("Fixture"), QStringLiteral("DMX Universe"),
                                          QStringLiteral("DMX Channel") };
    // not text at all (an archive, a picture): none of the columns, before
    // reading it all (UTF-16 has zero bytes of its own)
    if (data.startsWith("\xFF\xFE") == false && data.startsWith("\xFE\xFF") == false &&
        data.left(4096).contains('\0'))
    {
        table.error = MissingColumns;
        table.missingColumns = required;
        return table;
    }
    QString text = decode(data);
    if (text.startsWith(QChar(0xFEFF)))
        text.remove(0, 1);
    if (text.trimmed().isEmpty())
    {
        table.error = Empty;
        return table;
    }

    // the header first: the rest is read only when it is the table
    const QChar sep = separator(text);
    const QList<QStringList> first = parseCsv(text, sep, nullptr, 1);
    if (first.isEmpty())
    {
        table.error = Empty;
        return table;
    }

    // the columns by their English names (in any case)
    QHash<QString, int> columns;
    const QStringList header = first.first();
    for (int i = 0; i < header.count(); i++)
    {
        const QString name = header.at(i).trimmed().toLower();
        if (name.isEmpty() == false && columns.contains(name) == false)
            columns.insert(name, i);
    }
    for (const QString &column : required)
    {
        if (columns.contains(column.toLower()) == false)
            table.missingColumns.append(column);
    }
    if (table.missingColumns.isEmpty() == false)
    {
        table.error = MissingColumns;
        return table;
    }

    QList<int> lines;
    const QList<QStringList> records = parseCsv(text, sep, &lines);
    const bool hasInvertPan = columns.contains("invert pan");
    const bool hasInvertTilt = columns.contains("invert tilt");
    for (int r = 1; r < records.count(); r++)
    {
        const QStringList &record = records.at(r);
        auto cell = [&](const char *name) -> QString {
            const int index = columns.value(QString::fromLatin1(name), -1);
            return index >= 0 && index < record.count() ? record.at(index).trimmed() : QString();
        };
        auto integer = [&](const char *name) -> int {
            double value = 0;
            const QString t = cell(name);
            if (parseNumber(t, &value) == false || value < 1 || value > 1e6 || value != std::floor(value))
                return 0;
            return int(value);
        };

        // an empty line
        bool empty = true;
        for (const QString &value : record)
            empty = empty && value.trimmed().isEmpty();
        if (empty)
            continue;

        Row row;
        row.line = lines.value(r, r + 1);
        row.fixture = cell("fixture");
        row.mode = cell("dmx mode");
        row.channels = integer("dmx channels");
        row.universeText = cell("dmx universe");
        row.addressText = cell("dmx channel");
        row.universe = integer("dmx universe");
        row.address = integer("dmx channel");
        row.unit = cell("unit");
        row.number = integer("channel");
        if (row.number == 0)
            row.number = integer("p3 fixture number");
        row.layer = cell("layer");
        for (const QString &group : cell("groups").split(QLatin1Char(','), Qt::SkipEmptyParts))
        {
            const QString name = group.trimmed();
            if (name.isEmpty() == false && row.groups.contains(name) == false)
                row.groups.append(name);
        }

        double x = 0, y = 0, z = 0;
        if (parseLength(cell("position x"), &x) && parseLength(cell("position y"), &y) &&
            parseLength(cell("position z"), &z))
        {
            row.hasPosition = true;
            row.position = QVector3D(float(x), float(y), float(z));
        }

        // how it is turned: the columns given, an empty one at 0
        double rx = 0, ry = 0, rz = 0;
        const bool hasRx = parseNumber(cell("rotation x"), &rx);
        const bool hasRy = parseNumber(cell("rotation y"), &ry);
        const bool hasRz = parseNumber(cell("rotation z"), &rz);
        if (hasRx || hasRy || hasRz)
        {
            row.hasRotation = true;
            row.rotation = QVector3D(float(rx), float(ry), float(rz));
        }

        const QString panText = hasInvertPan ? cell("invert pan") : QString();
        const QString tiltText = hasInvertTilt ? cell("invert tilt") : QString();
        const int pan = parseYesNo(panText);
        const int tilt = parseYesNo(tiltText);
        row.invertPanGiven = pan >= 0;
        row.invertPan = pan == 1;
        row.invertTiltGiven = tilt >= 0;
        row.invertTilt = tilt == 1;
        // (a word the parser doesn't know: said, not taken as a no)
        if (pan < 0)
            row.invertPanText = panText;
        if (tilt < 0)
            row.invertTiltText = tiltText;
        row.identifier = cell("identifier");

        if (cell("dmx universe #2").isEmpty() == false || cell("dmx channel #2").isEmpty() == false)
        {
            row.secondBreak = true;
            row.secondUniverse = integer("dmx universe #2");
            row.secondAddress = integer("dmx channel #2");
        }

        if (row.layer.isEmpty() == false && table.layers.contains(row.layer) == false)
            table.layers.append(row.layer);
        table.hasGroups = table.hasGroups || row.groups.isEmpty() == false;
        table.rows.append(row);
    }
    return table;
}

/*********************************************************************
 * Matching
 *********************************************************************/

QString PatchTable::normalised(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (const QChar c : text)
    {
        if (c.isLetterOrNumber())
            out += c.toLower();
    }
    return out;
}

QStringList PatchTable::tokens(const QString &text)
{
    QStringList list;
    QString word;
    int kind = 0; // 1: letters, 2: digits
    auto flush = [&]() {
        if (word.isEmpty())
            return;
        // the same word written several ways
        if (word == QLatin1String("channels") || word == QLatin1String("channel") ||
            word == QLatin1String("chan") || word == QLatin1String("chs"))
            word = QStringLiteral("ch");
        else if (word == QLatin1String("bits"))
            word = QStringLiteral("bit");
        list.append(word);
        word.clear();
    };
    for (const QChar c : text)
    {
        const int k = c.isDigit() ? 2 : (c.isLetter() ? 1 : 0);
        if (k != kind)
            flush();
        kind = k;
        if (k != 0)
            word += c.toLower();
    }
    flush();
    return list;
}

qreal PatchTable::similarity(const QString &a, const QString &b)
{
    const QStringList ta = tokens(a);
    QStringList tb = tokens(b);
    if (ta.isEmpty() || tb.isEmpty())
        return 0;
    const qsizetype total = ta.count() + tb.count();
    int common = 0;
    for (const QString &t : ta)
    {
        const qsizetype i = tb.indexOf(t);
        if (i >= 0)
        {
            common++;
            tb.removeAt(i);
        }
    }
    return 2.0 * common / qreal(total);
}

bool PatchTable::similarModelAllowed(const QString &name, const QString &model)
{
    // the words that tell one kind of fixture of a family from another
    static const QStringList kinds = { "wash", "spot", "profile", "beam", "hybrid", "blinder", "strobe" };
    const QStringList library = tokens(name);
    const QStringList table = tokens(model);
    for (const QString &word : library)
    {
        // another number: another fixture ("Spot 300 Mk2" is still a "Spot 300")
        if (word.at(0).isDigit() && table.contains(word) == false)
            return false;
    }
    for (const QString &kind : kinds)
    {
        if (library.contains(kind) != table.contains(kind))
            return false;
    }
    return true;
}

QString PatchTable::typeKey(const QString &fixture, const QString &mode, int channels)
{
    return normalised(fixture) + QLatin1Char('|') + normalised(mode) + QLatin1Char('|') + QString::number(channels);
}

int PatchTable::matchMode(const QList<ModeInfo> &modes, const QString &name, int channels, How *how)
{
    auto result = [how](int index, How h) {
        if (how != nullptr)
            *how = index >= 0 ? h : NotFound;
        return index;
    };
    if (modes.isEmpty())
        return result(-1, NotFound);

    const QString wanted = name.trimmed();
    const QString wantedNorm = normalised(wanted);
    auto sameName = [&](const ModeInfo &mode) {
        return wanted.isEmpty() == false &&
               (mode.name.trimmed().compare(wanted, Qt::CaseInsensitive) == 0 ||
                (wantedNorm.isEmpty() == false && normalised(mode.name) == wantedNorm));
    };
    // the most alike of $candidates (the first one on a tie)
    auto mostAlike = [&](const QList<int> &candidates, qreal *score) {
        int best = -1;
        qreal bestScore = -1;
        for (int i : candidates)
        {
            const qreal s = similarity(modes.at(i).name, wanted);
            if (s > bestScore)
            {
                best = i;
                bestScore = s;
            }
        }
        if (score != nullptr)
            *score = bestScore;
        return best;
    };

    // its name, with its channel count
    for (int i = 0; i < modes.count(); i++)
    {
        if (sameName(modes.at(i)) && (channels <= 0 || modes.at(i).channels == channels))
            return result(i, Exact);
    }
    // the channel count: on the right addresses whatever it is called
    if (channels > 0)
    {
        QList<int> same;
        for (int i = 0; i < modes.count(); i++)
            if (modes.at(i).channels == channels)
                same.append(i);
        if (same.count() == 1)
            return result(same.first(), ByChannels);
        if (same.count() > 1)
            return result(mostAlike(same, nullptr), ByChannels);
    }
    // its name with another count (the caller says so)
    for (int i = 0; i < modes.count(); i++)
    {
        if (sameName(modes.at(i)))
            return result(i, Exact);
    }
    // the most alike, when the table doesn't say how many channels (with
    // another count, the fixture would take the addresses of the next one)
    if (wanted.isEmpty() == false && channels <= 0)
    {
        QList<int> all;
        for (int i = 0; i < modes.count(); i++)
            all.append(i);
        qreal score = 0;
        const int best = mostAlike(all, &score);
        if (best >= 0 && score >= kModeSimilarity)
            return result(best, Similar);
    }
    else if (wanted.isEmpty() && channels <= 0 && modes.count() == 1)
    {
        // nothing said, and it has one
        return result(0, Similar);
    }
    return result(-1, NotFound);
}

bool PatchTable::isGdtfTwin(const QString &model)
{
    return model.endsWith(kGdtfSuffix);
}

PatchTable::Library PatchTable::library(QLCFixtureDefCache *cache)
{
    Library lib;
    if (cache == nullptr)
        return lib;
    for (QLCFixtureDef *def : cache->defs())
        lib.models.append({ def->manufacturer(), def->model() });
    lib.modes = [cache](const QString &manufacturer, const QString &model) {
        QList<ModeInfo> list;
        // (the definitions of the library are read when they are needed)
        QLCFixtureDef *def = cache->fixtureDef(manufacturer, model);
        if (def == nullptr)
            return list;
        for (QLCFixtureMode *mode : def->modes())
            list.append({ mode->name(), int(mode->channels().count()) });
        return list;
    };
    return lib;
}

PatchTable::Match PatchTable::match(const Library &library, const QString &fixture, const QString &mode, int channels)
{
    Match none;
    const QStringList words = fixture.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    if (words.isEmpty() || library.models.isEmpty())
        return none;

    // the manufacturers by their normalised name, and by their first words
    // (the fixtures of GDTF-Share say "Martin Professional", a table "Martin")
    QHash<QString, QStringList> makers;
    QHash<QString, QStringList> longerMakers;
    QSet<QString> seen;
    for (const Library::Model &m : library.models)
    {
        if (seen.contains(m.manufacturer))
            continue;
        seen.insert(m.manufacturer);
        makers[normalised(m.manufacturer)].append(m.manufacturer);
        const QStringList makerWords = m.manufacturer.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        for (qsizetype k = 1; k < makerWords.count(); k++)
            longerMakers[normalised(makerWords.mid(0, k).join(QLatin1Char(' ')))].append(m.manufacturer);
    }

    // the string starts with the manufacturer: the longest one there is
    // (whole words: "Martin" doesn't start "Martini X"), the model after it;
    // then the ones whose name is longer
    struct Split
    {
        QString manufacturer;
        QString model;
        bool longer;
    };
    QList<Split> splits;
    for (qsizetype k = words.count() - 1; k >= 1; k--)
    {
        const QString prefix = normalised(words.mid(0, k).join(QLatin1Char(' ')));
        if (prefix.isEmpty())
            continue;
        for (const QString &maker : makers.value(prefix))
            splits.append({ maker, words.mid(k).join(QLatin1Char(' ')), false });
    }
    for (qsizetype k = words.count() - 1; k >= 1; k--)
    {
        const QString prefix = normalised(words.mid(0, k).join(QLatin1Char(' ')));
        for (const QString &maker : longerMakers.value(prefix))
        {
            if (std::none_of(splits.cbegin(), splits.cend(), [&maker](const Split &s) { return s.manufacturer == maker; }))
                splits.append({ maker, words.mid(k).join(QLatin1Char(' ')), true });
        }
    }
    // none: the whole string may be the model alone (by its name only)
    const bool byNameOnly = splits.isEmpty();
    // (not found: where to look for it by hand)
    if (byNameOnly == false)
        none.foundManufacturer = splits.first().manufacturer;

    struct Candidate
    {
        QString manufacturer;
        QString model;
        qreal score;
        int splitIndex;
    };
    QList<Candidate> candidates;
    qreal best = 0;
    for (const Library::Model &m : library.models)
    {
        // a GDTF twin is compared by the name of its fixture
        const QString name = isGdtfTwin(m.model) ? m.model.chopped(kGdtfSuffix.size()) : m.model;
        const QString nameNorm = normalised(name);
        qreal score = 0;
        int splitIndex = -1;
        if (byNameOnly)
        {
            const QString whole = words.join(QLatin1Char(' '));
            if (name.compare(whole, Qt::CaseInsensitive) == 0)
                score = 1.0;
            else if (nameNorm.isEmpty() == false && nameNorm == normalised(whole))
                score = 0.95;
        }
        else
        {
            for (int s = 0; s < splits.count(); s++)
            {
                if (splits.at(s).manufacturer != m.manufacturer)
                    continue;
                const QString &model = splits.at(s).model;
                qreal sc = 0;
                if (name.compare(model, Qt::CaseInsensitive) == 0)
                    sc = 1.0;
                else if (nameNorm.isEmpty() == false && nameNorm == normalised(model))
                    sc = 0.95;
                else
                {
                    const qreal sim = similarity(name, model);
                    if (sim >= kModelSimilarity && similarModelAllowed(name, model))
                        sc = 0.5 + sim * 0.4;
                }
                if (sc > score)
                {
                    score = sc;
                    splitIndex = s;
                }
            }
        }
        if (score <= 0)
            continue;
        if (score > best + 1e-9)
        {
            best = score;
            candidates.clear();
        }
        if (score >= best - 1e-9)
            candidates.append({ m.manufacturer, m.model, score, splitIndex });
    }
    if (candidates.isEmpty())
        return none;

    // of the ones that match the same: the best mode, then the QLC+
    // fixture before its GDTF twin, then the longest manufacturer (the
    // first split), then the first one
    int chosen = -1;
    int chosenMode = -1;
    How chosenHow = NotFound;
    qreal chosenRank = -1;
    QList<ModeInfo> chosenModes;
    for (int c = 0; c < candidates.count(); c++)
    {
        const QList<ModeInfo> modes = library.modes ? library.modes(candidates.at(c).manufacturer, candidates.at(c).model)
                                                    : QList<ModeInfo>();
        How how = NotFound;
        const int index = matchMode(modes, mode, channels, &how);
        qreal rank = 0;
        if (index >= 0)
        {
            const bool sameCount = channels <= 0 || modes.at(index).channels == channels;
            rank = how == Exact ? (sameCount ? 4 : 2) : how == ByChannels ? 3 : (sameCount ? 1.5 : 1);
        }
        // the QLC+ one first, the manufacturer of the table's name next
        // (the longest one first)
        const int s = candidates.at(c).splitIndex;
        rank -= isGdtfTwin(candidates.at(c).model) ? 0.1 : 0;
        rank -= s >= 0 && splits.at(s).longer ? 0.05 : 0;
        rank -= s > 0 ? 0.001 * qMin(s, 9) : 0;
        if (rank > chosenRank + 1e-9)
        {
            chosen = c;
            chosenRank = rank;
            chosenMode = index;
            chosenHow = how;
            chosenModes = modes;
        }
    }

    const Candidate &c = candidates.at(chosen);
    Match found;
    found.foundManufacturer = c.manufacturer;
    found.foundModel = c.model;
    if (chosenMode < 0)
        return found;
    found.manufacturer = c.manufacturer;
    found.model = c.model;
    found.mode = chosenModes.at(chosenMode).name;
    found.channels = chosenModes.at(chosenMode).channels;
    found.how = c.score < 0.95 ? Similar : chosenHow;
    return found;
}

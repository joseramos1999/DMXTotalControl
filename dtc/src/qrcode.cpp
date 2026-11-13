/*
  DMX Total Control
  qrcode.cpp

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

#include <QPoint>
#include <QList>
#include <cstdlib>

#include "qrcode.h"

namespace
{

const int KMaxVersion = 10;

/** The blocks of a version at level M (the table of the standard): how
 *  many short blocks and their data codewords, how many long ones (one
 *  data codeword more), and the error correction codewords of each block */
struct Blocks
{
    int shortBlocks;
    int shortData;
    int longBlocks;
    int ecCodewords;
};

const Blocks KBlocks[KMaxVersion] = {
    { 1, 16, 0, 10 }, { 1, 28, 0, 16 }, { 1, 44, 0, 26 }, { 2, 32, 0, 18 }, { 2, 43, 0, 24 },
    { 4, 27, 0, 16 }, { 4, 31, 0, 18 }, { 2, 38, 2, 22 }, { 3, 36, 2, 22 }, { 4, 43, 1, 26 },
};

/** The rows (and the columns) of the centres of the alignment patterns of
 *  each version; 0 ends a list (none in version 1) */
const int KAlignment[KMaxVersion][3] = {
    { 0, 0, 0 }, { 6, 18, 0 }, { 6, 22, 0 }, { 6, 26, 0 }, { 6, 30, 0 },
    { 6, 34, 0 }, { 6, 22, 38 }, { 6, 24, 42 }, { 6, 26, 46 }, { 6, 28, 50 },
};

int dataCodewords(int version)
{
    const Blocks &blocks = KBlocks[version - 1];
    return blocks.shortBlocks * blocks.shortData + blocks.longBlocks * (blocks.shortData + 1);
}

/** The count of bytes takes 8 bits up to version 9, 16 from version 10 */
int countBits(int version)
{
    return version < 10 ? 8 : 16;
}

/** a × b in GF(256), whose polynomial is x^8 + x^4 + x^3 + x^2 + 1 (0x11D) */
quint8 multiply(quint8 a, quint8 b)
{
    int product = 0;
    for (int bit = 7; bit >= 0; --bit)
    {
        product = (product << 1) ^ ((product >> 7) * 0x11D);
        if ((b >> bit) & 1)
            product ^= a;
    }
    return quint8(product);
}

/** The data codewords of a version: the byte mode, the count, the bytes,
 *  the terminator (up to 4 zeros), zeros to the end of the byte, then the
 *  pad codewords 0xEC and 0x11 in turn */
QByteArray dataCodewordsOf(const QByteArray &data, int version)
{
    QList<bool> bits;
    auto append = [&bits](int value, int count) {
        for (int bit = count - 1; bit >= 0; --bit)
            bits.append(((value >> bit) & 1) != 0);
    };
    const int total = dataCodewords(version);
    append(0x4, 4);
    append(int(data.size()), countBits(version));
    for (const char byte : data)
        append(quint8(byte), 8);
    append(0, qMin(4, total * 8 - int(bits.size())));
    append(0, (8 - int(bits.size()) % 8) % 8);

    QByteArray codewords;
    for (int at = 0; at < bits.size(); at += 8)
    {
        int byte = 0;
        for (int bit = 0; bit < 8; ++bit)
            byte = (byte << 1) | (bits.at(at + bit) ? 1 : 0);
        codewords.append(char(byte));
    }
    for (bool first = true; codewords.size() < total; first = !first)
        codewords.append(char(first ? 0xEC : 0x11));
    return codewords;
}

/** The codewords as the symbol holds them: the data split in its blocks,
 *  each with its error correction, and both interleaved block by block */
QByteArray interleave(const QByteArray &data, int version)
{
    const Blocks &blocks = KBlocks[version - 1];
    QList<QByteArray> dataBlocks;
    QList<QByteArray> ecBlocks;
    int at = 0;
    for (int block = 0; block < blocks.shortBlocks + blocks.longBlocks; ++block)
    {
        const int length = blocks.shortData + (block < blocks.shortBlocks ? 0 : 1);
        dataBlocks.append(data.mid(at, length));
        ecBlocks.append(QrCode::errorCorrection(dataBlocks.last(), blocks.ecCodewords));
        at += length;
    }
    QByteArray codewords;
    for (int i = 0; i <= blocks.shortData; ++i)
        for (const QByteArray &block : std::as_const(dataBlocks))
            if (i < block.size())
                codewords.append(block.at(i));
    for (int i = 0; i < blocks.ecCodewords; ++i)
        for (const QByteArray &block : std::as_const(ecBlocks))
            codewords.append(block.at(i));
    return codewords;
}

/** Whether a mask of the standard flips the module at column x, row y */
bool masked(int mask, int x, int y)
{
    switch (mask)
    {
        case 0: return (x + y) % 2 == 0;
        case 1: return y % 2 == 0;
        case 2: return x % 3 == 0;
        case 3: return (x + y) % 3 == 0;
        case 4: return (y / 2 + x / 3) % 2 == 0;
        case 5: return (x * y) % 2 + (x * y) % 3 == 0;
        case 6: return ((x * y) % 2 + (x * y) % 3) % 2 == 0;
        default: return ((x + y) % 2 + (x * y) % 3) % 2 == 0;
    }
}

/** Where the 15 format bits go, bit 0 first (x: column, y: row): the copy
 *  around the finder of the top left (0), or the one split under the
 *  finder of the top right and beside the one of the bottom left (1) */
QList<QPoint> formatPositions(int size, int copy)
{
    QList<QPoint> at;
    if (copy == 0)
    {
        // down column 8 (the timing row left out), then left along row 8
        for (int y = 0; y <= 5; ++y)
            at.append(QPoint(8, y));
        at.append(QPoint(8, 7));
        at.append(QPoint(8, 8));
        at.append(QPoint(7, 8));
        for (int x = 5; x >= 0; --x)
            at.append(QPoint(x, 8));
    }
    else
    {
        // left along row 8 from the right edge, then down column 8 to the bottom
        for (int x = size - 1; x >= size - 8; --x)
            at.append(QPoint(x, 8));
        for (int y = size - 7; y < size; ++y)
            at.append(QPoint(8, y));
    }
    return at;
}

/** Where the 18 version bits go, bit 0 first: the block of 6 × 3 above
 *  the finder of the bottom left (0), or its mirror, left of the finder of
 *  the top right (1) */
QList<QPoint> versionPositions(int size, int copy)
{
    QList<QPoint> at;
    for (int bit = 0; bit < 18; ++bit)
    {
        const int across = bit / 3;
        const int along = size - 11 + bit % 3;
        at.append(copy == 0 ? QPoint(across, along) : QPoint(along, across));
    }
    return at;
}

/** A symbol while it is made: its modules, and which of them the function
 *  patterns take (the data goes around them, the mask leaves them alone) */
class Symbol
{
public:
    explicit Symbol(int version)
        : m_version(version)
        , m_size(4 * version + 17)
        , m_dark(m_size * m_size, 0)
        , m_function(m_size * m_size, 0)
    {
        drawFunctionPatterns();
    }

    /** The codewords, their bits from the highest, in the zigzag of the
     *  standard: column pairs from the right, up and down in turn, around
     *  the function patterns. The remainder bits stay light */
    void drawCodewords(const QByteArray &codewords)
    {
        const int bits = int(codewords.size()) * 8;
        int bit = 0;
        for (int right = m_size - 1; right >= 1; right -= 2)
        {
            // (the timing column has no pair)
            if (right == 6)
                right = 5;
            const bool upwards = ((right + 1) & 2) == 0;
            for (int step = 0; step < m_size; ++step)
            {
                const int y = upwards ? m_size - 1 - step : step;
                for (int x = right; x >= right - 1; --x)
                {
                    if (isFunction(x, y) || bit >= bits)
                        continue;
                    m_dark[index(x, y)] = char((quint8(codewords.at(bit >> 3)) >> (7 - (bit & 7))) & 1);
                    ++bit;
                }
            }
        }
    }

    /** The modules with a mask, and its format bits */
    QByteArray withMask(int mask) const
    {
        QByteArray modules = m_dark;
        for (int y = 0; y < m_size; ++y)
            for (int x = 0; x < m_size; ++x)
                if (isFunction(x, y) == false && masked(mask, x, y))
                    modules[index(x, y)] = char(modules.at(index(x, y)) ^ 1);
        const quint16 format = QrCode::formatBits(mask);
        for (int copy = 0; copy < 2; ++copy)
        {
            const QList<QPoint> at = formatPositions(m_size, copy);
            for (int bit = 0; bit < 15; ++bit)
                modules[index(at.at(bit).x(), at.at(bit).y())] = char((format >> bit) & 1);
        }
        return modules;
    }

private:
    int index(int x, int y) const
    {
        return y * m_size + x;
    }

    bool isFunction(int x, int y) const
    {
        return m_function.at(index(x, y)) != 0;
    }

    void setFunction(int x, int y, bool dark)
    {
        m_dark[index(x, y)] = dark ? 1 : 0;
        m_function[index(x, y)] = 1;
    }

    void drawFunctionPatterns()
    {
        // the timing patterns along row 6 and column 6: dark on the even
        // modules (the finders cover their ends)
        for (int i = 0; i < m_size; ++i)
        {
            setFunction(6, i, i % 2 == 0);
            setFunction(i, 6, i % 2 == 0);
        }
        // the finders at three corners
        drawFinder(3, 3);
        drawFinder(m_size - 4, 3);
        drawFinder(3, m_size - 4);
        // the alignment patterns, but where they would cover a finder
        const int *centres = KAlignment[m_version - 1];
        const int count = centres[0] == 0 ? 0 : (centres[2] == 0 ? 2 : 3);
        for (int i = 0; i < count; ++i)
            for (int j = 0; j < count; ++j)
            {
                const bool atFinder = (i == 0 && j == 0) || (i == 0 && j == count - 1) || (i == count - 1 && j == 0);
                if (atFinder == false)
                    drawAlignment(centres[i], centres[j]);
            }
        // the format areas (their bits come with the mask) and the module
        // that is always dark
        for (int copy = 0; copy < 2; ++copy)
            for (const QPoint &at : formatPositions(m_size, copy))
                setFunction(at.x(), at.y(), false);
        setFunction(8, m_size - 8, true);
        // the version, from version 7
        if (m_version >= 7)
        {
            const quint32 version = QrCode::versionBits(m_version);
            for (int copy = 0; copy < 2; ++copy)
            {
                const QList<QPoint> at = versionPositions(m_size, copy);
                for (int bit = 0; bit < 18; ++bit)
                    setFunction(at.at(bit).x(), at.at(bit).y(), ((version >> bit) & 1) != 0);
            }
        }
    }

    /** A finder centred at x, y: dark 7 × 7 ring, light ring, dark 3 × 3,
     *  and the light separator around it (the part inside the symbol) */
    void drawFinder(int cx, int cy)
    {
        for (int dy = -4; dy <= 4; ++dy)
            for (int dx = -4; dx <= 4; ++dx)
            {
                const int x = cx + dx;
                const int y = cy + dy;
                if (x < 0 || y < 0 || x >= m_size || y >= m_size)
                    continue;
                const int ring = qMax(std::abs(dx), std::abs(dy));
                setFunction(x, y, ring != 2 && ring != 4);
            }
    }

    /** An alignment pattern centred at x, y: dark 5 × 5 ring, light ring,
     *  dark centre */
    void drawAlignment(int cx, int cy)
    {
        for (int dy = -2; dy <= 2; ++dy)
            for (int dx = -2; dx <= 2; ++dx)
                setFunction(cx + dx, cy + dy, qMax(std::abs(dx), std::abs(dy)) != 1);
    }

private:
    int m_version;
    int m_size;
    QByteArray m_dark;
    QByteArray m_function;
};

}

/*********************************************************************
 * Making it
 *********************************************************************/

QrCode::QrCode(int version, int mask, const QByteArray &modules)
    : m_version(version)
    , m_mask(mask)
    , m_modules(modules)
{
}

QrCode QrCode::encode(const QByteArray &data, int mask)
{
    int version = 1;
    while (version <= KMaxVersion && capacity(version) < data.size())
        ++version;
    if (version > KMaxVersion || mask > 7)
        return QrCode();

    Symbol symbol(version);
    symbol.drawCodewords(interleave(dataCodewordsOf(data, version), version));
    if (mask >= 0)
        return QrCode(version, mask, symbol.withMask(mask));

    // the mask the standard prefers: the lowest penalty, the first on ties
    QrCode best;
    int lowest = 0;
    for (int candidate = 0; candidate < 8; ++candidate)
    {
        const QrCode code(version, candidate, symbol.withMask(candidate));
        const int points = penalty(code);
        if (best.isNull() || points < lowest)
        {
            best = code;
            lowest = points;
        }
    }
    return best;
}

/*********************************************************************
 * The modules
 *********************************************************************/

bool QrCode::isNull() const
{
    return m_version == 0;
}

int QrCode::size() const
{
    return isNull() ? 0 : 4 * m_version + 17;
}

int QrCode::version() const
{
    return m_version;
}

int QrCode::mask() const
{
    return m_mask;
}

bool QrCode::dark(int x, int y) const
{
    const int n = size();
    if (x < 0 || y < 0 || x >= n || y >= n)
        return false;
    return m_modules.at(y * n + x) != 0;
}

QStringList QrCode::rows() const
{
    QStringList rows;
    const int n = size();
    for (int y = 0; y < n; ++y)
    {
        QString row(n, QLatin1Char('0'));
        for (int x = 0; x < n; ++x)
            if (dark(x, y))
                row[x] = QLatin1Char('1');
        rows.append(row);
    }
    return rows;
}

/*********************************************************************
 * The parts of the standard
 *********************************************************************/

int QrCode::capacity(int version)
{
    if (version < 1 || version > KMaxVersion)
        return 0;
    // (the mode takes 4 bits, the count 8 or 16)
    return (dataCodewords(version) * 8 - 4 - countBits(version)) / 8;
}

QByteArray QrCode::errorCorrection(const QByteArray &data, int ecCount)
{
    if (ecCount <= 0)
        return QByteArray();
    // the generator (x - α^0)(x - α^1)…(x - α^(n-1)), α = 2: its
    // coefficients from the highest power, without the first (always 1)
    QByteArray generator(ecCount, 0);
    generator[ecCount - 1] = 1;
    quint8 root = 1;
    for (int i = 0; i < ecCount; ++i)
    {
        for (int j = 0; j < ecCount; ++j)
        {
            quint8 coefficient = multiply(quint8(generator.at(j)), root);
            if (j + 1 < ecCount)
                coefficient ^= quint8(generator.at(j + 1));
            generator[j] = char(coefficient);
        }
        root = multiply(root, 2);
    }
    // the remainder of data × x^n divided by it
    QByteArray remainder(ecCount, 0);
    for (const char byte : data)
    {
        const quint8 factor = quint8(byte) ^ quint8(remainder.at(0));
        remainder.remove(0, 1);
        remainder.append('\0');
        for (int j = 0; j < ecCount; ++j)
            remainder[j] = char(quint8(remainder.at(j)) ^ multiply(quint8(generator.at(j)), factor));
    }
    return remainder;
}

quint16 QrCode::formatBits(int mask)
{
    // the level M is 00: the 5 bits of data are the mask; BCH(15,5)
    const int data = mask & 7;
    int remainder = data;
    for (int i = 0; i < 10; ++i)
        remainder = (remainder << 1) ^ ((remainder >> 9) * 0x537);
    return quint16(((data << 10) | remainder) ^ 0x5412);
}

quint32 QrCode::versionBits(int version)
{
    // BCH(18,6)
    int remainder = version;
    for (int i = 0; i < 12; ++i)
        remainder = (remainder << 1) ^ ((remainder >> 11) * 0x1F25);
    return quint32((version << 12) | remainder);
}

int QrCode::penalty(const QrCode &code)
{
    const int n = code.size();
    int points = 0;
    // each line, across and down: 3 for a run of 5 modules of a colour and
    // 1 more for each one past 5; 40 for each pattern like a finder
    // (1:1:3:1:1) with 4 light modules before or after it
    static const char *const finderLike[] = { "10111010000", "00001011101" };
    for (int line = 0; line < n; ++line)
        for (const bool down : { false, true })
        {
            QByteArray modules;
            for (int i = 0; i < n; ++i)
                modules.append(code.dark(down ? line : i, down ? i : line) ? '1' : '0');
            int run = 1;
            for (int i = 1; i <= n; ++i)
            {
                if (i < n && modules.at(i) == modules.at(i - 1))
                {
                    ++run;
                    continue;
                }
                if (run >= 5)
                    points += 3 + run - 5;
                run = 1;
            }
            for (const char *pattern : finderLike)
                for (qsizetype at = modules.indexOf(pattern); at >= 0; at = modules.indexOf(pattern, at + 1))
                    points += 40;
        }
    // 3 for each block of 2 × 2 of a colour
    for (int y = 0; y + 1 < n; ++y)
        for (int x = 0; x + 1 < n; ++x)
        {
            const bool colour = code.dark(x, y);
            if (code.dark(x + 1, y) == colour && code.dark(x, y + 1) == colour && code.dark(x + 1, y + 1) == colour)
                points += 3;
        }
    // 10 for each 5 % the dark modules are away from half of them
    int dark = 0;
    for (const char module : code.m_modules)
        dark += module;
    const int total = n * n;
    if (total > 0)
        points += 10 * (std::abs(dark * 20 - total * 10) / total);
    return points;
}

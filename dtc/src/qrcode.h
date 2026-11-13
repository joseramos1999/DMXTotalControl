/*
  DMX Total Control
  qrcode.h

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

#ifndef QRCODE_H
#define QRCODE_H

#include <QByteArray>
#include <QStringList>

/**
 * A QR code of some bytes, as ISO/IEC 18004 makes it: byte mode, error
 * correction level M (15 % of the code can be lost), versions 1 to 10 (21
 * to 57 modules a side, up to 213 bytes). Enough for the address of the
 * remote control, which a phone opens with its camera.
 *
 * Its modules are given without the quiet zone: whoever draws it leaves 4
 * light modules all around.
 */
class QrCode
{
public:
    /** A null code */
    QrCode() = default;

    /** The code of $data with this mask (0 to 7), or with the one the
     *  standard prefers (-1: the lowest penalty, the first on ties). Null
     *  when the data doesn't fit (more than 213 bytes) */
    static QrCode encode(const QByteArray &data, int mask = -1);

    bool isNull() const;
    /** Modules a side: 4 × version + 17 (0 when null) */
    int size() const;
    /** 1 to 10 (0 when null) */
    int version() const;
    /** 0 to 7 (-1 when null) */
    int mask() const;
    /** Whether the module at column $x and row $y is dark (false outside) */
    bool dark(int x, int y) const;
    /** The modules row by row, top first: '1' dark, '0' light */
    QStringList rows() const;

    /** The bytes a version holds in byte mode at level M (0 for a version
     *  out of 1 to 10) */
    static int capacity(int version);
    /** The Reed-Solomon error correction of $data: the $ecCount codewords
     *  of the remainder of its division by the generator of the standard */
    static QByteArray errorCorrection(const QByteArray &data, int ecCount);
    /** The 15 format bits of a mask at level M, masked with 0x5412 */
    static quint16 formatBits(int mask);
    /** The 18 version bits of the versions 7 and up */
    static quint32 versionBits(int version);
    /** How hard the modules are to read, by the rules of the standard (runs
     *  of a colour, blocks, patterns like the finders, the balance of dark
     *  and light): the code takes the mask with the lowest */
    static int penalty(const QrCode &code);

private:
    QrCode(int version, int mask, const QByteArray &modules);

private:
    int m_version = 0;
    int m_mask = -1;
    /** Row by row: 1 dark, 0 light */
    QByteArray m_modules;
};

#endif

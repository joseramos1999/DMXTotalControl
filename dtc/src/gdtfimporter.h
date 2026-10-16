/*
  DMX Total Control
  gdtfimporter.h

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

#ifndef GDTFIMPORTER_H
#define GDTFIMPORTER_H

#include <QByteArray>
#include <QStringList>
#include <QDir>

#include <functional>

class QLCFixtureDef;
class QZipReader;

/**
 * GDTF (General Device Type Format, DIN SPEC 15800): the open fixture format
 * the manufacturers publish on GDTF-Share. A .gdtf file is a zip with a
 * description.xml (the fixture type, its wheels, geometries and DMX modes)
 * and the pictures of its wheels.
 *
 * It becomes a QLC+ definition (.qxf), so the shows stay readable by QLC+:
 * - each DMX channel, with its fine channels (16 and 24 bit);
 * - channel functions and channel sets as capabilities, with the colours of
 *   the colour wheels (CIE xyY) and the pictures of the gobos;
 * - geometry references (the cells of a bar or a matrix) as heads;
 * - the attributes (Dimmer, Pan, ColorAdd_R, Gobo1, Shutter1...) as QLC+
 *   groups, colours and presets, so the programmer knows what they do;
 * - beam angle, power, lamp and size.
 *
 * Only the first DMX break of a mode is used (the engine patches a fixture
 * on one address); virtual channels (without DMX) are left out.
 *
 * Everything here works without the UI and in any thread.
 */
class GdtfImporter
{
public:
    struct Result
    {
        bool ok = false;
        QString manufacturer;
        QString model;
        /** The .qxf written */
        QString file;
        QString error;
        QStringList warnings;
    };

    /**
     * Convert a .gdtf (its bytes) into a .qxf in $target, its wheel pictures in
     * $target/wheels. $taken says if a manufacturer and model are already in
     * another library: then the model gets " (GDTF)", so both can be used.
     */
    static Result convert(const QByteArray &gdtf, const QDir &target,
                          const std::function<bool(const QString &, const QString &)> &taken = nullptr);

    /** The definition of a .gdtf, or nullptr (see $error). Wheel pictures are
     *  written to $wheelsDir when given. The caller owns it */
    static QLCFixtureDef *read(const QByteArray &gdtf, QString *error, QStringList *warnings,
                               const QDir *wheelsDir = nullptr);

    /** CIE 1931 xyY ("0.3127,0.3290,100") to an sRGB colour at full brightness */
    static QColor cieToColor(const QString &xyY);

    /**
     * The data of the entry $name (as QZipReader::fileInfoList() gives it) of
     * $zip. QZipReader::fileData() looks the names up decoded with the local
     * 8-bit code page, while fileInfoList() decodes them as UTF-8 when the
     * archive says so (QZipWriter, the Open Fixture Library and GDTF-Share
     * zips): on Windows an entry with an accent in its name came back empty.
     */
    static QByteArray zipFileData(const QZipReader &zip, const QString &name);
};

#endif

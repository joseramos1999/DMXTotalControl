/*
  DMX Total Control
  patchtable.h

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

#ifndef PATCHTABLE_H
#define PATCHTABLE_H

#include <QStringList>
#include <QVector3D>
#include <QList>
#include <functional>

class QLCFixtureDefCache;

/**
 * The fixture table a visualizer exports (CSV): its rows read into what the
 * patch needs, and the fixtures of the library that match their types.
 * Everything here is static and knows nothing of a show: PatchManager plans
 * and makes the import with it (patchimport.cpp), the tests call it directly.
 * Other formats of the same table (MVR) can come in through read().
 */
class PatchTable
{
public:
    /** A fixture of the table */
    struct Row
    {
        /** The line of the file it starts on (1: the header) */
        int line = 0;
        /** Manufacturer and model in one string ("Martin MAC 2000 Profile II") */
        QString fixture;
        QString mode;
        /** Of its mode; 0: the table doesn't say */
        int channels = 0;
        /** 1-based; 0: none (or not a number) */
        int universe = 0;
        int address = 0;
        /** What the table writes in them, for the problems */
        QString universeText;
        QString addressText;
        /** The label of the fixture ("1 FILA BEAM1"): its name */
        QString unit;
        /** Its fixture number; 0: none */
        int number = 0;
        QString layer;
        /** The names of the "Groups" column */
        QStringList groups;
        /** Where it is (mm: x across, y up, z towards the audience) */
        bool hasPosition = false;
        QVector3D position;
        /** How it is turned (degrees about x, y and z; an empty one: 0) */
        bool hasRotation = false;
        QVector3D rotation;
        /** Yes or No in the table; not given: the columns aren't there or are empty */
        bool invertPanGiven = false;
        bool invertPan = false;
        bool invertTilt = false;
        bool invertTiltGiven = false;
        /** What the table writes there when it is neither yes nor no */
        QString invertPanText;
        QString invertTiltText;
        /** Stable across exports: the key of a re-import */
        QString identifier;
        /** A second DMX break (not imported): universe and address, 1-based */
        bool secondBreak = false;
        int secondUniverse = 0;
        int secondAddress = 0;
    };

    enum Error
    {
        NoError,
        Empty,          ///< nothing in it
        MissingColumns  ///< not the table: the required columns aren't there
    };
    struct Table
    {
        QList<Row> rows;
        Error error = NoError;
        /** The required columns that are missing (MissingColumns), by
         *  their English names */
        QStringList missingColumns;
        /** The layers, in the order they come */
        QStringList layers;
        /** Whether a row has names in the "Groups" column */
        bool hasGroups = false;
    };

    /** The table in $data. $fileName tells the format (only CSV for now) */
    static Table read(const QByteArray &data, const QString &fileName = QString());
    static Table readCsv(const QByteArray &data);

    /*********************************************************************
     * CSV
     *********************************************************************/
    /** UTF-8 with or without BOM (UTF-16 with BOM; not UTF-8 at all: Latin-1) */
    static QString decode(const QByteArray &data);
    /** ',' or ';' (or a tab): the one the header line has most of, outside quotes */
    static QChar separator(const QString &text);
    /** RFC 4180: quotes, doubled quotes, separators and line breaks inside
     *  quotes. $lines: the line each record starts on (1-based);
     *  $maxRecords: only the first ones (0: all) */
    static QList<QStringList> parseCsv(const QString &text, QChar separator, QList<int> *lines = nullptr,
                                       int maxRecords = 0);

    /*********************************************************************
     * Values with units, in any locale
     *********************************************************************/
    /** "-24,38m", "1.5 m", "250mm", "12'", "3' 4\"", "40 in": $mm in
     *  millimetres (no unit: metres). @return false when it isn't one */
    static bool parseLength(const QString &text, double *mm);
    /** "-90°", "108,5°", "7,8kg": the number, the unit left out */
    static bool parseNumber(const QString &text, double *value);
    /** 1: yes (in several languages), 0: no, -1: empty or neither */
    static int parseYesNo(const QString &text);
    /** A fixture turned by $rotation (degrees about x, y and z) is upside
     *  down: a moving head at 0° hangs, so this one stands on its base.
     *  Its top points below the horizon, whatever the order of the turns
     *  (with no turn about y between them) */
    static bool upsideDown(const QVector3D &rotation);

    /*********************************************************************
     * The fixtures of the library that match
     *********************************************************************/
    /** Letters and digits only, lowercase ("MAC-2000 Profile" → "mac2000profile") */
    static QString normalised(const QString &text);
    /** Its words, letters and digits apart ("MH1" → "mh", "1"; "channels" → "ch") */
    static QStringList tokens(const QString &text);
    /** How alike two names are, by their words (0..1) */
    static qreal similarity(const QString &a, const QString &b);
    /** Whether the library's model $name may be the table's $model by
     *  similar words: not when $name has a number $model hasn't ("MH1"
     *  for "MH 5", "300E" for "600E"), nor when one of them says a kind of
     *  fixture (wash, spot, profile...) the other doesn't */
    static bool similarModelAllowed(const QString &name, const QString &model);
    /** Remembered choices and the rows of a type: fixture, mode and channels */
    static QString typeKey(const QString &fixture, const QString &mode, int channels);

    struct ModeInfo
    {
        QString name;
        int channels = 0;
    };
    /** How a fixture (its model and mode) was found */
    enum How
    {
        NotFound,
        Exact,       ///< the model and the mode by their name
        ByChannels,  ///< the mode by its channel count
        Similar      ///< the model or the mode by similar words
    };
    /** The mode of $modes for the table's $name and $channels (0: not
     *  given): its name, else the one with the channel count (several: the
     *  most alike), else the name exact with another count, else (the
     *  count not given) the most alike. -1: none */
    static int matchMode(const QList<ModeInfo> &modes, const QString &name, int channels, How *how = nullptr);

    /** What the library has: its models, and their modes when asked */
    struct Library
    {
        struct Model
        {
            QString manufacturer;
            QString model;
        };
        QList<Model> models;
        std::function<QList<ModeInfo>(const QString &manufacturer, const QString &model)> modes;
    };
    /** The definitions of $cache (the modes are read when asked) */
    static Library library(QLCFixtureDefCache *cache);
    /** A converted GDTF fixture that has a QLC+ twin ("X (GDTF)") */
    static bool isGdtfTwin(const QString &model);

    struct Match
    {
        How how = NotFound;
        QString manufacturer;
        QString model;
        QString mode;
        /** Of the mode found */
        int channels = 0;
        /** Not found: the model found, with no mode that fits; or only the
         *  manufacturer the string starts with (empty: not even that) */
        QString foundManufacturer;
        QString foundModel;
    };
    /** The fixture of $library for the table's $fixture string, $mode and
     *  $channels: the longest manufacturer the string starts with (also
     *  one whose name goes on: "Martin Professional" for "Martin X"), then
     *  the model by its name, normalised name or words; the mode as
     *  matchMode(). A QLC+ fixture wins over its "(GDTF)" twin when both
     *  match the same, the manufacturer as the table writes it over a longer one */
    static Match match(const Library &library, const QString &fixture, const QString &mode, int channels);
};

#endif

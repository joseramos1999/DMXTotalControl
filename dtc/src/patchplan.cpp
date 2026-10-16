/*
  DMX Total Control
  patchplan.cpp

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

/* PatchManager: LED matrices, the tools of the 2D plan and what the plan
 * shows live */

#include <QFileInfo>
#include <QVector3D>
#include <QColor>
#include <QUrl>
#include <qmath.h>

#include "patchmanager.h"
#include "monitorproperties.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "inputoutputmap.h"
#include "qlccapability.h"
#include "fixturegroup.h"
#include "qlcchannel.h"
#include "universe.h"
#include "fixture.h"
#include "doc.h"

/* Distance between the pixels of a matrix in the plan, in millimeters */
#define MATRIX_PITCH 100.0
/* Default distance between fixtures placed by the plan tools */
#define ARRANGE_SPACING 600.0

/*********************************************************************
 * LED matrices and strips
 *********************************************************************/

QString PatchManager::addMatrix(const QString &name, int columns, int rows, const QString &components,
                                int universe, int address, const QString &wiring, const QString &start)
{
    static const QStringList componentNames = { "RGB", "BGR", "BRG", "GBR", "GRB", "RGBW", "RBG" };
    int componentIndex = componentNames.indexOf(components.toUpper());
    if (componentIndex < 0)
        return tr("Unknown colour order %1").arg(components);
    Fixture::Components comp = Fixture::Components(componentIndex);
    int perPixel = comp == Fixture::RGBW ? 4 : 3;

    columns = qMax(1, columns);
    rows = qMax(1, rows);
    bool vertical = wiring.startsWith("columns") || wiring == "snakeColumns";
    bool snake = wiring.startsWith("snake");
    bool fromRight = start.endsWith("Right");
    bool fromBottom = start.startsWith("bottom");

    // one fixture per line of pixels, in wiring order
    int lines = vertical ? columns : rows;
    int length = vertical ? rows : columns;
    if (length * perPixel > UNIVERSE_SIZE)
        return tr("A line of %1 pixels needs %2 channels: more than a universe").arg(length).arg(length * perPixel);

    InputOutputMap *ioMap = m_doc->inputOutputMap();
    QString baseName = name.trimmed().isEmpty() ? (rows == 1 ? tr("LED strip") : tr("LED matrix")) : name.trimmed();

    // where each line goes, checked before patching any: all of them or none
    const int lineChannels = length * perPixel;
    QList<QPair<quint32, quint32>> places;
    {
        quint32 uniIdx = quint32(qMax(0, universe));
        quint32 addr = quint32(qMax(1, address) - 1);
        for (int line = 0; line < lines; line++)
        {
            if (addr + quint32(lineChannels) > UNIVERSE_SIZE)
            {
                // the next line starts on the next universe
                uniIdx++;
                addr = 0;
            }
            if (isRangeFree(int(uniIdx), int(addr), lineChannels, Fixture::invalidId()) == false)
                return tr("Addresses %1-%2 of universe %3 are used by another fixture")
                    .arg(addr + 1).arg(addr + quint32(lineChannels)).arg(uniIdx + 1);
            places.append(qMakePair(uniIdx, addr));
            addr += quint32(lineChannels);
        }
    }

    FixtureGroup *group = new FixtureGroup(m_doc);
    group->setName(baseName);
    group->setSize(QSize(columns, rows));
    if (m_doc->addFixtureGroup(group) == false)
    {
        delete group;
        return tr("Unable to create the group");
    }

    // the plan: the matrix in the middle of the stage, one pixel every
    // MATRIX_PITCH; below the matrices already there, if there is room
    MonitorProperties *monitor = m_doc->monitorProperties();
    QSizeF stage = stageSize();
    qreal x0 = qMax(MATRIX_PITCH / 2, stage.width() / 2 - (columns - 1) * MATRIX_PITCH / 2);
    qreal y0 = qMax(MATRIX_PITCH / 2, stage.height() / 2 - (rows - 1) * MATRIX_PITCH / 2);
    {
        QList<QPointF> spots;
        for (Fixture *other : m_doc->fixtures())
            if (monitor->containsFixture(other->id()))
                spots.append(position(other->id()));
        auto isFree = [&](qreal top) {
            QRectF area(x0 - MATRIX_PITCH / 2, top - MATRIX_PITCH / 2, columns * MATRIX_PITCH, rows * MATRIX_PITCH);
            for (const QPointF &p : std::as_const(spots))
                if (area.contains(p))
                    return false;
            return true;
        };
        for (qreal top = y0; top + (rows - 1) * MATRIX_PITCH <= stage.height(); top += (rows + 1) * MATRIX_PITCH)
        {
            if (isFree(top))
            {
                y0 = top;
                break;
            }
        }
    }

    QLCFixtureDef *def = nullptr;
    QLCFixtureMode *mode = nullptr;
    QList<quint32> added;
    for (int line = 0; line < lines; line++)
    {
        Fixture *fxi = new Fixture(m_doc);
        if (def == nullptr)
        {
            def = fxi->genericRGBPanelDef(length, comp, false);
            mode = fxi->genericRGBPanelMode(def, comp, false, quint32(length * MATRIX_PITCH), quint32(MATRIX_PITCH));
        }
        fxi->setFixtureDefinition(def, mode);

        const quint32 uniIdx = places.at(line).first;
        while (uniIdx >= ioMap->universesCount())
        {
            ioMap->addUniverse();
            ioMap->startUniverses();
        }
        fxi->setUniverse(uniIdx);
        fxi->setAddress(places.at(line).second);
        fxi->setName(lines > 1 ? tr("%1 - %2").arg(baseName).arg(line + 1) : baseName);
        if (m_doc->addFixture(fxi) == false)
        {
            // (checked above: Doc refuses it all the same) the lines patched
            // and the group go too
            delete fxi;
            for (quint32 id : std::as_const(added))
                m_doc->deleteFixture(id);
            m_doc->deleteFixtureGroup(group->id());
            m_doc->refreshChannelCapabilities();
            notifyFixtures();
            emit groupsChanged();
            return tr("Unable to patch line %1").arg(line + 1);
        }
        added.append(fxi->id());

        // where its pixels are: the line, and the direction of its pixels
        int across = vertical ? (fromRight ? columns - 1 - line : line) : (fromBottom ? rows - 1 - line : line);
        bool backwards = vertical ? fromBottom : fromRight;
        if (snake && line % 2 == 1)
            backwards = !backwards;
        for (int h = 0; h < fxi->heads(); h++)
        {
            int along = backwards ? length - 1 - h : h;
            QLCPoint point = vertical ? QLCPoint(across, along) : QLCPoint(along, across);
            group->assignHead(point, GroupHead(fxi->id(), h));
        }

        // in the plan, a bar of pixels turned to the way they run
        qreal cx = vertical ? x0 + across * MATRIX_PITCH : x0 + (columns - 1) * MATRIX_PITCH / 2;
        qreal cy = vertical ? y0 + (rows - 1) * MATRIX_PITCH / 2 : y0 + across * MATRIX_PITCH;
        qreal rotation = vertical ? (backwards ? 270 : 90) : (backwards ? 180 : 0);
        monitor->setFixturePosition(fxi->id(), 0, 0, QVector3D(float(cx), float(cy), 0));
        monitor->setFixtureRotation(fxi->id(), 0, 0, QVector3D(0, 0, float(rotation)));
    }

    m_doc->setModified();
    notifyFixtures();
    emit groupsChanged();
    return QString();
}

/*********************************************************************
 * 2D plan tools
 *********************************************************************/

QPointF PatchManager::position(quint32 id) const
{
    MonitorProperties *monitor = m_doc->monitorProperties();
    if (monitor->containsFixture(id) == false)
        return QPointF();
    QVector3D p = monitor->fixturePosition(id, 0, 0);
    return QPointF(p.x(), p.y());
}

void PatchManager::setPositions(const QHash<quint32, QPointF> &positions)
{
    MonitorProperties *monitor = m_doc->monitorProperties();
    QSizeF stage = stageSize();
    for (auto it = positions.cbegin(); it != positions.cend(); ++it)
    {
        qreal x = qBound(0.0, it.value().x(), stage.width());
        qreal y = qBound(0.0, it.value().y(), stage.height());
        monitor->setFixturePosition(it.key(), 0, 0, QVector3D(float(x), float(y), 0));
    }
    if (positions.isEmpty() == false)
    {
        m_doc->setModified();
        notifyFixtures();
    }
}

static QList<quint32> existing(Doc *doc, const QVariantList &ids)
{
    QList<quint32> list;
    for (const QVariant &v : ids)
        if (doc->fixture(v.toUInt()) != nullptr && list.contains(v.toUInt()) == false)
            list.append(v.toUInt());
    return list;
}

void PatchManager::arrange(const QVariantList &ids, const QString &shape, const QVariantMap &options)
{
    QList<quint32> fixtures = existing(m_doc, ids);
    int n = fixtures.count();
    if (n == 0)
        return;

    // around the centre of the fixtures, where they are now
    QPointF centre;
    for (quint32 id : fixtures)
        centre += position(id);
    centre /= n;

    qreal spacing = options.value("spacing", ARRANGE_SPACING).toDouble();
    QHash<quint32, QPointF> positions;
    if (shape == "circle")
    {
        qreal radius = options.value("radius", qMax(spacing * n / (2 * M_PI), spacing)).toDouble();
        for (int i = 0; i < n; i++)
        {
            // the first one at the top, clockwise
            qreal angle = -M_PI / 2 + 2 * M_PI * i / n;
            positions.insert(fixtures.at(i), centre + QPointF(radius * qCos(angle), radius * qSin(angle)));
        }
    }
    else if (shape == "grid")
    {
        int columns = qMax(1, options.value("columns", qCeil(qSqrt(n))).toInt());
        int rows = (n + columns - 1) / columns;
        QString order = options.value("order", "rows").toString();
        bool byColumns = order.endsWith("olumns");
        bool snake = order.startsWith("snake");
        int lineLength = byColumns ? rows : columns;
        for (int i = 0; i < n; i++)
        {
            int line = i / lineLength;
            int along = i % lineLength;
            if (snake && line % 2 == 1)
                along = lineLength - 1 - along;
            int col = byColumns ? line : along;
            int row = byColumns ? along : line;
            positions.insert(fixtures.at(i), centre + QPointF((col - (columns - 1) / 2.0) * spacing,
                                                              (row - (rows - 1) / 2.0) * spacing));
        }
    }
    else
    {
        // a line, left to right or front to back
        bool vertical = options.value("vertical").toBool();
        for (int i = 0; i < n; i++)
        {
            qreal offset = (i - (n - 1) / 2.0) * spacing;
            positions.insert(fixtures.at(i), centre + (vertical ? QPointF(0, offset) : QPointF(offset, 0)));
        }
    }
    setPositions(positions);
}

void PatchManager::align(const QVariantList &ids, const QString &how)
{
    QList<quint32> fixtures = existing(m_doc, ids);
    if (fixtures.count() < 2)
        return;

    bool horizontal = how == "left" || how == "right" || how == "centerX";
    qreal target = how == "left" || how == "top" ? 1e9 : (how == "right" || how == "bottom" ? -1e9 : 0);
    for (quint32 id : fixtures)
    {
        qreal v = horizontal ? position(id).x() : position(id).y();
        if (how == "left" || how == "top")
            target = qMin(target, v);
        else if (how == "right" || how == "bottom")
            target = qMax(target, v);
        else
            target += v / fixtures.count();
    }

    QHash<quint32, QPointF> positions;
    for (quint32 id : fixtures)
    {
        QPointF p = position(id);
        positions.insert(id, horizontal ? QPointF(target, p.y()) : QPointF(p.x(), target));
    }
    setPositions(positions);
}

void PatchManager::distribute(const QVariantList &ids, const QString &axis)
{
    QList<quint32> fixtures = existing(m_doc, ids);
    if (fixtures.count() < 3)
        return;

    bool horizontal = axis != "vertical";
    std::sort(fixtures.begin(), fixtures.end(), [&](quint32 a, quint32 b) {
        return horizontal ? position(a).x() < position(b).x() : position(a).y() < position(b).y();
    });
    qreal first = horizontal ? position(fixtures.first()).x() : position(fixtures.first()).y();
    qreal last = horizontal ? position(fixtures.last()).x() : position(fixtures.last()).y();
    qreal step = (last - first) / (fixtures.count() - 1);

    QHash<quint32, QPointF> positions;
    for (int i = 0; i < fixtures.count(); i++)
    {
        QPointF p = position(fixtures.at(i));
        qreal v = first + step * i;
        positions.insert(fixtures.at(i), horizontal ? QPointF(v, p.y()) : QPointF(p.x(), v));
    }
    setPositions(positions);
}

void PatchManager::rotate(const QVariantList &ids, qreal degrees)
{
    QList<quint32> fixtures = existing(m_doc, ids);
    if (fixtures.isEmpty())
        return;

    QPointF centre;
    for (quint32 id : fixtures)
        centre += position(id);
    centre /= fixtures.count();

    qreal rad = qDegreesToRadians(degrees);
    MonitorProperties *monitor = m_doc->monitorProperties();
    QHash<quint32, QPointF> positions;
    for (quint32 id : fixtures)
    {
        QPointF d = position(id) - centre;
        positions.insert(id, centre + QPointF(d.x() * qCos(rad) - d.y() * qSin(rad),
                                              d.x() * qSin(rad) + d.y() * qCos(rad)));
        // each one turns on itself too
        QVector3D rot = monitor->fixtureRotation(id, 0, 0);
        float z = float(std::fmod(rot.z() + degrees + 360.0, 360.0));
        monitor->setFixtureRotation(id, 0, 0, QVector3D(rot.x(), rot.y(), z));
    }
    setPositions(positions);
}

void PatchManager::moveFixtures(const QVariantList &ids, qreal dx, qreal dy, qreal snap)
{
    QHash<quint32, QPointF> positions;
    for (quint32 id : existing(m_doc, ids))
    {
        QPointF p = position(id) + QPointF(dx, dy);
        if (snap > 0)
            p = QPointF(qRound(p.x() / snap) * snap, qRound(p.y() / snap) * snap);
        positions.insert(id, p);
    }
    setPositions(positions);
}

void PatchManager::setFixtureShape(const QVariantList &ids, const QString &shape)
{
    for (quint32 id : existing(m_doc, ids))
    {
        if (shape == "square" || shape == "rectangle")
            m_shapes.insert(id, shape);
        else
            m_shapes.remove(id);
    }
    m_doc->setModified();
    notifyFixtures();
}

void PatchManager::clearPlan()
{
    m_shapes.clear();
}

/*********************************************************************
 * What the plan shows live
 *********************************************************************/

QVariantList PatchManager::liveStates() const
{
    QVariantList list;
    QList<Universe *> universes = m_doc->inputOutputMap()->claimUniverses();

    for (Fixture *fxi : m_doc->fixtures())
    {
        if (fxi->universe() >= quint32(universes.count()))
            continue;
        Universe *universe = universes.at(int(fxi->universe()));
        // what goes out (dimmer limits and grand master included)
        auto value = [&](quint32 channel) -> int {
            return channel == QLCChannel::invalid() ? -1 : universe->postGMValue(int(fxi->address() + channel));
        };
        auto level = [&](quint32 channel) { return qMax(0, value(channel)) / 255.0; };

        // the dimmer of the fixture and of each head
        quint32 master = fxi->masterIntensityChannel();
        qreal dimmer = master != QLCChannel::invalid() ? level(master) : 1.0;

        // channels by group: the first coarse one (colour wheel, gobo, shutter)
        quint32 wheel = QLCChannel::invalid(), gobo = QLCChannel::invalid(), shutter = QLCChannel::invalid();
        for (quint32 c = 0; c < fxi->channels(); c++)
        {
            const QLCChannel *ch = fxi->channel(c);
            if (ch == nullptr || ch->controlByte() != QLCChannel::MSB)
                continue;
            if (ch->group() == QLCChannel::Colour && wheel == QLCChannel::invalid())
                wheel = c;
            else if (ch->group() == QLCChannel::Gobo && gobo == QLCChannel::invalid())
                gobo = c;
            else if (ch->group() == QLCChannel::Shutter && shutter == QLCChannel::invalid())
                shutter = c;
        }

        // a colour wheel slot with its colour
        QColor wheelColour;
        if (wheel != QLCChannel::invalid())
        {
            QLCCapability *cap = fxi->channel(wheel)->searchCapability(uchar(value(wheel)));
            if (cap != nullptr && (cap->presetType() == QLCCapability::SingleColor ||
                                   cap->presetType() == QLCCapability::DoubleColor))
                wheelColour = cap->resource(0).value<QColor>();
        }

        QVariantList colors;
        qreal brightest = 0;
        int heads = qMax(1, fxi->heads());
        for (int h = 0; h < heads; h++)
        {
            qreal headDimmer = dimmer;
            quint32 intensity = fxi->channelNumber(QLCChannel::Intensity, QLCChannel::MSB, h);
            if (intensity != QLCChannel::invalid() && intensity != master)
                headDimmer *= level(intensity);
            else if (intensity == QLCChannel::invalid() && master == QLCChannel::invalid())
                headDimmer = 1.0;

            qreal r = 1, g = 1, b = 1;
            QVector<quint32> rgb = fxi->rgbChannels(h);
            QVector<quint32> cmy = fxi->cmyChannels(h);
            if (rgb.count() == 3)
            {
                r = level(rgb.at(0)); g = level(rgb.at(1)); b = level(rgb.at(2));
                // a white led adds to the three
                for (quint32 c : fxi->head(h).channels())
                {
                    const QLCChannel *ch = fxi->channel(c);
                    if (ch != nullptr && ch->group() == QLCChannel::Intensity && ch->colour() == QLCChannel::White)
                    {
                        qreal w = level(c);
                        r = qMin(1.0, r + w); g = qMin(1.0, g + w); b = qMin(1.0, b + w);
                    }
                }
                // RGB without a dimmer: the colour is the light
                if (master == QLCChannel::invalid() && intensity == QLCChannel::invalid())
                    headDimmer = 1.0;
            }
            else if (cmy.count() == 3)
            {
                r = 1 - level(cmy.at(0)); g = 1 - level(cmy.at(1)); b = 1 - level(cmy.at(2));
            }
            else if (wheelColour.isValid())
            {
                r = wheelColour.redF(); g = wheelColour.greenF(); b = wheelColour.blueF();
            }
            else if (master == QLCChannel::invalid() && intensity == QLCChannel::invalid())
            {
                // no dimmer at all (smoke, scanners without one): nothing to show
                headDimmer = 0;
            }

            r *= headDimmer; g *= headDimmer; b *= headDimmer;
            brightest = qMax(brightest, qMax(r, qMax(g, b)));
            colors.append(QColor::fromRgbF(float(r), float(g), float(b)).name());
        }

        bool strobe = false;
        if (shutter != QLCChannel::invalid())
        {
            QLCCapability *cap = fxi->channel(shutter)->searchCapability(uchar(value(shutter)));
            strobe = cap != nullptr && ((cap->preset() >= QLCCapability::StrobeSlowToFast &&
                                         cap->preset() <= QLCCapability::PulseFreqRange) ||
                                        cap->name().contains("strobe", Qt::CaseInsensitive));
        }

        QString goboName, goboImage;
        if (gobo != QLCChannel::invalid())
        {
            QLCCapability *cap = fxi->channel(gobo)->searchCapability(uchar(value(gobo)));
            bool open = cap == nullptr || cap->name().contains("open", Qt::CaseInsensitive) ||
                        cap->name().contains("abiert", Qt::CaseInsensitive) || cap->preset() == QLCCapability::GoboMacro;
            if (open == false)
            {
                goboName = cap->name();
                QString path = cap->resource(0).toString();
                if (cap->presetType() == QLCCapability::Picture && QFileInfo::exists(path))
                    goboImage = QUrl::fromLocalFile(path).toString();
            }
        }

        QVariantMap state;
        state.insert("id", fxi->id());
        state.insert("dimmer", brightest);
        state.insert("colors", colors);
        state.insert("strobe", strobe && brightest > 0.02);
        state.insert("gobo", goboName);
        state.insert("goboImage", goboImage);
        list.append(state);
    }

    m_doc->inputOutputMap()->releaseUniverses(false);
    return list;
}

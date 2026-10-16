/*
  DMX Total Control
  gdtfimporter.cpp

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

#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QBuffer>
#include <QColor>
#include <QFile>
#include <QHash>
#include <QSet>

#include <cmath>
#include <memory>
#include <vector>

// QZipReader moved from QtGui to QtCore in recent Qt versions
#if __has_include(<QtCore/private/qzipreader_p.h>)
#include <QtCore/private/qzipreader_p.h>
#else
#include <QtGui/private/qzipreader_p.h>
#endif

#include "gdtfimporter.h"

#include "qlcfixturehead.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcphysical.h"
#include "qlcchannel.h"

namespace
{

/*********************************************************************
 * A small tree of the XML: GDTF links nodes by name all over the file
 *********************************************************************/

struct Node
{
    QString tag;
    QHash<QString, QString> attrs;
    std::vector<std::unique_ptr<Node>> children;
    Node *parent = nullptr;

    QString attr(const char *name, const QString &fallback = QString()) const
    {
        auto it = attrs.constFind(QLatin1String(name));
        return it != attrs.constEnd() ? it.value() : fallback;
    }
    const Node *child(const char *name) const
    {
        for (const auto &c : children)
            if (c->tag == QLatin1String(name))
                return c.get();
        return nullptr;
    }
    QList<const Node *> all(const char *name) const
    {
        QList<const Node *> list;
        for (const auto &c : children)
            if (c->tag == QLatin1String(name))
                list.append(c.get());
        return list;
    }
};

std::unique_ptr<Node> parseXml(const QByteArray &data, QString *error)
{
    QXmlStreamReader xml(data);
    auto root = std::make_unique<Node>();
    Node *current = root.get();
    while (xml.atEnd() == false)
    {
        xml.readNext();
        if (xml.isStartElement())
        {
            auto node = std::make_unique<Node>();
            node->tag = xml.name().toString();
            for (const QXmlStreamAttribute &a : xml.attributes())
                node->attrs.insert(a.name().toString(), a.value().toString());
            node->parent = current;
            Node *raw = node.get();
            current->children.push_back(std::move(node));
            current = raw;
        }
        else if (xml.isEndElement())
        {
            if (current->parent != nullptr)
                current = current->parent;
        }
    }
    if (xml.hasError())
    {
        *error = QObject::tr("description.xml can't be read: %1").arg(xml.errorString());
        return nullptr;
    }
    return root;
}

/*********************************************************************
 * DMX values: "128/1", "32768/2", "255/1s" (byte shifting), "None"
 *********************************************************************/

struct DmxValue
{
    quint64 value = 0;
    int bytes = 1;
    bool shift = false;
    bool valid = false;
};

DmxValue parseDmx(const QString &text)
{
    DmxValue v;
    QString t = text.trimmed();
    if (t.isEmpty() || t.compare("None", Qt::CaseInsensitive) == 0)
        return v;
    int slash = int(t.indexOf('/'));
    QString number = slash < 0 ? t : t.left(slash);
    if (slash >= 0)
    {
        QString rest = t.mid(slash + 1);
        v.shift = rest.endsWith('s');
        if (v.shift)
            rest.chop(1);
        v.bytes = qBound(1, rest.toInt(), 4);
    }
    bool ok = false;
    v.value = number.toULongLong(&ok);
    v.valid = ok;
    return v;
}

/** A value in the resolution of the channel: mirroring (255/1 = 65535/2) or shifting (255/1s = 65280/2) */
quint64 inResolution(const DmxValue &v, int bytes)
{
    if (v.valid == false)
        return 0;
    if (v.bytes == bytes)
        return v.value;
    if (v.bytes > bytes)
        return v.value >> (8 * (v.bytes - bytes));
    if (v.shift)
        return v.value << (8 * (bytes - v.bytes));
    const quint64 from = (quint64(1) << (8 * v.bytes)) - 1;
    const quint64 to = (quint64(1) << (8 * bytes)) - 1;
    return from == 0 ? 0 : (v.value * to + from / 2) / from;
}

int coarse(quint64 value, int bytes)
{
    return int(qBound<quint64>(0, value >> (8 * (bytes - 1)), 255));
}

/*********************************************************************
 * Attributes: what QLC+ knows of them
 *********************************************************************/

struct Kind
{
    QLCChannel::Group group = QLCChannel::Effect;
    QLCChannel::PrimaryColour colour = QLCChannel::NoColour;
    /** Presets for the coarse and fine channels when the channel is one plain function */
    QLCChannel::Preset preset = QLCChannel::Custom;
    QLCChannel::Preset finePreset = QLCChannel::Custom;
    QString name;
};

/** "Gobo1WheelSpin" -> "Gobo 1 wheel spin", "ColorAdd_R" -> "Color add R" */
QString humanize(const QString &attribute)
{
    QString out;
    for (int i = 0; i < attribute.size(); i++)
    {
        const QChar c = attribute.at(i);
        if (c == '_' || c == '.')
        {
            out += ' ';
            continue;
        }
        if (i > 0)
        {
            const QChar p = attribute.at(i - 1);
            if ((c.isUpper() && p.isLower()) || (c.isDigit() && p.isLetter()) || (c.isLetter() && p.isDigit()))
                out += ' ';
        }
        out += c;
    }
    QStringList words = out.split(' ', Qt::SkipEmptyParts);
    for (int i = 1; i < words.count(); i++)
        if (words.at(i).length() > 1 && words.at(i).at(1).isLower())
            words[i] = words.at(i).toLower();
    return words.join(' ');
}

Kind kindOf(const QString &a)
{
    Kind k;
    k.name = humanize(a);

    static const QRegularExpression numbered(QStringLiteral("^([A-Za-z]+?)(\\d+)(.*)$"));
    const QRegularExpressionMatch m = numbered.match(a);
    const QString base = m.hasMatch() ? m.captured(1) : a;
    const QString number = m.hasMatch() ? m.captured(2) : QString();
    const QString rest = m.hasMatch() ? m.captured(3) : QString();
    // "Shutter", "Shutter 2": the first one without its number
    const QString suffix = number.isEmpty() || number == QLatin1String("1") ? QString() : QStringLiteral(" ") + number;

    auto set = [&k](QLCChannel::Group group, const QString &name,
                    QLCChannel::Preset preset = QLCChannel::Custom, QLCChannel::Preset fine = QLCChannel::Custom,
                    QLCChannel::PrimaryColour colour = QLCChannel::NoColour) {
        k.group = group;
        k.name = name;
        k.preset = preset;
        k.finePreset = fine;
        k.colour = colour;
    };

    // intensity and position
    if (a == QLatin1String("Dimmer"))
        set(QLCChannel::Intensity, "Dimmer", QLCChannel::IntensityMasterDimmer, QLCChannel::IntensityMasterDimmerFine);
    else if (a == QLatin1String("Pan"))
        set(QLCChannel::Pan, "Pan", QLCChannel::PositionPan, QLCChannel::PositionPanFine);
    else if (a == QLatin1String("Tilt"))
        set(QLCChannel::Tilt, "Tilt", QLCChannel::PositionTilt, QLCChannel::PositionTiltFine);
    else if (a == QLatin1String("PanRotate"))
        set(QLCChannel::Pan, "Pan rotation");
    else if (a == QLatin1String("TiltRotate"))
        set(QLCChannel::Tilt, "Tilt rotation");
    else if (a == QLatin1String("PositionMSpeed") || a == QLatin1String("PanTiltMSpeed"))
        set(QLCChannel::Speed, "Pan/Tilt speed");
    else if (a.startsWith(QLatin1String("PositionEffect")))
        set(QLCChannel::Effect, k.name);
    // colour: the emitters, CMY, hue and saturation
    else if (a == QLatin1String("ColorAdd_R") || a == QLatin1String("ColorRGB_Red"))
        set(QLCChannel::Intensity, "Red", QLCChannel::IntensityRed, QLCChannel::IntensityRedFine, QLCChannel::Red);
    else if (a == QLatin1String("ColorAdd_G") || a == QLatin1String("ColorRGB_Green"))
        set(QLCChannel::Intensity, "Green", QLCChannel::IntensityGreen, QLCChannel::IntensityGreenFine, QLCChannel::Green);
    else if (a == QLatin1String("ColorAdd_B") || a == QLatin1String("ColorRGB_Blue"))
        set(QLCChannel::Intensity, "Blue", QLCChannel::IntensityBlue, QLCChannel::IntensityBlueFine, QLCChannel::Blue);
    else if (a == QLatin1String("ColorAdd_W") || a == QLatin1String("ColorRGB_White"))
        set(QLCChannel::Intensity, "White", QLCChannel::IntensityWhite, QLCChannel::IntensityWhiteFine, QLCChannel::White);
    else if (a == QLatin1String("ColorAdd_WW"))
        set(QLCChannel::Intensity, "Warm white", QLCChannel::Custom, QLCChannel::Custom, QLCChannel::White);
    else if (a == QLatin1String("ColorAdd_CW"))
        set(QLCChannel::Intensity, "Cold white", QLCChannel::Custom, QLCChannel::Custom, QLCChannel::White);
    else if (a == QLatin1String("ColorAdd_A") || a == QLatin1String("ColorRGB_Amber"))
        set(QLCChannel::Intensity, "Amber", QLCChannel::IntensityAmber, QLCChannel::IntensityAmberFine, QLCChannel::Amber);
    else if (a == QLatin1String("ColorAdd_RY"))
        set(QLCChannel::Intensity, "Red-yellow", QLCChannel::Custom, QLCChannel::Custom, QLCChannel::Amber);
    else if (a == QLatin1String("ColorAdd_UV"))
        set(QLCChannel::Intensity, "UV", QLCChannel::IntensityUV, QLCChannel::IntensityUVFine, QLCChannel::UV);
    else if (a == QLatin1String("ColorAdd_GY"))
        set(QLCChannel::Intensity, "Lime", QLCChannel::IntensityLime, QLCChannel::IntensityLimeFine, QLCChannel::Lime);
    else if (a == QLatin1String("ColorSub_C"))
        set(QLCChannel::Intensity, "Cyan", QLCChannel::IntensityCyan, QLCChannel::IntensityCyanFine, QLCChannel::Cyan);
    else if (a == QLatin1String("ColorSub_M"))
        set(QLCChannel::Intensity, "Magenta", QLCChannel::IntensityMagenta, QLCChannel::IntensityMagentaFine, QLCChannel::Magenta);
    else if (a == QLatin1String("ColorSub_Y"))
        set(QLCChannel::Intensity, "Yellow", QLCChannel::IntensityYellow, QLCChannel::IntensityYellowFine, QLCChannel::Yellow);
    // additive cyan, indigo... are not CMY: a colour channel of their own
    else if (a.startsWith(QLatin1String("ColorAdd_")) || a.startsWith(QLatin1String("ColorSub_")) ||
             a.startsWith(QLatin1String("ColorRGB_")) || a.startsWith(QLatin1String("CIE_")))
        set(QLCChannel::Colour, k.name);
    else if (a == QLatin1String("HSB_Hue"))
        set(QLCChannel::Intensity, "Hue", QLCChannel::IntensityHue, QLCChannel::IntensityHueFine);
    else if (a == QLatin1String("HSB_Saturation"))
        set(QLCChannel::Intensity, "Saturation", QLCChannel::IntensitySaturation, QLCChannel::IntensitySaturationFine);
    else if (a == QLatin1String("HSB_Brightness"))
        set(QLCChannel::Intensity, "Brightness", QLCChannel::IntensityLightness, QLCChannel::IntensityLightnessFine);
    else if (a == QLatin1String("CTO"))
        set(QLCChannel::Colour, "CTO", QLCChannel::ColorCTOMixer);
    else if (a == QLatin1String("CTC"))
        set(QLCChannel::Colour, "CTC", QLCChannel::ColorCTCMixer);
    else if (a == QLatin1String("CTB"))
        set(QLCChannel::Colour, "CTB", QLCChannel::ColorCTBMixer);
    else if (a == QLatin1String("Tint"))
        set(QLCChannel::Colour, "Tint");
    else if (a.endsWith(QLatin1String("Mode")) || a.endsWith(QLatin1String("Curve")))
        set(QLCChannel::Maintenance, k.name);
    else if (base == QLatin1String("Color") && rest.isEmpty())
        set(QLCChannel::Colour, "Color wheel " + number, QLCChannel::ColorWheel, QLCChannel::ColorWheelFine);
    else if (base == QLatin1String("Color"))
        set(QLCChannel::Colour, "Color wheel " + number + " rotation");
    else if (base == QLatin1String("ColorMacro"))
        set(QLCChannel::Colour, rest.isEmpty() ? "Color macro" + suffix : k.name);
    // beam
    else if (base == QLatin1String("Gobo") && rest.isEmpty())
        set(QLCChannel::Gobo, "Gobo wheel " + number, QLCChannel::GoboWheel, QLCChannel::GoboWheelFine);
    else if (base == QLatin1String("Gobo") && rest.startsWith(QLatin1String("Pos")))
        set(QLCChannel::Gobo, "Gobo " + number + " rotation");
    else if (base == QLatin1String("Gobo"))
        set(QLCChannel::Gobo, "Gobo wheel " + number + " spin");
    else if (a.startsWith(QLatin1String("Animation")))
        set(QLCChannel::Effect, k.name);
    else if (base == QLatin1String("Shutter"))
        set(QLCChannel::Shutter, rest.isEmpty() ? "Shutter" + suffix : humanize(rest) + suffix);
    else if (a.startsWith(QLatin1String("Strobe")))
        set(QLCChannel::Shutter, k.name);
    else if (a == QLatin1String("Iris"))
        set(QLCChannel::Shutter, "Iris", QLCChannel::ShutterIrisMinToMax, QLCChannel::ShutterIrisFine);
    else if (a.startsWith(QLatin1String("Iris")))
        set(QLCChannel::Shutter, k.name);
    else if (base == QLatin1String("Focus") && rest.isEmpty())
        set(QLCChannel::Beam, "Focus" + suffix, suffix.isEmpty() ? QLCChannel::BeamFocusNearFar : QLCChannel::Custom,
            suffix.isEmpty() ? QLCChannel::BeamFocusFine : QLCChannel::Custom);
    else if (base == QLatin1String("Focus"))
        set(QLCChannel::Beam, "Focus" + suffix + " " + humanize(rest).toLower());
    else if (a == QLatin1String("Zoom"))
        set(QLCChannel::Beam, "Zoom", QLCChannel::BeamZoomSmallBig, QLCChannel::BeamZoomFine);
    else if (a.startsWith(QLatin1String("Zoom")))
        set(QLCChannel::Beam, k.name);
    else if (base == QLatin1String("Frost"))
        set(QLCChannel::Beam, rest.isEmpty() ? "Frost" + suffix : k.name);
    else if (base == QLatin1String("Prism") && rest.isEmpty())
        set(QLCChannel::Prism, "Prism" + suffix);
    else if (base == QLatin1String("Prism"))
        set(QLCChannel::Prism, "Prism" + suffix + " rotation");
    else if (a.startsWith(QLatin1String("Blade")) || a.startsWith(QLatin1String("Shaper")) ||
             a.startsWith(QLatin1String("BeamShaper")))
        set(QLCChannel::Beam, k.name);
    else if (a.contains(QLatin1String("MSpeed")))
        set(QLCChannel::Speed, k.name);
    else if (a.startsWith(QLatin1String("Control")) || a.startsWith(QLatin1String("Function")) ||
             a.startsWith(QLatin1String("Lamp")) || a.startsWith(QLatin1String("Reset")) ||
             a.startsWith(QLatin1String("Fixture")) || a.startsWith(QLatin1String("Display")) ||
             a.startsWith(QLatin1String("DMXInput")) || a == QLatin1String("NoFeature") ||
             a.startsWith(QLatin1String("Dummy")) || a.startsWith(QLatin1String("Blackout")) ||
             a.endsWith(QLatin1String("Reset")))
        set(QLCChannel::Maintenance, k.name);
    else
        set(QLCChannel::Effect, k.name);
    return k;
}

/*********************************************************************
 * The conversion
 *********************************************************************/

struct Slot
{
    QString name;
    QColor color;
    QString media;
};

struct Context
{
    const Node *fixtureType = nullptr;
    QHash<QString, QList<Slot>> wheels;
    /** Every geometry by name */
    QHash<QString, const Node *> geometries;
    QByteArray archive;
    QZipReader *zip = nullptr;
    const QDir *wheelsDir = nullptr;
    QString fileBase;
    QHash<QString, QString> writtenMedia;
    QStringList *warnings = nullptr;
};

void collectGeometries(const Node *node, QHash<QString, const Node *> &map)
{
    for (const auto &c : node->children)
    {
        if (c->attrs.contains(QStringLiteral("Name")) && c->tag != QLatin1String("Break"))
            map.insert(c->attr("Name"), c.get());
        collectGeometries(c.get(), map);
    }
}

/** The top-level geometry that holds $node */
const Node *topLevelOf(const Node *node, const Node *geometries)
{
    const Node *n = node;
    while (n != nullptr && n->parent != geometries)
        n = n->parent;
    return n;
}

void findReferences(const Node *node, QList<const Node *> &refs)
{
    for (const auto &c : node->children)
    {
        if (c->tag == QLatin1String("GeometryReference"))
            refs.append(c.get());
        findReferences(c.get(), refs);
    }
}

const Node *findFirst(const Node *node, const char *tag)
{
    for (const auto &c : node->children)
    {
        if (c->tag == QLatin1String(tag))
            return c.get();
        if (const Node *found = findFirst(c.get(), tag))
            return found;
    }
    return nullptr;
}

/** A wheel picture, written next to the definition: its absolute path */
QString mediaPath(Context &ctx, const QString &media)
{
    if (media.isEmpty() || ctx.zip == nullptr || ctx.wheelsDir == nullptr)
        return QString();
    if (ctx.writtenMedia.contains(media))
        return ctx.writtenMedia.value(media);

    QByteArray data;
    for (const QString &ext : { QStringLiteral(".png"), QStringLiteral(".svg"), QStringLiteral(".jpg"), QString() })
    {
        data = GdtfImporter::zipFileData(*ctx.zip, QStringLiteral("wheels/") + media + ext);
        if (data.isEmpty() == false)
        {
            QDir dir(ctx.wheelsDir->filePath(ctx.fileBase));
            dir.mkpath(".");
            QString name = media;
            name.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_\\-\\.]+")), QStringLiteral("-"));
            QString path = dir.filePath(name + (ext.isEmpty() ? QStringLiteral(".png") : ext));
            QFile file(path);
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(data);
                file.close();
                ctx.writtenMedia.insert(media, path);
                return path;
            }
            break;
        }
    }
    ctx.writtenMedia.insert(media, QString());
    return QString();
}

QString functionName(const Node *function, const Kind &kind)
{
    const QString attribute = function->attr("Attribute");
    if (attribute == QLatin1String("NoFeature"))
        return QStringLiteral("No function");
    // the default names of GDTF are the attribute and a number: not worth showing
    QString name = function->attr("Name").trimmed();
    static const QRegularExpression generated(QStringLiteral("^(.*?)\\s*\\d*$"));
    const QString stem = generated.match(name).captured(1);
    if (name.isEmpty() || stem == attribute || stem == function->parent->attr("Attribute"))
        return attribute == function->parent->attr("Attribute") ? kind.name : kindOf(attribute).name;
    return name;
}

/** The capability preset of a function or a set, from its attribute and its name */
QLCCapability::Preset capabilityPreset(const QString &attribute, const QString &name, const Node *function)
{
    const QString n = name.toLower();
    const bool shutter = attribute.startsWith(QLatin1String("Shutter")) || attribute.startsWith(QLatin1String("Strobe"));
    if (shutter && (attribute.endsWith(QLatin1String("StrobeRandom")) || n.contains(QLatin1String("random"))))
        return QLCCapability::StrobeRandom;
    if (shutter && (attribute.contains(QLatin1String("Strobe")) || n.contains(QLatin1String("strobe"))))
    {
        const double from = function->attr("PhysicalFrom", "0").toDouble();
        const double to = function->attr("PhysicalTo", "1").toDouble();
        return from <= to ? QLCCapability::StrobeSlowToFast : QLCCapability::StrobeFastToSlow;
    }
    if (shutter && (n == QLatin1String("open") || n.startsWith(QLatin1String("open "))))
        return QLCCapability::ShutterOpen;
    if (shutter && (n.startsWith(QLatin1String("close")) || n == QLatin1String("blackout")))
        return QLCCapability::ShutterClose;
    return QLCCapability::Custom;
}

struct Piece
{
    int from;
    int to;
    QString name;
    QLCCapability::Preset preset = QLCCapability::Custom;
    QVariant resource;
};

Piece piece(int from, int to, const QString &name, QLCCapability::Preset preset = QLCCapability::Custom)
{
    Piece p;
    p.from = from;
    p.to = to;
    p.name = name;
    p.preset = preset;
    return p;
}

/** The capabilities of a DMX channel, in steps of its coarse byte */
QList<Piece> capabilitiesOf(Context &ctx, const Node *dmxChannel, int bytes, const Kind &kind)
{
    const quint64 maxValue = (quint64(1) << (8 * bytes)) - 1;
    QList<Piece> pieces;

    for (const Node *logical : dmxChannel->all("LogicalChannel"))
    {
        const QList<const Node *> functions = logical->all("ChannelFunction");
        int lastTo = -1;
        quint64 lastFrom = 0;
        for (int i = 0; i < functions.count(); i++)
        {
            const Node *f = functions.at(i);
            const quint64 from = inResolution(parseDmx(f->attr("DMXFrom", "0/1")), bytes);
            quint64 to = maxValue;
            if (i + 1 < functions.count())
            {
                const quint64 next = inResolution(parseDmx(functions.at(i + 1)->attr("DMXFrom", "0/1")), bytes);
                if (next > from)
                    to = next - 1;
            }
            // the functions of another mode of a mode master start again from 0
            if (i > 0 && from < lastFrom)
                lastTo = -1;
            lastFrom = from;

            const QString fname = functionName(f, kind);
            const QString attribute = f->attr("Attribute");
            const QList<Slot> slots_ = ctx.wheels.value(f->attr("Wheel"));
            const bool colourWheel = attribute.startsWith(QLatin1String("Color"));

            QList<Piece> parts;
            const QList<const Node *> sets = f->all("ChannelSet");
            quint64 cursor = from;
            for (int s = 0; s < sets.count(); s++)
            {
                const Node *set = sets.at(s);
                quint64 sFrom = inResolution(parseDmx(set->attr("DMXFrom", "0/1")), bytes);
                if (sFrom < from)
                    sFrom = from;
                quint64 sTo = to;
                if (s + 1 < sets.count())
                {
                    const quint64 next = inResolution(parseDmx(sets.at(s + 1)->attr("DMXFrom", "0/1")), bytes);
                    if (next > sFrom)
                        sTo = qMin(to, next - 1);
                }
                // what comes before the first set is the function itself
                if (sFrom > cursor)
                    parts.append(piece(coarse(cursor, bytes), coarse(sFrom - 1, bytes), fname));

                Piece part = piece(coarse(sFrom, bytes), coarse(sTo, bytes), set->attr("Name").trimmed());
                const int slotIndex = set->attr("WheelSlotIndex", "0").toInt();
                if (slotIndex > 0 && slotIndex <= slots_.count())
                {
                    const Slot &slot = slots_.at(slotIndex - 1);
                    if (part.name.isEmpty())
                        part.name = slot.name;
                    const QString picture = mediaPath(ctx, slot.media);
                    if (picture.isEmpty() == false)
                    {
                        part.preset = QLCCapability::GoboMacro;
                        part.resource = picture;
                    }
                    else if (colourWheel && slot.color.isValid())
                    {
                        part.preset = QLCCapability::ColorMacro;
                        part.resource = slot.color;
                    }
                }
                if (part.name.isEmpty())
                    part.name = fname;
                if (part.preset == QLCCapability::Custom)
                    part.preset = capabilityPreset(attribute, part.name, f);
                parts.append(part);
                cursor = sTo + 1;
            }
            if (sets.isEmpty())
                parts.append(piece(coarse(from, bytes), coarse(to, bytes), fname, capabilityPreset(attribute, fname, f)));
            else if (cursor <= to)
                parts.append(piece(coarse(cursor, bytes), coarse(to, bytes), fname));

            // in steps of the coarse byte: two values of a 16 bit channel can share one
            for (Piece p : std::as_const(parts))
            {
                if (p.from <= lastTo)
                    p.from = lastTo + 1;
                if (p.from > p.to)
                    continue;
                lastTo = p.to;
                pieces.append(p);
            }
        }
    }
    return pieces;
}

/** The default value of a DMX channel, in its resolution */
quint64 defaultOf(const Node *dmxChannel, int bytes)
{
    // GDTF 1.0: on the channel; later: the default of its initial function
    if (dmxChannel->attrs.contains(QStringLiteral("Default")))
        return inResolution(parseDmx(dmxChannel->attr("Default")), bytes);
    const QString initial = dmxChannel->attr("InitialFunction").section('.', -1);
    const Node *first = nullptr;
    for (const Node *logical : dmxChannel->all("LogicalChannel"))
    {
        for (const Node *f : logical->all("ChannelFunction"))
        {
            if (first == nullptr)
                first = f;
            if (initial.isEmpty() == false && f->attr("Name") == initial)
                return inResolution(parseDmx(f->attr("Default", f->attr("DMXFrom", "0/1"))), bytes);
        }
    }
    return first != nullptr ? inResolution(parseDmx(first->attr("Default", "0/1")), bytes) : 0;
}

/** The DMX addresses of a channel, highest byte first ("1,2"); GDTF 0.87 had Coarse/Fine */
QList<int> offsetsOf(const Node *dmxChannel)
{
    QList<int> offsets;
    const QString offset = dmxChannel->attr("Offset");
    if (offset.isEmpty() == false)
    {
        if (offset.compare("None", Qt::CaseInsensitive) == 0)
            return offsets;
        for (const QString &part : offset.split(',', Qt::SkipEmptyParts))
            if (part.trimmed().toInt() > 0)
                offsets.append(part.trimmed().toInt());
        return offsets;
    }
    for (const char *byte : { "Coarse", "Fine", "Ultra", "Uber" })
    {
        int value = dmxChannel->attr(byte, "0").toInt();
        if (value <= 0)
            break;
        offsets.append(value);
    }
    return offsets;
}

/** A channel of the definition, made once for every mode that has the same one */
QString signature(const QLCChannel *ch)
{
    QStringList parts = { ch->name(), QString::number(ch->group()), QString::number(ch->colour()),
                          QString::number(ch->controlByte()), QString::number(ch->preset()),
                          QString::number(ch->defaultValue()) };
    for (const QLCCapability *cap : ch->capabilities())
        parts << QString("%1-%2-%3-%4-%5").arg(cap->min()).arg(cap->max()).arg(cap->name())
                     .arg(int(cap->preset())).arg(cap->resource(0).toString());
    return parts.join('|');
}

} // namespace

/*********************************************************************
 * GdtfImporter
 *********************************************************************/

QColor GdtfImporter::cieToColor(const QString &xyY)
{
    const QStringList parts = xyY.split(',');
    if (parts.count() < 2)
        return QColor();
    const double x = parts.at(0).trimmed().toDouble();
    const double y = parts.at(1).trimmed().toDouble();
    if (y <= 0.0)
        return QColor();

    // xyY -> XYZ (Y = 1) -> linear sRGB, the brightest at full
    const double X = x / y, Y = 1.0, Z = (1.0 - x - y) / y;
    double rgb[3] = { 3.2406 * X - 1.5372 * Y - 0.4986 * Z,
                      -0.9689 * X + 1.8758 * Y + 0.0415 * Z,
                      0.0557 * X - 0.2040 * Y + 1.0570 * Z };
    double top = 0.0;
    for (double &c : rgb)
    {
        c = qMax(0.0, c);
        top = qMax(top, c);
    }
    if (top <= 0.0)
        return QColor(Qt::black);
    for (double &c : rgb)
    {
        c /= top;
        c = c <= 0.0031308 ? 12.92 * c : 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
    }
    return QColor::fromRgbF(float(qBound(0.0, rgb[0], 1.0)), float(qBound(0.0, rgb[1], 1.0)),
                            float(qBound(0.0, rgb[2], 1.0)));
}

QByteArray GdtfImporter::zipFileData(const QZipReader &zip, const QString &name)
{
    QByteArray data = zip.fileData(name);
    if (data.isEmpty())
    {
        // the name as fileData() decodes it: the UTF-8 bytes read as local 8-bit
        const QString local = QString::fromLocal8Bit(name.toUtf8());
        if (local != name)
            data = zip.fileData(local);
    }
    return data;
}

QLCFixtureDef *GdtfImporter::read(const QByteArray &gdtf, QString *error, QStringList *warnings, const QDir *wheelsDir)
{
    QString dummyError;
    QStringList dummyWarnings;
    if (error == nullptr)
        error = &dummyError;
    if (warnings == nullptr)
        warnings = &dummyWarnings;

    QBuffer buffer;
    buffer.setData(gdtf);
    buffer.open(QIODevice::ReadOnly);
    QZipReader zip(&buffer);
    const bool isZip = zip.status() == QZipReader::NoError && zip.count() > 0;
    QByteArray description;
    if (isZip)
        description = zip.fileData(QStringLiteral("description.xml"));
    else if (gdtf.trimmed().startsWith('<'))
        description = gdtf;   // a description.xml alone
    if (description.isEmpty())
    {
        *error = QObject::tr("Not a GDTF file: there is no description.xml");
        return nullptr;
    }

    std::unique_ptr<Node> root = parseXml(description, error);
    if (root == nullptr)
        return nullptr;
    const Node *gdtfNode = root->child("GDTF");
    const Node *type = gdtfNode != nullptr ? gdtfNode->child("FixtureType") : nullptr;
    if (type == nullptr)
    {
        *error = QObject::tr("Not a GDTF file: there is no fixture type");
        return nullptr;
    }

    Context ctx;
    ctx.fixtureType = type;
    ctx.zip = isZip ? &zip : nullptr;
    ctx.wheelsDir = wheelsDir;
    ctx.warnings = warnings;

    const QString manufacturer = type->attr("Manufacturer").trimmed();
    QString model = type->attr("Name").trimmed();
    if (model.isEmpty())
        model = type->attr("LongName").trimmed();
    if (manufacturer.isEmpty() || model.isEmpty())
    {
        *error = QObject::tr("The GDTF file has no manufacturer or name");
        return nullptr;
    }
    ctx.fileBase = QString("%1-%2").arg(manufacturer, model);
    ctx.fileBase.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_\\-\\.]+")), QStringLiteral("-"));

    // the wheels and their slots
    if (const Node *wheels = type->child("Wheels"))
    {
        for (const Node *wheel : wheels->all("Wheel"))
        {
            QList<Slot> slots_;
            for (const Node *slot : wheel->all("Slot"))
            {
                Slot s;
                s.name = slot->attr("Name").trimmed();
                if (slot->attrs.contains(QStringLiteral("Color")))
                    s.color = cieToColor(slot->attr("Color"));
                s.media = slot->attr("MediaFileName").trimmed();
                slots_.append(s);
            }
            ctx.wheels.insert(wheel->attr("Name"), slots_);
        }
    }

    const Node *geometries = type->child("Geometries");
    if (geometries != nullptr)
        collectGeometries(geometries, ctx.geometries);

    const Node *modesNode = type->child("DMXModes");
    if (modesNode == nullptr || modesNode->all("DMXMode").isEmpty())
    {
        *error = QObject::tr("The GDTF file has no DMX modes");
        return nullptr;
    }

    auto def = std::make_unique<QLCFixtureDef>();
    def->setManufacturer(manufacturer);
    def->setModel(model);
    def->setAuthor(QStringLiteral("GDTF"));

    QHash<QString, QLCChannel *> channelsBySignature;
    QSet<QString> channelNames;
    bool hasPan = false, hasTilt = false, hasColour = false, hasDimmer = false, hasShutter = false;
    bool fog = false, haze = false, otherFeatures = false;
    int maxHeads = 0;

    // a channel object for the definition: the same one is shared by the modes
    auto addChannel = [&](QLCChannel *ch) -> QLCChannel * {
        const QString sig = signature(ch);
        if (QLCChannel *existing = channelsBySignature.value(sig))
        {
            delete ch;
            return existing;
        }
        // a different channel with a name already used
        QString name = ch->name();
        for (int n = 2; channelNames.contains(name.toLower()); n++)
            name = QString("%1 (%2)").arg(ch->name()).arg(n);
        ch->setName(name);
        channelNames.insert(name.toLower());
        def->addChannel(ch);
        channelsBySignature.insert(sig, ch);
        return ch;
    };

    for (const Node *modeNode : modesNode->all("DMXMode"))
    {
        const QString modeName = modeNode->attr("Name").trimmed();
        const Node *top = ctx.geometries.value(modeNode->attr("Geometry"));
        const Node *channelsNode = modeNode->child("DMXChannels");
        if (channelsNode == nullptr)
            continue;

        // the geometries used through references (the cells of a bar), and their references
        QList<const Node *> references;
        if (top != nullptr)
            findReferences(top, references);
        QHash<const Node *, QList<const Node *>> referencesOf;   // template -> its references, in order
        for (const Node *ref : std::as_const(references))
            if (const Node *templ = ctx.geometries.value(ref->attr("Geometry")))
                referencesOf[templ].append(ref);

        struct Placed
        {
            int address;
            QLCChannel *channel;
            int head;
        };
        QList<Placed> placed;
        QSet<QString> usedNames;
        bool otherBreaks = false;

        for (const Node *dmxChannel : channelsNode->all("DMXChannel"))
        {
            const QList<int> offsets = offsetsOf(dmxChannel);
            const QList<const Node *> logicals = dmxChannel->all("LogicalChannel");
            if (offsets.isEmpty() || logicals.isEmpty())
                continue;   // virtual: no DMX

            const QString attribute = logicals.first()->attr("Attribute");
            const Kind kind = kindOf(attribute);
            const int bytes = int(offsets.count());

            hasPan = hasPan || kind.group == QLCChannel::Pan;
            hasTilt = hasTilt || kind.group == QLCChannel::Tilt;
            hasColour = hasColour || kind.colour != QLCChannel::NoColour || kind.group == QLCChannel::Colour;
            hasDimmer = hasDimmer || attribute == QLatin1String("Dimmer");
            hasShutter = hasShutter || kind.group == QLCChannel::Shutter;
            fog = fog || attribute.startsWith(QLatin1String("Fog"));
            haze = haze || attribute.startsWith(QLatin1String("Haze"));
            otherFeatures = otherFeatures || kind.group == QLCChannel::Gobo || kind.group == QLCChannel::Prism ||
                            kind.group == QLCChannel::Beam;

            // the instances: once, or once for each reference to its geometry
            const Node *geometry = ctx.geometries.value(dmxChannel->attr("Geometry"));
            const Node *templ = geometry != nullptr && geometries != nullptr ? topLevelOf(geometry, geometries) : nullptr;
            QList<const Node *> instances = templ != nullptr ? referencesOf.value(templ) : QList<const Node *>();
            const bool referenced = instances.isEmpty() == false;
            if (referenced == false)
                instances.append(nullptr);

            // one plain function: the preset of QLC+ says it all
            QList<const Node *> functions;
            for (const Node *logical : logicals)
                functions.append(logical->all("ChannelFunction"));
            bool plain = functions.count() == 1 && functions.first()->all("ChannelSet").isEmpty() &&
                         inResolution(parseDmx(functions.first()->attr("DMXFrom", "0/1")), bytes) == 0;
            QLCChannel::Preset preset = plain ? kind.preset : QLCChannel::Custom;
            QLCChannel::Preset finePreset = plain ? kind.finePreset : QLCChannel::Custom;
            if (preset == QLCChannel::BeamZoomSmallBig && functions.first()->attr("PhysicalFrom", "0").toDouble() >
                                                           functions.first()->attr("PhysicalTo", "1").toDouble())
                preset = QLCChannel::BeamZoomBigSmall;
            if (preset == QLCChannel::ShutterIrisMinToMax && functions.first()->attr("PhysicalFrom", "0").toDouble() >
                                                              functions.first()->attr("PhysicalTo", "1").toDouble())
                preset = QLCChannel::ShutterIrisMaxToMin;

            const QList<Piece> pieces = capabilitiesOf(ctx, dmxChannel, bytes, kind);
            const quint64 defaultValue = defaultOf(dmxChannel, bytes);

            for (int instance = 0; instance < instances.count(); instance++)
            {
                const Node *ref = instances.at(instance);
                int dmxBreak = dmxChannel->attr("DMXBreak", "1").toInt();
                int shift = 0;
                if (ref != nullptr)
                {
                    const QList<const Node *> breaks = ref->all("Break");
                    const Node *chosen = nullptr;
                    if (dmxChannel->attr("DMXBreak").compare("Overwrite", Qt::CaseInsensitive) == 0)
                        chosen = breaks.isEmpty() ? nullptr : breaks.last();
                    else
                        for (const Node *b : breaks)
                            if (b->attr("DMXBreak", "1").toInt() == qMax(1, dmxBreak))
                                chosen = b;
                    if (chosen != nullptr)
                    {
                        dmxBreak = chosen->attr("DMXBreak", "1").toInt();
                        shift = chosen->attr("DMXOffset", "1").toInt() - 1;
                    }
                }
                if (dmxBreak > 1)
                {
                    otherBreaks = true;
                    continue;
                }

                // its name: "Red", "Red 2" for the second cell
                QString name = kind.name;
                if (referenced && instances.count() > 1)
                    name += QString(" %1").arg(instance + 1);
                for (int n = 2; usedNames.contains(name.toLower()); n++)
                    name = QString("%1 %2").arg(kind.name).arg(n);
                usedNames.insert(name.toLower());

                for (int b = 0; b < bytes; b++)
                {
                    QLCChannel *ch = new QLCChannel();
                    const bool fine = b > 0;
                    const QLCChannel::Preset p = fine ? finePreset : preset;
                    if (p != QLCChannel::Custom)
                    {
                        ch->setPreset(p);
                        ch->addPresetCapability();
                    }
                    else
                    {
                        ch->setGroup(kind.group);
                        if (kind.group == QLCChannel::Intensity)
                            ch->setColour(kind.colour);
                        ch->setControlByte(fine ? QLCChannel::LSB : QLCChannel::MSB);
                        if (fine)
                        {
                            ch->addCapability(new QLCCapability(0, 255, QString("%1 fine").arg(name)));
                        }
                        else
                        {
                            for (const Piece &piece : pieces)
                            {
                                QLCCapability *cap = new QLCCapability(uchar(piece.from), uchar(piece.to), piece.name);
                                if (piece.preset != QLCCapability::Custom)
                                {
                                    cap->setPreset(piece.preset);
                                    if (piece.resource.isValid())
                                        cap->setResource(0, piece.resource);
                                }
                                if (ch->addCapability(cap) == false)
                                    delete cap;   // a function of another mode master
                            }
                            if (ch->capabilities().isEmpty())
                                ch->addCapability(new QLCCapability(0, 255, name));
                        }
                    }
                    ch->setName(fine ? (b == 1 ? QString("%1 fine").arg(name) : QString("%1 fine %2").arg(name).arg(b))
                                     : name);
                    ch->setDefaultValue(uchar((defaultValue >> (8 * (bytes - 1 - b))) & 0xff));
                    placed.append({ offsets.at(b) + shift, addChannel(ch), referenced ? instance : -1 });
                }
            }
        }

        if (otherBreaks)
            warnings->append(QObject::tr("Mode %1: only the first DMX break is used").arg(modeName));
        if (placed.isEmpty())
        {
            warnings->append(QObject::tr("Mode %1 has no DMX channels").arg(modeName));
            continue;
        }

        // the channels in the order of their addresses, the gaps as unused channels
        std::sort(placed.begin(), placed.end(), [](const Placed &a, const Placed &b) { return a.address < b.address; });
        QLCFixtureMode *mode = new QLCFixtureMode(def.get());
        mode->setName(modeName.isEmpty() ? QString("%1 ch").arg(placed.last().address) : modeName);
        QHash<int, QLCFixtureHead> heads;
        quint32 position = 0;
        int address = 1;
        int unused = 0;
        bool collision = false;
        for (const Placed &p : std::as_const(placed))
        {
            if (p.address < address)
            {
                collision = true;   // two channels on one address: the first stays
                continue;
            }
            while (address < p.address)
            {
                QLCChannel *gap = new QLCChannel();
                gap->setName(QString("Not used %1").arg(++unused));
                gap->setGroup(QLCChannel::Nothing);
                gap->addCapability(new QLCCapability(0, 255, QStringLiteral("No function")));
                mode->insertChannel(addChannel(gap), position++);
                address++;
            }
            if (mode->insertChannel(p.channel, position) == false)
            {
                // the same channel twice in a mode: a copy of its own
                QLCChannel *copy = p.channel->createCopy();
                copy->setName(QString("%1 (%2)").arg(p.channel->name()).arg(p.address));
                mode->insertChannel(addChannel(copy), position);
            }
            if (p.head >= 0)
                heads[p.head].addChannel(position);
            position++;
            address++;
        }
        if (collision)
            warnings->append(QObject::tr("Mode %1: two channels on the same address, the first one is used").arg(modeName));
        QList<int> headIndexes = heads.keys();
        std::sort(headIndexes.begin(), headIndexes.end());
        if (headIndexes.count() > 1)
        {
            for (int h : std::as_const(headIndexes))
                mode->insertHead(-1, heads.value(h));
            maxHeads = qMax(maxHeads, int(headIndexes.count()));
        }

        // the physical part: size, beam, power, pan and tilt
        QLCPhysical physical;
        if (top != nullptr)
        {
            if (const Node *models = type->child("Models"))
            {
                for (const Node *m : models->all("Model"))
                {
                    if (m->attr("Name") != top->attr("Model"))
                        continue;
                    physical.setWidth(qRound(m->attr("Length", "0").toDouble() * 1000));
                    physical.setDepth(qRound(m->attr("Width", "0").toDouble() * 1000));
                    physical.setHeight(qRound(m->attr("Height", "0").toDouble() * 1000));
                }
            }
            if (const Node *beam = top->tag == QLatin1String("Beam") ? top : findFirst(top, "Beam"))
            {
                physical.setBulbType(beam->attr("LampType"));
                physical.setBulbLumens(qRound(beam->attr("LuminousFlux", "0").toDouble()));
                physical.setBulbColourTemperature(qRound(beam->attr("ColorTemperature", "0").toDouble()));
                physical.setPowerConsumption(qRound(beam->attr("PowerConsumption", "0").toDouble()));
                const double angle = beam->attr("BeamAngle", "0").toDouble();
                physical.setLensDegreesMin(angle);
                physical.setLensDegreesMax(angle);
            }
        }
        for (const Node *dmxChannel : channelsNode->all("DMXChannel"))
        {
            for (const Node *logical : dmxChannel->all("LogicalChannel"))
            {
                const QString attribute = logical->attr("Attribute");
                if (attribute != QLatin1String("Pan") && attribute != QLatin1String("Tilt"))
                    continue;
                for (const Node *f : logical->all("ChannelFunction"))
                {
                    const int range = qRound(qAbs(f->attr("PhysicalTo", "0").toDouble() - f->attr("PhysicalFrom", "0").toDouble()));
                    if (range > 1)
                        attribute == QLatin1String("Pan") ? physical.setFocusPanMax(range) : physical.setFocusTiltMax(range);
                }
            }
        }
        if (hasPan || hasTilt)
            physical.setFocusType(QStringLiteral("Head"));
        if (headIndexes.count() > 1)
            physical.setLayoutSize(QSize(int(headIndexes.count()), 1));
        mode->setPhysical(physical);
        def->addMode(mode);
        if (def->physical().width() == 0)
            def->setPhysical(physical);
    }

    if (def->modes().isEmpty())
    {
        *error = QObject::tr("The GDTF file has no DMX mode that can be used");
        return nullptr;
    }

    // what kind of fixture it is
    QLCFixtureDef::FixtureType fixtureType = QLCFixtureDef::Other;
    if (hasPan && hasTilt)
        fixtureType = QLCFixtureDef::MovingHead;
    else if (maxHeads > 1)
        fixtureType = QLCFixtureDef::LEDBarPixels;
    else if (haze)
        fixtureType = QLCFixtureDef::Hazer;
    else if (fog)
        fixtureType = QLCFixtureDef::Smoke;
    else if (hasColour)
        fixtureType = QLCFixtureDef::ColorChanger;
    else if (hasShutter && otherFeatures == false && hasDimmer == false)
        fixtureType = QLCFixtureDef::Strobe;
    else if (hasDimmer && otherFeatures == false)
        fixtureType = QLCFixtureDef::Dimmer;
    def->setType(fixtureType);

    return def.release();
}

GdtfImporter::Result GdtfImporter::convert(const QByteArray &gdtf, const QDir &target,
                                           const std::function<bool(const QString &, const QString &)> &taken)
{
    Result result;
    QDir wheels(target.filePath(QStringLiteral("wheels")));
    std::unique_ptr<QLCFixtureDef> def(read(gdtf, &result.error, &result.warnings, &wheels));
    if (def == nullptr)
        return result;

    // the same fixture in another library: both can be used
    if (taken && taken(def->manufacturer(), def->model()))
        def->setModel(def->model() + QStringLiteral(" (GDTF)"));

    QString name = QString("%1-%2").arg(def->manufacturer(), def->model());
    name.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_\\-\\.]+")), QStringLiteral("-"));
    target.mkpath(".");
    const QString path = target.filePath(name + QStringLiteral(".qxf"));
    if (def->saveXML(path) != QFile::NoError)
    {
        result.error = QObject::tr("Unable to write %1").arg(path);
        return result;
    }

    result.ok = true;
    result.manufacturer = def->manufacturer();
    result.model = def->model();
    result.file = path;
    return result;
}

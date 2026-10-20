/*
  DMX Total Control
  uiaudit.cpp

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

#include <QQuickWindow>
#include <QQuickItem>
#include <QFont>
#include <QtMath>
#include <QRegularExpression>
#include <algorithm>

#include "uiaudit.h"

namespace
{

/** An item as it is painted: its place in the window, how opaque, where it can be seen */
struct Painted
{
    QQuickItem *item;
    QRectF rect;
    qreal opacity;
    QRectF visibleArea;
};

/** The items in the order they are painted (a parent, then its children by z) */
void collect(QQuickItem *item, qreal opacity, const QRectF &area, QList<Painted> &out)
{
    if (item == nullptr || item->isVisible() == false)
        return;
    opacity *= item->opacity();
    if (opacity < 0.05)
        return;
    const QRectF rect = item->mapRectToScene(QRectF(0, 0, item->width(), item->height()));
    out.append({ item, rect, opacity, area });
    // a clipping item hides what its children paint outside it
    const QRectF childArea = item->clip() ? area.intersected(rect) : area;
    QList<QQuickItem *> children = item->childItems();
    std::stable_sort(children.begin(), children.end(), [](QQuickItem *a, QQuickItem *b) { return a->z() < b->z(); });
    for (QQuickItem *child : std::as_const(children))
        collect(child, opacity, childArea, out);
}

bool isText(const QQuickItem *item)
{
    return item->inherits("QQuickText") || item->inherits("QQuickTextInput") || item->inherits("QQuickTextEdit");
}

/** A few of its ancestors, to find it: "Panel > RowLayout > AppButton" */
QString placeOf(const QQuickItem *item)
{
    QStringList names;
    for (const QQuickItem *it = item->parentItem(); it != nullptr && names.count() < 4; it = it->parentItem())
    {
        QString name = QString::fromLatin1(it->metaObject()->className());
        name = name.section("_QMLTYPE", 0, 0).section("_QML_", 0, 0);
        if (it->objectName().isEmpty() == false)
            name += "#" + it->objectName();
        names.prepend(name);
    }
    return names.join(" > ");
}

} // namespace

qreal UiAudit::contrast(const QColor &a, const QColor &b)
{
    auto luminance = [](const QColor &c) {
        auto channel = [](qreal v) { return v <= 0.03928 ? v / 12.92 : qPow((v + 0.055) / 1.055, 2.4); };
        return 0.2126 * channel(c.redF()) + 0.7152 * channel(c.greenF()) + 0.0722 * channel(c.blueF());
    };
    const qreal la = luminance(a), lb = luminance(b);
    return (qMax(la, lb) + 0.05) / (qMin(la, lb) + 0.05);
}

QColor UiAudit::blend(const QColor &top, const QColor &bottom)
{
    const qreal alpha = top.alphaF();
    return QColor::fromRgbF(top.redF() * alpha + bottom.redF() * (1 - alpha),
                            top.greenF() * alpha + bottom.greenF() * (1 - alpha),
                            top.blueF() * alpha + bottom.blueF() * (1 - alpha));
}

QStringList UiAudit::run(QQuickWindow *window)
{
    QStringList findings;
    if (window == nullptr || window->contentItem() == nullptr)
        return findings;

    const QRectF windowRect(0, 0, window->width(), window->height());
    QList<Painted> painted;
    collect(window->contentItem(), 1.0, windowRect, painted);
    QColor windowColour = window->color();
    windowColour.setAlphaF(1.0);

    for (int i = 0; i < painted.count(); i++)
    {
        const Painted &text = painted.at(i);
        QQuickItem *item = text.item;
        if (isText(item) == false)
            continue;
        const QString content = item->property("text").toString().simplified();
        // (rich text: its words alone)
        QString plain = content;
        plain.remove(QRegularExpression(QStringLiteral("<[^>]*>")));
        if (plain.trimmed().isEmpty() || text.rect.width() < 1 || text.rect.height() < 1)
            continue;
        const QPointF centre = text.rect.center();
        // hidden by a clipping parent, or out of the window
        if (text.visibleArea.contains(centre) == false)
            continue;

        // what is under its centre: the window, then each filled rectangle painted before it
        QColor background = windowColour;
        for (int j = 0; j < i; j++)
        {
            const Painted &below = painted.at(j);
            if (below.item->inherits("QQuickRectangle") == false || below.rect.contains(centre) == false ||
                below.visibleArea.contains(centre) == false)
                continue;
            QColor fill = below.item->property("color").value<QColor>();
            if (fill.isValid() == false || fill.alpha() == 0)
                continue;
            fill.setAlphaF(fill.alphaF() * below.opacity);
            background = blend(fill, background);
        }
        QColor foreground = item->property("color").value<QColor>();
        foreground.setAlphaF(foreground.alphaF() * text.opacity);
        const QColor shown = blend(foreground, background);

        const QFont font = item->property("font").value<QFont>();
        const qreal pixels = font.pixelSize() > 0 ? font.pixelSize() : font.pointSizeF() * 96.0 / 72.0;
        const bool large = pixels >= 24 || ((font.bold() || font.weight() >= QFont::Bold) && pixels >= 18.66);
        const bool disabled = item->isEnabled() == false;
        // an icon (the icon font) is a graphic: 3, as large text
        const bool icon = font.family().contains(QStringLiteral("Awesome"), Qt::CaseInsensitive);
        const qreal needed = disabled ? 2.0 : (large || icon) ? 3.0 : 4.5;
        const qreal ratio = contrast(shown, background);
        const QString where = QString("%1,%2 %3x%4").arg(qRound(text.rect.x())).arg(qRound(text.rect.y()))
                                                    .arg(qRound(text.rect.width())).arg(qRound(text.rect.height()));
        const QString what = QString::fromLatin1(item->metaObject()->className()).section("_QMLTYPE", 0, 0);
        if (ratio < needed)
        {
            findings.append(QStringList({ disabled ? "contrast-disabled" : icon ? "contrast-icon" : "contrast",
                                          QString::number(ratio, 'f', 2) + "<" + QString::number(needed, 'f', 1),
                                          shown.name() + " on " + background.name(), where,
                                          placeOf(item) + " > " + what, plain.left(60) }).join('\t'));
        }

        // shortened, out of its box, out of the window
        if (item->property("truncated").toBool())
        {
            findings.append(QStringList({ "shortened", "", "", where, placeOf(item) + " > " + what, plain.left(60) }).join('\t'));
        }
        else if (item->inherits("QQuickText"))
        {
            const qreal contentWidth = item->property("contentWidth").toReal();
            const qreal contentHeight = item->property("contentHeight").toReal();
            if (contentWidth > item->width() + 2 || contentHeight > item->height() + 4)
                findings.append(QStringList({ "overflow", QString("%1x%2 in %3x%4").arg(qRound(contentWidth)).arg(qRound(contentHeight))
                                                                     .arg(qRound(item->width())).arg(qRound(item->height())),
                                              "", where, placeOf(item) + " > " + what, plain.left(60) }).join('\t'));
        }
        if (windowRect.contains(text.rect.adjusted(1, 1, -1, -1)) == false && windowRect.intersects(text.rect))
            findings.append(QStringList({ "off-window", "", "", where, placeOf(item) + " > " + what, plain.left(60) }).join('\t'));
    }
    return findings;
}

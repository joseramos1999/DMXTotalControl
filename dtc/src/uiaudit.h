/*
  DMX Total Control
  uiaudit.h

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

#ifndef UIAUDIT_H
#define UIAUDIT_H

#include <QStringList>
#include <QColor>

class QQuickWindow;

/**
 * Developer option --audit: what the window shows that is hard to read.
 * Each visible text against what is painted under it (the WCAG contrast
 * ratio: 4.5 for text, 3 for large text), the texts shortened with "…" and
 * those that go out of their box or out of the window. A line per finding,
 * tab-separated: kind, ratio or sizes, colours, place, the item, the text.
 */
class UiAudit
{
public:
    static QStringList run(QQuickWindow *window);

    /** The WCAG contrast ratio of two opaque colours, 1..21 */
    static qreal contrast(const QColor &a, const QColor &b);
    /** $top (with its alpha) painted over the opaque $bottom */
    static QColor blend(const QColor &top, const QColor &bottom);
};

#endif

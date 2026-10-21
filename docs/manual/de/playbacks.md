# Playbacks

![Die Seite Playbacks](../img/playbacks.png)

**Playbacks** (F10) ist ein Faderpult. Jedes **Playback** spielt eine Cuelist oder eine beliebige Szene ab und hat ein **GO**, eine Taste **Pause/Zurück**, eine Taste **Flash** und einen **Fader**, der seine Intensität ist. Die Playbacks sind in **Seiten** mit 8 bis 20 organisiert (standardmäßig 10).

So fahren Sie eine Show mit Cuelists am bequemsten: Sie nehmen die Cues aus dem Programmer direkt auf ein Playback auf und starten sie mit GO oder mit der Leertaste.

## Der Streifen eines Playbacks

Von oben nach unten:

- **Die Nummer** auf der Farbe des Playbacks. Sie leuchtet, solange es läuft. Das **×** rechts gibt es frei.
- **Der Name**: Klicken Sie darauf, um das Playback **auszuwählen** (orangefarbener Rand). Ein Playback ohne Inhalt zeigt „Leer“.
- **Der laufende Cue und der nächste** („3 · Strophe“, „▸ 4 · Bridge“) und ein Balken mit dem Fortschritt der Überblendung. Eine gestoppte Cuelist zeigt „Freigegeben“; eine Szene zeigt ihren Typ und ob sie „An“, „Aus“ oder „Pausiert“ ist.
- **Taste 1 (GO)**: der nächste Cue; ist das Playback aus, startet sie es.
- **Taste 2 (Pause / Zurück)**: pausiert eine laufende Überblendung; gibt es keine, geht sie zum vorherigen Cue zurück.
- **Der Fader** und sein Prozentwert.
- **Taste 3 (Flash)**: voll, solange Sie sie drücken.

Die Tasten leuchten je nach Zustand: **GO** grün, solange es läuft (es blinkt schnell während einer Überblendung oder Wartezeit und langsam in Pause), **Pause/Zurück** orange in Pause und **Flash** orange, solange sie gedrückt ist.

### Der Fader

Der Fader ist die Intensität dessen, was das Playback abspielt:

- Ziehen Sie ihn **von null hoch**, startet das Playback.
- Ziehen Sie ihn **auf null**, wird eine Szene freigegeben; eine Cuelist bleibt auf ihrem Cue (das lässt sich in den [Eigenschaften](playbacks.md#eigenschaften-eines-playbacks) ändern).
- Ein **GO mit dem Fader unten** spielt voll ab.

Ein Doppelklick auf den Fader setzt ihn auf voll oder auf null.

## Etwas auf ein Playback legen

Es gibt mehrere Wege:

- **Aus dem Programmer aufnehmen**: siehe [Auf ein Playback aufnehmen](playbacks.md#auf-ein-playback-aufnehmen).
- **„Hinzufügen“** auf einem leeren Playback: **„Neue Cuelist aus dem Programmer“** oder **„Szene hier ablegen…“**.
- **Zuweisen**: Drücken Sie **„Zuweisen“** im Kopf. Rechts erscheint die Szenenliste: Ziehen Sie Szenen auf die Playbacks (solange die Liste offen ist, ersetzt eine auf einem belegten Playback abgelegte Szene dessen Inhalt) oder doppelklicken Sie auf eine, um sie auf das erste freie Playback zu legen.
- **Rechtsklick › „Szene zuweisen…“**: Wählen Sie die Cuelist oder Szene in der Liste und drücken Sie **„Auf Playback N legen“** (oder Doppelklick).

Unter Programmieren können Sie auch eine neue Cuelist aufnehmen und auf das erste freie Playback legen (siehe [Aufnehmen](programar.md#aufnehmen)).

## GO, Zurück und das ausgewählte Playback

Rechts im Kopf finden Sie die Bedienelemente des **ausgewählten Playbacks**:

- **GO** (die große grüne Taste, oder die **Leertaste**): der nächste Cue des ausgewählten Playbacks. Sie zeigt seinen Namen und welcher Cue kommt („Song 1 ▸ 3 · Bridge“).
- **„Zurück“** (**Umschalt+Leertaste**): der vorherige Cue.
- **„Alle freigeben“**: gibt alle Playbacks und alle laufenden Cuelists frei.

Leertaste und Umschalt+Leertaste funktionieren auf dieser Seite, solange Sie nicht in ein Textfeld schreiben. Jeder Druck gibt genau ein GO: Halten Sie die Taste gedrückt, werden die Cues nicht durchlaufen.

### Der Bereich des ausgewählten Playbacks

Unten sehen Sie das ausgewählte Playback („Playback N · Name“), seinen Zustand und seine Tasten: Zurück, **GO**, Pause / Fortsetzen, Freigeben und **„Unter Szenen bearbeiten“**.

Ist es eine Cuelist, erscheint die Liste ihrer Cues (CUE, NAME, AUSLÖSER, VERZÖG., ÜBERBL.). Der laufende Cue trägt einen grünen Balken und der nächste einen grünen Rand. In jeder Zeile:

- **„Weiter“**: Dieser Cue wird der des nächsten GO.
- **„Los“**: springt jetzt zu diesem Cue.

Ein Klick auf eine Zeile markiert sie nur: So wird nichts aus Versehen gestartet.

## Auf ein Playback aufnehmen

1. Bauen Sie unter [Programmieren](programar.md) einen Look.
2. Drücken Sie unter Playbacks **„Aufnehmen“**: Die Taste blinkt rot, und jedes Playback zeigt, was beim Anklicken geschieht.
3. Klicken Sie auf ein Playback:
   - **Leer** („Neue Cuelist“): Eine neue Cuelist mit dem Programmer als Cue 1 wird erstellt.
   - **Mit einer Cuelist** („Cue N aufnehmen“): Ihr nächster Cue wird aufgenommen, nur mit dem, was sich gegenüber den übernommenen Werten ändert.
   - **Mit einer Szene** („Enthält eine Szene“): Darauf lässt sich nicht aufnehmen.

**Esc** bricht die aktivierte Aufnahme ab; beim Verlassen der Seite wird sie ebenfalls deaktiviert.

### Einen Cue aktualisieren

Ändern Sie die Werte im Programmer, wählen Sie das Playback aus und drücken Sie **„Aktualisieren ▾“**:

- **„Cue N aktualisieren (auch für die folgenden Cues)“**: Die Änderung geht in die folgenden Cues über, die diese Kanäle übernehmen.
- **„Nur dieser Cue (die folgenden Cues behalten ihren Look)“**.

Aktualisiert wird der Cue, der in den Programmer geladen ist, oder, wenn keiner geladen ist, der laufende. Das Rechtsklickmenü jedes Playbacks enthält außerdem **„Cue N aufnehmen“**, **„Aktuellen Cue aktualisieren (auch für die folgenden Cues)“** und **„Nur den aktuellen Cue aktualisieren“**.

> **Tipp:** Drücken Sie nach dem Aufnehmen unter Programmieren „Alles zurücksetzen“ (oder haken Sie „Programmer nach dem Aufnehmen leeren“ an): Solange der Programmer Werte enthält, hat er Vorrang vor dem, was Sie starten.

## Seiten

- Klicken Sie auf den Namen einer Seite, um sie zu sehen. **◀ ▶** (oder **Bild↑** / **Bild↓**) wechseln zur vorherigen oder nächsten.
- **„+ Seite“** fügt eine Seite hinzu.
- **„Fader pro Seite“**: 8, 10, 12, 16 oder 20 Playbacks pro Seite.
- Ein Doppelklick auf eine Seite benennt sie um. Rechtsklick: **„Umbenennen…“**, **„Nach links“**, **„Nach rechts“** und **„Seite löschen…“** (bei mehr als einer Seite). Beim Löschen werden ihre Playbacks geleert; was sie abspielen, läuft weiter, und die Cuelists und Szenen bleiben unter Szenen.

## Eigenschaften eines Playbacks

Rechtsklick › **„Eigenschaften…“**. Änderungen gelten sofort.

- **Beschriftung**: der angezeigte Name (leer: der der Szene).
- **Farbe**: ein Farbfeld, oder das **A** für die Farbe ihres Szenentyps.
- **Taste 1**, **Taste 2** und **Taste 3**: was jede tut: „Nichts“, „GO“, „Zurück“, „Pause“, „Pause / Zurück“, „Freigeben“, „Flash“, „Flash + GO“ (voll, solange gedrückt, und der nächste Cue), „An / Aus“ oder „Auswählen“ (damit Main GO, Aufnehmen und Aktualisieren darauf wirken).
- **Der Fader startet es**: ob das Hochziehen des Faders das Playback startet.
- **Der Fader auf null gibt es frei**: „Standard“ (eine Szene ja, eine Cuelist nein: Sie behält ihren Cue), „Ja“ oder „Nein“.

Dasselbe Menü enthält **„Unter Szenen bearbeiten“**, **„Freigeben“** und **„Playback leeren“** (entfernt den Inhalt vom Playback; die Cuelist oder Szene bleibt unter Szenen).

## Freigeben

- Das **×** an der Nummer eines laufenden Playbacks oder **„Freigeben“** in seinem Menü.
- **„Alle freigeben“** im Kopf.

Das Freigeben eines Playbacks gibt die Szene auch frei, wenn sie in einer Live-Gruppe läuft.

## Akai-APC-Controller

Unter [Zuordnungen](mapeos.md#vorlagen-für-akai-apc) › „Geräte“ ordnet **„Alles zuordnen“** einen ganzen Controller auf einmal den Playbacks zu:

- **Akai APC mini** und **APC mini mk2** → Playbacks: Jede Spalte ist ein Playback 1-8 der aktuellen Seite: sein Fader und, von unten, GO, Pause/Zurück, Flash und seine ersten 5 Cues. Die Taste unter den Pads gibt es frei, der letzte Fader ist der Grand Master, und die Tasten rechts sind vorherige und nächste Seite, GO und Zurück des ausgewählten Playbacks, Blackout, Tap, BeatGO und Alles stoppen.
- **Akai APC40 mkII** → Playbacks: Jede Spur ist ein Playback 1-8, mit seinen ersten 5 Cues auf den Clips; die Device-Regler sind die Fader der Playbacks 9-16.

Die LEDs zeigen den Zustand jedes Playbacks.

## Playbacks in der SHOW und in der Fernsteuerung

- Die [SHOW](show.md#playbacks-in-der-show) kann einen Playback-Bereich haben, der der aktuellen Seite folgt oder eine feste Seite zeigt.
- In der [Fernsteuerung](remoto.md) erscheint eine Seite „Playbacks“ mit ihren Streifen, sobald ein Playback etwas enthält.

## Bei gesperrter Programmierung

Sperren Sie die **„Programmierung“** mit dem Schloss in der oberen Leiste, funktionieren die Playbacks weiter (GO, Fader, Flash), aber Aufnehmen, Aktualisieren, Zuweisen und das Ändern der Seiten oder Eigenschaften sind nicht möglich.

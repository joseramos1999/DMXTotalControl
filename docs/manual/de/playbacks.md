# Playbacks

![Die Seite Playbacks](../img/playbacks.png)

**Playbacks** (F10) ist ein Faderpult. Jedes **Playback** spielt eine Cuelist oder eine beliebige Szene ab und hat ein **GO**, eine Taste **Pause/Zurück**, eine Taste **Flash** und einen **Fader**, der seine Intensität ist. Die Playbacks sind in **Seiten** mit 8 bis 20 organisiert (standardmäßig 10).

So fahren Sie eine Show mit Cuelists am bequemsten: Sie nehmen die Cues aus dem Programmer direkt auf ein Playback auf und starten sie mit GO oder mit der Leertaste.

## Der Streifen eines Playbacks

Von oben nach unten:

- **Die Nummer** auf der Farbe des Playbacks. Sie leuchtet, solange es läuft. Das **×** rechts gibt es frei. Eine Cuelist eines anderen Typs trägt ihr Kennzeichen: **SUB** (Submaster), **CHASE**, **TC** (Timecode) oder **OVR** (Override).
- **Der Name**: Klicken Sie darauf, um das Playback **auszuwählen** (orangefarbener Rand). Ein Playback ohne Inhalt zeigt „Leer“.
- **Der laufende Cue und der nächste** („3 · Strophe“, „▸ 4 · Bridge“) und ein Balken mit dem Fortschritt der Überblendung. Ist der nächste Cue ein Sprung einer Verknüpfung des Cues, trägt er **↺** und die Nummer des Sprungs: „▸ 2 ↺ 1/3“ ist der erste von drei (siehe [Cuelists](escenas.md#cuelists)). Ein Chase zeigt zuerst sein Tempo: „♩ 120 ▸ 4“. Eine Timecode-Cuelist zeigt zuerst ihre Uhr: „TC 00:01:23.4 · 3 · Strophe“. Eine gestoppte Cuelist zeigt „Freigegeben“; eine Szene zeigt ihren Typ und ob sie „An“, „Aus“ oder „Pausiert“ ist.
- **Taste 1 (GO)**: der nächste Cue; ist das Playback aus, startet sie es.
- **Taste 2 (Pause / Zurück)**: pausiert eine laufende Überblendung; gibt es keine, geht sie zum vorherigen Cue zurück.
- **Der Fader** und sein Prozentwert. Ein A/B-Fader zeigt „A/B“ und wie weit seine Überblendung ist.
- **Taste 3 (Flash)**: voll, solange Sie sie drücken (siehe [Der Flash](playbacks.md#der-flash)).

Die Tasten leuchten je nach Zustand: **GO** grün, solange es läuft (es blinkt schnell während einer Überblendung oder Wartezeit und langsam in Pause), **Pause/Zurück** orange in Pause und **Flash** orange, solange sie gedrückt ist und bis ihr Abfall endet.

### Der Flash

Solange Sie **Flash** drücken, ist das Playback voll. Lassen Sie los, geht es auf den Pegel seines Faders zurück. War es aus, startet der Flash es und gibt es beim Loslassen frei.

Mit einem **Anstieg** und einem **Abfall** in seinen [Eigenschaften](playbacks.md#eigenschaften-eines-playbacks) geschieht das nicht schlagartig:

- Beim Drücken steigt es in der Anstiegszeit auf voll, von dort, wo es gerade ist. Ein Playback, das aus ist, steigt von null an.
- Beim Loslassen geht es in der Abfallzeit auf den Faderpegel zurück. Ein Playback, das der Flash gestartet hat, geht auf null und wird am Ende des Abfalls freigegeben.
- Drücken Sie während des Abfalls erneut, steigt es von dort, wo es ist, ohne vorher abzusinken.
- Bewegen Sie den Fader während des Abfalls, endet der Abfall beim neuen Pegel.
- Ein GO während des Abfalls lässt es an: Der Abfall endet beim Faderpegel.

Kanäle, die keine Intensität sind (Position, Farbrad), kommen wie immer sofort.

### Der Fader

Der Fader ist die Intensität dessen, was das Playback abspielt:

- Ziehen Sie ihn **von null hoch**, startet das Playback.
- Ziehen Sie ihn **auf null**, wird eine Szene freigegeben; eine Cuelist bleibt auf ihrem Cue (das lässt sich in den [Eigenschaften](playbacks.md#eigenschaften-eines-playbacks) ändern).
- Ein **GO mit dem Fader unten** spielt voll ab.

Bei einer **Submaster**-Cuelist bestimmt der Fader: Hochziehen von null startet sie auf dem Pegel des Faders, nie voll, und Ziehen auf null gibt sie frei. Freigegeben wählen GO und Zurück nur den Cue, mit dem sie starten wird (siehe [Optionen der Cuelist](escenas.md#optionen-der-cuelist)).

Bei einer **Override**-Cuelist blendet der Fader alles, was sie hält, auch Farben, Positionen und Gobos, zwischen dem, was darunter liegt, und ihrem Cue über. Auf voll ersetzt ihr Cue, was darunter liegt (siehe [Optionen der Cuelist](escenas.md#optionen-der-cuelist)).

Bei einer normalen Cuelist kann der Fader eine **Überblendung (A/B)** sein („Fader“ in den [Eigenschaften](playbacks.md#eigenschaften-eines-playbacks)). Dann ist er nicht die Intensität: Das Playback spielt auf dem Pegel, den es hatte, und jeder Weg des Faders von einem Ende zum anderen ist ein GO, dessen Überblendung Sie von Hand machen.

- Sobald er sein Ende verlässt, startet der nächste Cue, und seine Überblendung folgt dem Fader: 40 % des Wegs sind 40 % der Überblendung. Verzögerungen, Ausblendungen und die Zeiten von Attributen und Fixtures folgen im Verhältnis.
- Am anderen Ende ist der Cue fertig. Der nächste Weg, in die Gegenrichtung, spielt den Cue danach.
- Gehen Sie zurück zum Ende, von dem Sie kamen, wird abgebrochen, und der Cue davor ist wieder da. Hat dieser Weg die Cuelist gestartet, wird sie freigegeben.
- Eine gestoppte Cuelist startet mit dem Cue eines GO, der mit dem Fader einblendet. Ein Cue ohne Zeit blendet linear über den Weg.
- Die FX des Cues starten mit ihrer eigenen Zeit.
- **GO** (die Taste, Main GO oder eine Zuordnung) spielt den nächsten Cue mit seiner Zeit. Mitten in einem Weg beendet es diese Überblendung mit ihrer Zeit und spielt keinen weiteren Cue.
- Folgen und Warten des nächsten Cues warten, bis der Weg zu Ende ist. Cue-Tempo und Live-Zeit ändern die Überblendung, die Sie von Hand machen, nicht.

Ein Doppelklick auf den Fader setzt ihn auf voll oder auf null. Bei einem A/B-Fader ist das ein ganzer Weg: Der Cue ist sofort fertig.

## Etwas auf ein Playback legen

Es gibt mehrere Wege:

- **Aus dem Programmer aufnehmen**: siehe [Auf ein Playback aufnehmen](playbacks.md#auf-ein-playback-aufnehmen).
- **„Hinzufügen“** auf einem leeren Playback: **„Neue Cuelist aus dem Programmer“** oder **„Szene hier ablegen…“**.
- **Zuweisen**: Drücken Sie **„Zuweisen“** im Kopf. Rechts erscheint die Szenenliste: Ziehen Sie Szenen auf die Playbacks (solange die Liste offen ist, ersetzt eine auf einem belegten Playback abgelegte Szene dessen Inhalt) oder doppelklicken Sie auf eine, um sie auf das erste freie Playback zu legen.
- **Rechtsklick › „Szene zuweisen…“**: Wählen Sie die Cuelist oder Szene in der Liste und drücken Sie **„Auf Playback N legen“** (oder Doppelklick).

Unter Programmieren können Sie auch eine neue Cuelist aufnehmen und auf das erste freie Playback legen (siehe [Aufnehmen](programar.md#aufnehmen)).

Auch die Cue-Listen von QLC+ können auf die Playbacks kommen: siehe [Szenen › Cue-Listen von QLC+](escenas.md#cue-listen-von-qlc).

## GO, Zurück und das ausgewählte Playback

Rechts im Kopf finden Sie die Bedienelemente des **ausgewählten Playbacks**:

- **GO** (die große grüne Taste, oder die **Leertaste**): der nächste Cue des ausgewählten Playbacks. Sie zeigt seinen Namen und welcher Cue kommt („Song 1 ▸ 3 · Bridge“).
- **„Zurück“** (**Umschalt+Leertaste**): der vorherige Cue.
- **„Alle freigeben“**: gibt alle Playbacks und alle laufenden Cuelists frei.
- **„Cue-Tempo“**: das Tempo aller Cuelists (siehe [Cue-Tempo](playbacks.md#cue-tempo)).

Leertaste und Umschalt+Leertaste funktionieren auf dieser Seite, solange Sie nicht in ein Textfeld schreiben. Jeder Druck gibt genau ein GO: Halten Sie die Taste gedrückt, werden die Cues nicht durchlaufen.

### Der Bereich des ausgewählten Playbacks

Unten sehen Sie das ausgewählte Playback („Playback N · Name“), seinen Zustand und seine Tasten: Zurück, **GO**, Pause / Fortsetzen, Freigeben und **„Unter Szenen bearbeiten“**.

Ist es eine Cuelist, erscheint die Liste ihrer Cues (CUE, NAME, AUSLÖSER, VERZÖG., ÜBERBL.). Der laufende Cue trägt einen grünen Balken und der nächste einen grünen Rand. In jeder Zeile:

- **„Weiter“**: Dieser Cue wird der des nächsten GO.
- **„Los“**: springt jetzt zu diesem Cue.

Ein Klick auf eine Zeile markiert sie nur: So wird nichts aus Versehen gestartet.

## Cue-Tempo und Live-Zeit

### Cue-Tempo

**„Cue-Tempo“** im Kopf ändert das Tempo aller Cuelists auf einmal: ihre Überblendungen, Verzögerungen, Folgen und Warten, ihren Tap-Rhythmus und die Schritte eines Chase. Es reicht von ×0,1 bis ×10, mit ×1 in der Mitte.

- Es wirkt sofort, auch auf laufende Überblendungen: Sie werden von dort aus, wo sie sind, schneller oder langsamer, ohne Sprünge.
- Der Wert wird orange, wenn er nicht ×1 ist. Ein Doppelklick setzt ihn auf ×1 zurück.
- Es ändert weder FX noch die anderen Szenen.
- Es wird nicht gespeichert: Jede Show öffnet mit ×1.

Es gibt es auch auf den Fadern der [SHOW](show.md#fader) und in den [Zuordnungen](mapeos.md#steuert).

### Live-Zeit

Während ein Cue überblendet, zeigt der Bereich des ausgewählten Playbacks **„Live-Zeit“**: die Verzögerung plus die Überblendung dieses Cues (zum Beispiel „4,0 s“). Sie können sie ändern, während sie läuft:

- Geben Sie eine andere Zeit ein, oder drücken Sie **„×½“** oder **„×2“**: Die Überblendung läuft von dort aus weiter, wo sie ist, und endet in der neuen Zeit.
- **„Jetzt“** beendet die Überblendung sofort.
- Hat der nächste Cue **Folgen**, wartet er auf das neue Ende.

Die Änderung gilt nur für diesen Durchlauf: Der nächste Cue nutzt seine eigenen Zeiten. „Live-Zeit“ wird orange, wenn die Zeit nicht die des Cues ist. **„Im Cue speichern“** speichert die geänderten Zeiten im Cue (auch die seiner Attribute und Fixtures). Das geht nicht, wenn der Cue mit einer BACK-Zeit oder mit der Einblendung einer Gruppe oder eines Blocks läuft: Das sind nicht seine Zeiten. Das lässt sich wie jede andere Bearbeitung der Cuelist rückgängig machen.

Dieselbe Leiste steht im Kopf der Cuelist in [Szenen](escenas.md#die-cue-tabelle). Von einem Controller aus nutzen Sie die Zuordnungen „Live-Zeit eines Playbacks“ oder „Live-Zeit des ausgewählten Playbacks“ (siehe [Zuordnungen](mapeos.md#steuert)).

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
- **Flash-Anstieg** und **Flash-Abfall**: wie lange der Flash braucht, um auf voll zu steigen und beim Loslassen zurückzugehen, von 0 bis 10 s (0 = sofort). Sie gelten für seine Taste Flash oder Flash + GO, die Zuordnungen, die SHOW und die Fernsteuerung (siehe [Der Flash](playbacks.md#der-flash)).
- **Fader**: „Intensität“ (Standard) oder „Überblendung (A/B)“, nur bei einer normalen Cuelist (siehe [Der Fader](playbacks.md#der-fader)). Bei A/B fehlen die zwei folgenden Optionen: Der Fader startet sie immer und gibt sie nie frei.
- **Der Fader startet es**: ob das Hochziehen des Faders das Playback startet.
- **Der Fader auf null gibt es frei**: „Standard“ (ja für eine Szene und eine Submaster-Cuelist; nein für die anderen Cuelists: Sie behalten ihren Cue), „Ja“ oder „Nein“.

Dasselbe Menü enthält **„Unter Szenen bearbeiten“**, **„Freigeben“** und **„Playback leeren“** (entfernt den Inhalt vom Playback; die Cuelist oder Szene bleibt unter Szenen).

## Freigeben

- Das **×** an der Nummer eines laufenden Playbacks oder **„Freigeben“** in seinem Menü.
- **„Alle freigeben“** im Kopf.

Das Freigeben eines Playbacks gibt die Szene auch frei, wenn sie in einer Live-Gruppe läuft.

## Akai-APC-Controller

Unter [Zuordnungen](mapeos.md#vorlagen-für-akai-apc) › „Geräte“ ordnet **„Alles zuordnen“** einen ganzen Controller auf einmal den Playbacks zu:

- **Akai APC mini** und **APC mini mk2** → Playbacks: Jede Spalte ist ein Playback 1-8 der aktuellen Seite: sein Fader und, von unten, GO, Pause/Zurück, Flash und seine ersten 5 Cues. Die Taste unter den Pads gibt es frei, der letzte Fader ist der Grand Master, und die Tasten rechts sind vorherige und nächste Seite, GO und Zurück des ausgewählten Playbacks, Blackout, Tap, BeatGO und Alles stoppen.
- **Akai APC40 mkII** → Playbacks: Jede Spur ist ein Playback 1-8, mit seinen ersten 5 Cues auf den Clips (die nächste pulsiert grün); die Device-Regler sind die Fader der Playbacks 9-16.

Die LEDs zeigen den Zustand jedes Playbacks.

## Playbacks in der SHOW und in der Fernsteuerung

- Die [SHOW](show.md#playbacks-in-der-show) kann einen Playback-Bereich haben, der der aktuellen Seite folgt oder eine feste Seite zeigt.
- In der [Fernsteuerung](remoto.md) erscheint eine Seite „Playbacks“ mit ihren Streifen, sobald ein Playback etwas enthält.

## Bei gesperrter Programmierung

Sperren Sie die **„Programmierung“** mit dem Schloss in der oberen Leiste, funktionieren die Playbacks weiter (GO, Fader, Flash), aber Aufnehmen, Aktualisieren, Zuweisen und das Ändern der Seiten oder Eigenschaften sind nicht möglich.

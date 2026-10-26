# Programmieren

![Die Seite Programmieren](../img/programar.png)

**Programmieren** (F3) ist der Programmer: Sie wählen Fixtures aus, geben ihnen Dimmer, Farbe, Position, Gobos oder jeden anderen Kanal, setzen FX und Paletten ein und nehmen das Ergebnis als Szene, Cuelist oder Cue auf.

Was Sie in den Programmer geben, **wird sofort ausgegeben und hat Vorrang vor allem** (Szenen, Gruppen und Playbacks), bis Sie ihn leeren. Der Untertitel der Seite erinnert Sie daran: „N Kanäle im Programmer — sie haben Vorrang vor den Playbacks, bis Sie den Programmer leeren“.

Die Seite enthält von oben nach unten: den Kopf mit seinen Tasten, die [Befehlszeile](programar.md#befehlszeile), links den Bereich **Fixtures** und rechts den Bereich **Schnell** und den Bereich **Features / Fader / Paletten / FX**.

## Der Kopf

- **„Beam on“**: Dimmer voll und Shutter offen in der Auswahl.
- **„Beam off“**: Dimmer auf null in der Auswahl.
- **„Zentrieren“**: Pan und Tilt in die Mitte.
- **„Blind“**: siehe [Blind](programar.md#blind).
- **„Alles zurücksetzen“**: gibt alle Live-Änderungen frei (Werte und FX). Die Szenen sind wieder zu sehen.
- **„Auswahl zurücksetzen“**: gibt nur die Änderungen der ausgewählten Fixtures frei.
- **„Snapshot“** und **„Aufnehmen…“**: siehe [Aufnehmen](programar.md#aufnehmen).

In einem schmalen Fenster zeigen einige Tasten nur ihr Symbol; fahren Sie mit der Maus darüber, um ihren Namen zu sehen.

## Fixtures auswählen

Der Bereich **„Fixtures (N ausgew.)“** zeigt eine Kachel pro Fixture mit Name, Nummer (für die Befehlszeile), Adresse (Universum.Adresse) und Typ.

- **Klick** auf eine Kachel: wählt nur diese aus.
- **Strg+Klick**: fügt sie der Auswahl hinzu oder entfernt sie. **Umschalt+Klick**: wählt alle Kacheln von der zuletzt angeklickten bis zu dieser aus (in dieser Richtung, die die Reihenfolge der Auswahl ist); **Strg+Umschalt+Klick** fügt sie hinzu.
- **„Alle“** und **„Keine“**, im Kopf des Bereichs.
- **„Invertieren“**: wählt die aus, die nicht ausgewählt waren.
- **½**, **⅓**, **¼**: behält jedes zweite, dritte oder vierte.
- **◀ ▶**: verschiebt die Auswahl nach links oder rechts.

**Die Reihenfolge zählt**: Die Reihenfolge, in der Sie die Fixtures auswählen, ist die Reihenfolge, der Fächer, FX und Colour FX folgen. Im 2D-Plan zeigt jedes ausgewählte Fixture seine Nummer in dieser Reihenfolge.

> **Tipp:** Sie können auch per Eingabe auswählen: `1 THRU 8` und Eingabe. Siehe [die Befehlszeile](programar.md#befehlszeile).

### Fixture-Gruppen und Auswahlen

Über den Kacheln erscheinen die [Fixture-Gruppen](patch.md#fixture-gruppen) aus dem Patch als runde Beschriftungen:

- **Klick** auf eine Gruppe: wählt ihre Fixtures aus. **Strg+Klick** oder **Umschalt+Klick**: fügt sie hinzu.
- Die Beschriftung leuchtet auf, wenn alle ihre Fixtures ausgewählt sind. Fahren Sie mit der Maus darüber, um ihre Nummer für die Befehlszeile zu sehen („GROUP 2 in der Befehlszeile“).
- Die **Auswahlen** einer Gruppe (geordnete Teile, mit eigenem Symbol) wählen ihre Fixtures in ihrer Reihenfolge aus.
- Bei vielen Gruppen sind drei Reihen von Beschriftungen zu sehen, die mit dem Mausrad scrollen: **„Alle Gruppen (N)“** öffnet sie (bis zur halben Höhe des Felds), **„Weniger“** klappt sie wieder ein. So bleiben die Kacheln der Fixtures im Blick.

### 2D-Plan

Die Rastertaste im Kopf des Bereichs („2D-Plan“) ersetzt die Kacheln durch den Bühnenplan:

- **Klick** auf ein Fixture wählt es aus; **Strg+Klick** oder **Umschalt+Klick** fügt es hinzu oder entfernt es.
- **Ziehen** Sie über die leere Bühne, um mit einem Rechteck auszuwählen. Ein Klick ins Leere hebt die Auswahl auf.
- Das **Mausrad** zoomt; Ziehen mit der **rechten** oder mittleren Maustaste verschiebt die Ansicht, und ein Doppelklick mit diesen Tasten zentriert sie wieder.

Unter Programmieren ist der Plan immer **live**: Jedes Fixture zeigt seine Farbe, seinen Dimmer, sein Stroboskop (es blinkt) und sein Gobo, und jeder Pixel der Bars und Matrizen ist zu sehen. Die Fixtures werden im [2D-Plan des Patch](patch.md#2d-plan) platziert.

## Der Bereich Schnell

### Dimmer

Der Fader **Dimmer** und die Tasten **100%**, **75%**, **50%**, **25%** und **0%** geben der Auswahl Intensität.

### Farbe

Klicken Sie auf eines der 16 Farbfelder oder bewegen Sie **Farbton** und **Sätt.** Die Farbe wird in den RGB-Kanälen jedes Fixtures gemischt (der Weißanteil geht auf den Weißkanal, falls das Fixture einen hat) oder in den CMY-Kanälen. Für Farbräder verwenden Sie deren Presets unter [Features](programar.md#features) › Farbe.

> **Tipp:** Für einen Dimmer-Fächer verwenden Sie die Befehlszeile: `@ 0 THRU 100` geht von 0 % beim ersten bis 100 % beim letzten. Der Fächer für die Position ist unter [Features › Position](programar.md#position-xy-feld-und-fächer).

## Features

**Features** zeigt die Kanäle der Auswahl in Tabs: **Dimmer**, **Farbe**, **Position**, **Gobo**, **Beam**, **Fokus** und **Sonstige**. Tabs ohne Kanäle in der Auswahl sind deaktiviert.

Jeder Kanal ist eine Karte mit Name, Wert in %, einem Schieberegler und seinen **Presets** als Tasten (mit ihrer Farbe oder dem Bild ihres Gobos, wenn das Profil sie mitbringt). Klicken Sie auf ein Preset, um es zu setzen. Die Karte wird orange markiert, wenn der Kanal im Programmer ist; ihr **×** gibt ihn frei.

Die Kanäle werden **nach Funktion** gruppiert, nicht nach Name: „Red“, „R“ oder „Rot“ verschiedener Fixtures sind derselbe Regler, ebenso Dimmer, Pan/Tilt, CMY, Farbrad, Gobo, Shutter, Iris, Zoom, Fokus, Prisma… Jeder Kopf einer Bar behält seine eigenen Kanäle.

### Farbe: Farbwähler und Favoriten

Im Tab **Farbe** gibt das Feld links den Farbton (von Seite zu Seite) und die Sättigung (von oben nach unten) vor. Ziehen Sie das Fadenkreuz, um es zu platzieren, oder ziehen Sie an einer anderen Stelle des Felds für eine Feineinstellung.

Darunter **„Favoriten“**: Das **+** speichert die aktuelle Farbe; ein Klick auf einen Favoriten setzt ihn; ein Rechtsklick entfernt ihn. Die Favoriten gelten für alle Shows.

### Position: XY-Feld und Fächer

Der Tab **Position** bewegt nur **Pan** und **Tilt** (mit ihren Feinkanälen, in 16 Bit). Die Endlosrotation von Pan oder Tilt und ihre Geschwindigkeit finden Sie unter **Sonstige**.

- Ziehen Sie das **Fadenkreuz**, um es zu platzieren, oder ziehen Sie an einer anderen Stelle des Felds für eine langsame, feine Bewegung.
- Der Balken links ist der **Tilt**, der unten der **Pan**.
- Bei mehreren Fixtures ist jedes ein Punkt im Feld.
- **„Zentrieren“** bringt Pan und Tilt in die Mitte.

Rechts ist der **Fächer**: Er fächert die Position der Auswahl in ihrer Reihenfolge auf, jedes Fixture ausgehend von seiner bisherigen Position.

- **„Offen“**: von einem Ende zum anderen (das erste zur einen Seite, das letzte zur anderen, das mittlere bleibt stehen).
- **„V-Form“**: Die Enden öffnen sich, die Mitte bleibt stehen.
- **„Ab dem ersten“**: Das erste bleibt stehen, jedes weitere bewegt sich etwas mehr.
- **Pan** und **Tilt**: wie weit sich jeder öffnet (von −100 bis 100 %; Doppelklick: auf 0). Bei 100 % bewegen sich die Enden um ein halbes Feld.
- **„Kein Fächer“**: Die Fixtures kehren an ihre Position zurück.
- Die Taste **„Fächer“** unter dem Feld sorgt dafür, dass Ziehen im Feld den Fächer öffnet, statt zu bewegen: seitlich der Pan, auf und ab der Tilt.

Bewegen Sie danach das Fadenkreuz, verschiebt sich die Mitte, und der Fächer bleibt. Ändert sich die Position auf anderem Weg (eine Palette, eine andere Auswahl, „Alles zurücksetzen“), geht der Fächer auf 0 zurück und beginnt neu, von dort, wo die Fixtures gerade stehen.

### Follow: auf einen Punkt zielen

Die Taste **„Follow“** unter dem Feld ersetzt den Fächer durch einen Bühnenplan. Klicken oder ziehen Sie darauf: **Alle ausgewählten Moving Heads zielen auf diesen Punkt**, jeder mit seinem eigenen Pan und Tilt, und eine Linie führt von jedem zum Punkt.

- **„Höhe des Punkts“**: 0 m ist der Boden; erhöhen Sie sie, um zum Beispiel auf das Gesicht der singenden Person zu zielen (1,6 m).
- Jeder Kopf berechnet Pan und Tilt aus seinem Platz im Plan, seiner Höhe, ob er hängt oder steht und wohin er schaut. All das stellen Sie unter [Patch › 2D-Plan › „Höhe und Ausrichtung“](patch.md#höhe-und-ausrichtung-follow) ein.
- Beim Ziehen nimmt jeder Kopf den kurzen Weg von dort, wohin er zielte, ohne die lange Runde zu drehen.
- Die Werte bleiben im Programmer wie jeder Pan und Tilt: Nehmen Sie sie in einer Szene, einem Cue oder einer Palette auf.
- Darunter steht, wie viele Köpfe zielen, welche den Punkt nicht erreichen (sie bleiben so nah wie möglich), welchen der Pan- und Tilt-Bereich in ihrer Definition fehlt (es werden 540° und 270° verwendet) und welche Scanner sich nicht bewegen (ihr Spiegel folgt dieser Geometrie nicht).
- Invertierungen, Grenzen und das Tauschen von Pan und Tilt aus dem [Patch](patch.md#grenzen) werden bereits berücksichtigt.

Bewegen Sie das Fadenkreuz, den Fächer oder „Zentrieren“, oder ändern Sie die Auswahl, verschwindet der Punkt.

> **Nicht mit echten Geräten getestet:** Jeder Hersteller richtet seine Köpfe auf seine eigene Weise aus. Zielt einer falsch, passen Sie seine Ausrichtung an oder kehren Sie seinen Pan oder Tilt unter „Höhe und Ausrichtung“ um.

## Fader

**Fader** zeigt jeden Kanal der Auswahl als senkrechten Fader, mit Name, Gruppe und Wert (0–255). Die Listentaste öffnet seine **Presets** („Name (min–max)“), und das **×** gibt ihn frei.

> **Tipp:** Nachdem Sie auf einen Fader oder Schieberegler geklickt haben, bewegen ihn die Pfeiltasten ↑ ↓ um je einen Schritt und Bild↑ / Bild↓ um je zehn. Ein Doppelklick setzt ihn auf seinen Ruhewert zurück (normalerweise null).

## Blind

Ist **„Blind“** aktiv, bearbeitet der Programmer, **ohne etwas an die Scheinwerfer auszugeben**: Die Bühne zeigt weiter, was läuft, und Sie bereiten den nächsten Look vor. Der Untertitel warnt: „Blind: die Scheinwerfer zeigen nicht, was Sie programmieren“. Sie können ganz normal aufnehmen. Schalten Sie es aus, wird der Inhalt des Programmers ausgegeben.

## Hervorheben und Lokalisieren

In der Leiste der Befehlszeile:

- **„Hervorheben“**: die Auswahl voll in offenem Weiß, über allem, um zu sehen, welche Fixtures es sind. Wird nicht aufgenommen. Drücken Sie erneut, um es aufzuheben.
- **„Lokalisieren“**: setzt die Auswahl voll, in offenem Weiß, zentriert und mit offenem Strahl, **in den Programmer** (diese Werte werden aufgenommen).
- **◀ ▶** (PREV / NEXT): geht die Auswahl **einzeln** durch: Nur ein Fixture bleibt ausgewählt, und Sie sehen, bei welchem Sie sind („2/8“). **„Alle“** kehrt zur ganzen Auswahl zurück (ALL).

## Paletten

Eine **Palette** speichert einen Intensitäts-, Positions-, Farb- oder Beam-Wert für jedes Fixture. Szenen und Cues, die mit einer Palette aufgenommen wurden, verwenden sie **per Referenz**: Aktualisieren Sie die Palette, ändern sie sich von selbst (die Live-Änderungen ebenfalls).

Unter **Paletten** gibt es vier Spalten: **Intensität**, **Position**, **Farbe** und **Beam**.

### Eine Palette aufnehmen

1. Wählen Sie Fixtures aus und geben Sie ihnen den Wert (zum Beispiel eine Farbe).
2. Drücken Sie in der Spalte **Farbe** **„+ Aufnehmen“**.

Die Palette wird mit einem Namen und einer Nummer erstellt („Farbe 1“, COL 1). Ab diesem Moment folgt ihr alles, was Sie mit ihr aufnehmen.

### Eine Palette verwenden

Wählen Sie Fixtures aus und klicken Sie auf die Palette: Sie wird auf die Auswahl angewendet. Eine Palette wird markiert, wenn sie in der Auswahl gesetzt ist (stärker, wenn sie in der ganzen Auswahl gesetzt ist). Bei einem Fixture, das nicht in der Palette aufgenommen wurde, verwendet sie die Werte eines anderen Fixtures desselben Typs oder seine Attribute: Eine Farbe wird in RGB oder CMY gemischt oder geht auf den ähnlichsten Platz des Farbrads.

Auch: `COL 2`, `POS 1`, `BEAM 3` oder `INT 4` in der Befehlszeile.

### Das Menü einer Palette

Rechtsklick auf eine Palette:

- **„Auf die Auswahl anwenden“**.
- **„Fixtures der Palette in den Programmer laden“**.
- **„Mit dem Programmer aktualisieren“**: fügt die Werte des Programmers der Palette hinzu oder ändert sie. Die Szenen, die sie verwenden, ändern sich ebenfalls.
- **„Neu aufnehmen“**: nimmt sie erneut auf, nur mit dem, was jetzt im Programmer ist.
- **„Umbenennen…“**, **„Nummer ändern…“** und **„Löschen“** (die Szenen behalten die Werte).

Ein Verknüpfungssymbol auf der Palette zeigt an, dass Szenen sie verwenden; der Text beim Darüberfahren sagt, wie viele.

> **Tipp:** Paletten lassen sich in die [SHOW](show.md) (Aktion „Eine Palette“), in die [Fernsteuerung](remoto.md) und in eine [Zuordnung](mapeos.md) legen, auf ihre eigenen Fixtures oder auf eine Gruppe, als Live-Änderung. Alt+Klick auf eine Palette bringt sie in die SHOW.

## FX

Ein **FX** bewegt einen Parameter der Auswahl (den Dimmer, das Rot, den Zoom, Pan und Tilt zusammen…) um seinen Wert im Programmer oder, wenn er keinen Wert hat, um das, was die Szenen darunter senden. Er läuft sofort und wird mit der Szene aufgenommen.

Unter **FX** gibt es drei Zonen: links die Parameter und die laufenden FX, in der Mitte die Regler des FX und rechts die Makros.

### Einen FX setzen

1. Wählen Sie Fixtures aus.
2. Wählen Sie unter **Parameter**, was bewegt werden soll. Standardmäßig ist der zuletzt berührte Parameter gewählt. Parameter, die in der Auswahl schon einen FX haben, tragen die Markierung „FX“.
3. Drücken Sie **„FX starten“** oder klicken Sie auf ein **Makro**.
4. Stellen Sie die Regler ein: Der FX ändert sich live.

### Die Regler des FX

- **Die Figur oder der Modus** (die Zeichnung links): Klicken Sie darauf, um eine andere zu wählen. Für Pan/Tilt gibt es **12 Figuren** (Kreis, Acht, Quadrat, Dreieck, Diamant, Pan-Linie, Tilt-Linie, Diagonale, Kleeblatt, Lissajous, Ballyhoo und Kreuz). Für alles andere **22 Modi**: Die gepunktete Linie ist der Grundwert, und die Welle geht darüber, darunter oder zu beiden Seiten; die Modi 9 bis 12 sind für Lauflichter, mit Schritt.
- **Amplitude** (von 0 bis 250 %). Bei Pan/Tilt **„Pan-Amplitude“** und **„Tilt-Amplitude“** getrennt.
- **Tempo**, in BPM. **½** und **×2** halbieren oder verdoppeln es. **„BPM der Show“** lässt es dem Tempo der Show folgen (ein BPM-Fader oder -Drehregler der SHOW oder eine BPM- oder Tap-Zuordnung). In einer FX-Szene aufgenommen, folgt es stattdessen dem Rhythmus, den Sie mit der Taste der Szene vorgeben, wenn sie einen hat ([Live](directo.md#rhythmus-einer-szene-mit-schritten)). **„Multiplikator“**: von ×¼ bis ×8.
- Nur Pan/Tilt: **„Pan invertieren“**, **„Tilt invertieren“**, **„Rückwärts“** und **„Drehen“** (0–359°).

### FX-Timing

Wie sich der FX auf die Fixtures verteilt:

- **„Welle pro X“**: Die Form verteilt sich über X Fixtures (oder Sätze), jedes etwas später.
- **„Schritt pro X“**: ein Fixture (oder Satz) nach dem anderen, jeweils X.
- **„Gruppierung“**: „Aus“, „Jedes X-te“, „Blöcke zu X“, „In X teilen“, „Spiegeln“ oder „Nach Gruppe“ (Fixture-Gruppe).
- **„Reihenfolge“**: „Auswahl“, „Umgekehrt“, „Zufällig“ (mit einer Taste für eine andere Zufallsfolge) oder „Nach Nummer“.
- **„Wartezeit“**: Zeit auf dem Grundwert nach jedem Durchlauf.
- **„Phase“**.

Darunter zeichnet eine animierte Vorschau, wie sich jedes Fixture bewegt.

### Laufende FX, verknüpfen und stoppen

- **„Laufende FX (N)“** listet die FX des Programmers. Ein Klick wählt ihre Fixtures und ihren Parameter aus; das **×** stoppt den FX.
- **„Verknüpfen“**: Verknüpfte FX teilen Tempo und Timing und stoppen gemeinsam.
- **„Stopp“** stoppt den FX des gewählten Parameters; **„Alle FX stoppen“** stoppt alle.

„Alles zurücksetzen“ entfernt die FX ebenfalls aus dem Programmer. „Alles stoppen“ in der oberen Leiste entfernt sie nicht: Sie sind Live-Änderungen.

### Eigene FX einzelner Fixtures

Ein FX bleibt **ein einziger**, auch wenn Sie einen Teil seiner Fixtures ändern. Wählen Sie nur einige der Fixtures eines FX aus und ändern Sie **Form, Größe, Richtung („Rückwärts“), Invertierungen, Drehung oder Phase**, ist die Änderung **eigen für diese Fixtures**, auf derselben Uhr: Sie bleiben mit den anderen synchron. **Tempo und Timing** gelten immer für den ganzen FX.

- Der Bereich weist darauf hin: „Dieser FX läuft auch auf anderen Fixtures…“.
- Sind alle seine Fixtures ausgewählt, gilt die Änderung für den ganzen FX.
- Wählen Sie Fixtures aus, die nicht im FX waren, schließen sie sich ihm an. Zwei FX desselben Parameters, die zusammen ausgewählt sind, verschmelzen zu einem, und jedes Fixture behält, was es tat.
- Unter „Laufende FX“ sehen Sie, wie viele etwas Eigenes haben („N eigene“).
- **„Wie der Rest“** entfernt das Eigene der ausgewählten Fixtures: Sie tun wieder dasselbe wie der FX.

### Der Spiegel

Liegen die Fixtures, die rückwärts laufen (oder mit eigenem invertiertem Pan oder Tilt), **alle in einer Hälfte** des FX, folgt jedes der Welle seines Gegenstücks auf der anderen Seite, sodass auch die Welle gespiegelt läuft. Der Bereich zeigt das mit „Spiegeln: …“ an.

Beispiel mit 6 Beams im Kreis mit Welle:

1. Wählen Sie alle 6 aus, wählen Sie **Pan / Tilt**, drücken Sie **„FX starten“**, Figur **Kreis** und **„Welle pro X“** auf „Alle“.
2. Wählen Sie nur die letzten 3 aus und aktivieren Sie **„Rückwärts“**.

Die letzten 3 drehen rückwärts und folgen der Welle des 3., des 2. und des 1.: ein synchroner Spiegel. Setzen Sie abwechselnde (2, 4 und 6) auf rückwärts, ist das kein Spiegel: Jedes behält seinen Platz in der Welle.

### FX-Makros

Rechts die mitgelieferten **FX-Makros** nach Kategorien: **Position** (Kreis, Kreiswelle, Acht, Ballyhoo, Pan-Schwenk, Cancan, Pan gespiegelt, Quadrat, Dreieck, Kleeblatt), **Intensität** (Dimmer-Welle, Lauflicht, Zufällig, Puls, Atmen, Stroboskop, Von außen nach innen), **Farbe** (Regenbogen, Rot / Blau, Feuer, Cyan / Magenta) und **Beam** (Zoom, Iris, Fokus). Ein Klick wendet das Makro auf die Auswahl an.

**„Als Makro speichern…“** speichert die FX des Programmers als Ihr eigenes Makro, unter **„Meine“**, für alle Shows. Rechtsklick auf eines Ihrer Makros: **„Umbenennen“** oder **„Löschen“**.

## Befehlszeile

Die Befehlszeile funktioniert wie die eines Lichtpults. Beim Öffnen von Programmieren empfängt sie die Tastatur: Tippen Sie und drücken Sie **Eingabe**. Daneben sehen Sie, was sie getan hat, oder warum sie den Befehl nicht versteht.

| Befehl | Was er tut |
|---|---|
| `1 THRU 8` | Wählt die Fixtures 1 bis 8 aus (auch `1 > 8` oder `1 / 8`). `8 THRU 1` in umgekehrter Reihenfolge. |
| `1 + 3 - 2` | Fügt hinzu und entfernt. `1,3,5` ist dasselbe wie `1 + 3 + 5`. Beginnt der Befehl mit `+ 5` oder `- 5`, ändert er die bestehende Auswahl. |
| `GROUP 2` | Die Fixture-Gruppe 2 (in der Reihenfolge des Patch). `GROUP 1 THRU 3`: die Gruppen 1 bis 3. |
| `ALL` | Alle Fixtures, nach Nummer geordnet (nach NEXT / PREV zurück zur ganzen Auswahl). |
| `@ 50` | Dimmer auf 50 %. `@ 5` ist 50 % (eine einzelne Ziffer steht für Zehner). |
| `@ FULL` / `@ OUT` | Dimmer voll oder auf null. `@ @` ist ebenfalls voll. |
| `@ 0 THRU 100` | Dimmer-Fächer, von 0 % beim ersten bis 100 % beim letzten. |
| `@ + 10` / `@ - 10` | Erhöht oder senkt den Dimmer um 10 %. |
| `POS 1`, `COL 2`, `BEAM 3`, `INT 4` | Wendet die Positions-, Farb-, Beam- oder Intensitätspalette mit dieser Nummer an. |
| `LOCATE` | Lokalisieren. |
| `HIGHLIGHT` | Hervorheben (umschalten). |
| `NEXT` / `PREV` | Einzeln durch die Auswahl. |
| `REL` | Gibt die Änderungen der Auswahl frei. |
| `CLEAR` | Leert den Programmer. |

Befehle lassen sich verketten: `1 THRU 4 @ FULL COL 2` wählt aus, setzt den Dimmer auf voll und wendet die Farbpalette 2 an.

Die Wörter funktionieren auch auf Spanisch: HASTA, GRUPO, TODO, LLENO, TOPE, CERO, INTENSIDAD, POSICION, COLOR, HAZ, LIMPIAR, RESALTAR, LOCALIZAR, SIGUIENTE, ANTERIOR, LIBERAR, IR, ATRÁS, GRABAR, ACTUALIZAR, ELIMINAR… und abgekürzt: THR, FL, GRP (oder G), CLR, LOC, HI, SIG, ANT, REL.

### Cuelist-Befehle

Sie wirken auf die **ausgewählte Cuelist**: die des ausgewählten Playbacks, die Sie unter Szenen geöffnet haben, oder die zuletzt erstellte.

| Befehl | Was er tut |
|---|---|
| `GO` | Der nächste Cue. |
| `BACK` | Der vorherige Cue. |
| `GO CUE 3` | Springt zu Cue 3. |
| `CUE 3` | Lädt Cue 3 in den Programmer (was die Bühne bei diesem Cue zeigt), um ihn zu ändern. |
| `REC` | Nimmt den nächsten Cue auf. `REC CUE 2.5` nimmt Cue 2.5 auf (zwischen 2 und 3). |
| `UPD` | Aktualisiert den in den Programmer geladenen Cue oder, wenn keiner geladen ist, den laufenden. `UPD CUE 4`: Cue 4. |
| `DELETE CUE 4` | Löscht Cue 4 und behält den Look der folgenden Cues bei. |
| `PB 3` | Wählt Playback 3 der aktuellen Seite aus (und seine Cuelist). |

Bricht ein Befehl auf halbem Weg ab (zum Beispiel weil es die Palette nicht gibt), bleibt das bereits Ausgeführte bestehen, und in der Zeile bleibt nur der Teil stehen, der noch zu korrigieren ist.

### Verlauf und Bildschirmtastatur

- **↑ ↓** blättern durch die letzten Befehle.
- **Esc** leert die Zeile (bei leerer Zeile ist Esc „Alles stoppen“).
- Die Taste **Tastenfeld** öffnet eine Bildschirmtastatur für Touchscreens, mit Zahlen, THRU, @, FULL, OUT, GROUP, INT, POS, COL, BEAM, NEXT, PREV, ALL, LOCATE, CLR und den Cuelist-Tasten (CUE, Dezimalpunkt, REC, UPD, GO, BACK, DELETE, PB). Sie bleibt offen, während Sie Fixtures wählen.

## Aufnehmen

### Das Fenster „Aufnehmen“

Drücken Sie **„Aufnehmen…“**:

1. Wählen Sie unter **Aufnehmen in**:
   - **„Neue Szene“**: eine statische Szene oder, wenn der Programmer FX enthält, eine FX-Szene mit ihren Grundwerten und ihren FX.
   - **„Neue Cuelist“**: Der Programmer wird zu Cue 1 einer neuen Cuelist. Ist **„Auf das erste freie Playback legen“** angehakt, erscheint sie auch unter Playbacks.
   - **Eine bestehende Cuelist**: Der Programmer wird als Cue dieser Cuelist aufgenommen. Die ausgewählte Cuelist ist standardmäßig gewählt.
2. Geben Sie unter **Name** einen Namen ein (und bei einem Cue die Nummer unter **Cue**: leer ist die nächste; `2.5` liegt zwischen 2 und 3).
3. Für eine neue Szene oder Cuelist: **„Zur Gruppe hinzufügen“** (oder „Keine Gruppe“) und **„Priorität (0-10)“**.
4. Für einen Cue: **„Nur dieser Cue (was er ändert, geht nicht in die folgenden Cues über)“** und **„Alle Werte (Block-Cue)“** (auch die Werte, die den übernommenen gleichen, damit er gleich aussieht, egal was die Cues davor tun).
5. Drücken Sie **„Aufnehmen“** (oder „Cue N aufnehmen“).

Ein Cue speichert nur, was der Programmer gegenüber dem ändert, was die vorherigen Cues auf der Bühne hinterlassen. Gibt es die Cue-Nummer schon, wird gefragt, was geschehen soll: **„Zusammenführen“** (der Programmer geht in den Cue ein), **„Ersetzen“** (der Cue enthält nur den Inhalt des Programmers; Name und Zeiten bleiben) oder **„Danach einfügen“** (ein neuer Cue zwischen diesem und dem nächsten).

**„Programmer nach dem Aufnehmen leeren“** leert den Programmer nach jeder Aufnahme und Aktualisierung. Die Option wird gespeichert.

### Snapshot

Das Menü **„Snapshot“** enthält:

- **„Änderungen in einer Szene speichern“**: übernimmt die Werte des Programmers in eine bestehende statische Szene.
- **„Alles auf der Bühne in eine neue Szene…“**: Alles, was die Scheinwerfer jetzt zeigen (Szenen, Live-Änderungen und der Mischer, ohne Grand Master), wird zu einer neuen statischen Szene, auf jedem Fixture mit Licht. Ist „Hervorheben“ aktiv, müssen Sie es vorher ausschalten („Hervorheben aus“).
- **„Nur die Änderungen in eine neue Szene…“**.

### Szenen und Cues bearbeiten

- **Eine statische oder FX-Szene**: Unter Live oder Szenen lädt **„Im Programmer bearbeiten“** sie in den Programmer. Ändern Sie, was Sie möchten, und drücken Sie unter [Szenen](escenas.md) **„Programmer in dieser Szene speichern“**.
- **Ein Cue**: Wenn Sie ihn laden (Doppelklick auf den Cue unter Szenen, „Laden“ oder `CUE 3`), erscheint eine Leiste „Cue 3 von ‚Song 1‘ wird bearbeitet“. Ändern Sie die Werte und drücken Sie **„Aktualisieren ▾“**: **„Cue 3 aktualisieren (auch für die folgenden Cues)“** oder **„Nur dieser Cue (die folgenden Cues behalten ihren Look)“**. Das **×** beendet die Bearbeitung des Cues (der Programmer behält seine Werte).

## Den Programmer leeren

- **„Alles zurücksetzen“** (oder `CLEAR`): gibt den ganzen Programmer frei, Werte und FX.
- **„Auswahl zurücksetzen“** (oder `REL`): nur die ausgewählten Fixtures.
- Das **×** einer Karte oder eines Faders: nur diesen Kanal.

Was mit den Live-Änderungen geschieht, wenn eine Szene gestartet wird, und welche Priorität sie gegenüber den Szenen haben, legen Sie unter [Optionen](opciones.md#wiedergabe) fest.

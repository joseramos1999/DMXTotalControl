# Patch

![Die Seite Patch](../img/patch.png)

**Patch** (F5) enthält die Fixtures Ihres Rigs: was sie sind, in welchem Universum und auf welcher Adresse sie liegen, ihre Nummer, die Fixture-Gruppen und wo sie auf der Bühne stehen.

Im Kopf der Seite wählen Sie die Ansicht und finden die Werkzeuge zum Hinzufügen:

- **„Liste“**: alle Fixtures nach Adresse.
- **„Kanäle“**: die 512 Kanäle jedes Universums, mit den Fixtures darüber.
- **„2D-Plan“**: die Bühne von oben gesehen.
- **„Importieren“**, **„LED-Matrix“** und **„Fixtures hinzufügen“**.

## Fixtures hinzufügen

1. Drücken Sie **„Fixtures hinzufügen“**.
2. Wählen Sie den **Hersteller** (tippen Sie in „Hersteller suchen…“, um zu filtern) und das **Modell**.
3. Im Bereich **Patch**:
   - **Name**: der Name, den Sie im ganzen Programm sehen (standardmäßig der des Modells).
   - **Modus**: der DMX-Modus, mit seiner Kanalzahl.
   - **Anzahl**: wie viele gleiche.
   - **Universum** und **Adresse**: Die Adresse wird automatisch auf die erste freie im Universum gesetzt.
   - **Abstand**: freie Kanäle zwischen einem Fixture und dem nächsten.
   - Bei mehr als einem: **„Gruppe daraus erstellen“** und der Name der **Gruppe**.
4. Unten sehen Sie, welche Adressen belegt werden („Belegt die Adressen 1 bis 20“). Passen sie nicht in den Rest des Universums, gehen sie ins nächste.
5. Drücken Sie **„Fixture patchen“** oder **„N Fixtures patchen“**.

Das Fenster bleibt offen, damit Sie weitere hinzufügen können, mit der Adresse schon auf der nächsten freien. Lässt sich etwas nicht patchen (zum Beispiel weil die Adressen belegt sind), erscheint der Grund unten in Rot. Drücken Sie am Ende **„Schließen“**.

Für einzelne Dimmer wählen Sie **Generic › Generic Dimmer**: Statt eines Modus geben Sie an, wie viele **Kanäle** Sie brauchen.

> **Tipp:** Finden Sie Ihr Fixture nicht, laden Sie die [Open Fixture Library](fixtures.md#open-fixture-library) herunter, importieren Sie seine GDTF-Datei oder erstellen Sie es auf der Seite [Fixtures](fixtures.md).

## Die Liste

Spalten: Auswahlkästchen, **#** (Nummer), **NAME**, **ADRESSE** (Universum . Adresse), **KANÄLE**, **FIXTURE** (Hersteller und Modell), **MODUS**, **INVERT.** (Pan und Tilt) und die Taste zum Identifizieren.

- **„Nach Fixture gruppieren“** gruppiert sie nach Modell, mit einem Kästchen, um alle eines Modells auszuwählen. Ein Klick auf die Kopfzeile eines Modells klappt es ein (blendet seine Fixtures aus) oder wieder auf.
- Ein **Doppelklick** auf ein Fixture bearbeitet es.

## Auswählen

Die Werkzeuge der Leiste arbeiten mit der Auswahl:

- **Klick** wählt aus; **Strg+Klick** fügt hinzu oder entfernt; **Umschalt+Klick** wählt alle Fixtures vom zuletzt angeklickten bis zu diesem aus (in dieser Richtung, die die Reihenfolge der Auswahl ist), und **Strg+Umschalt+Klick** fügt sie hinzu. **„Alle“** und **„Keine“**.
- **„Identifizieren“**: Die ausgewählten Fixtures blinken weiß, damit Sie sie auf der Bühne finden. **„Identifizieren beenden“** stoppt das (es stoppt auch, wenn Sie die Seite verlassen). Die Taste jeder Zeile identifiziert nur dieses Fixture. Die Liste bleibt, wo sie war.
- **Pan invertieren** und **Tilt invertieren** (die Pfeilsymbole): für die Fixtures, die diese Achsen haben.
- **„Grenzen“**, **„Bearbeiten“**, **„Neu adressieren“**, **„Nummerieren“**, **„Gruppe erstellen“** und **„Löschen“**: werden im Folgenden erklärt.

## Adressen und Universen

- **Ein Fixture bearbeiten**: Wählen Sie eines aus und drücken Sie **„Bearbeiten“** (oder Doppelklick). Sie können **Name**, **Modus**, **Universum**, **Adresse** und **Nummer** ändern und **„Anwenden“** drücken.
- **Mehrere neu adressieren**: Wählen Sie sie aus und drücken Sie **„Neu adressieren“**. Wählen Sie **Universum**, **Startadresse** und **Abstand**: Sie bekommen fortlaufende Adressen, in der Reihenfolge der Liste.
- **In der Ansicht Kanäle ziehen**: Wählen Sie oben das Universum und ziehen Sie ein Fixture auf eine andere Adresse. Der Schatten ist grün, wenn es passt, und rot, wenn die Kanäle nicht frei sind. Oben sehen Sie, wie viele der 512 Kanäle belegt sind. Ein Doppelklick auf ein Fixture bearbeitet es.

Universen werden unter [Ausgänge](salidas.md) erstellt und mit den Interfaces verbunden.

## Fixture-Nummern

Jedes Fixture hat eine **Nummer**; die [Befehlszeile](programar.md#befehlszeile) unter Programmieren verwendet sie (`1 THRU 8 @ FULL`). Ein neues Fixture bekommt die Nummer nach der letzten.

- Ändern Sie eine unter **„Bearbeiten“** › **Nummer**.
- Um mehrere fortlaufend zu nummerieren, wählen Sie sie aus, drücken Sie **„Nummerieren“** und geben Sie die **Erste Nummer** ein: Sie werden in Adressreihenfolge nummeriert.

## Fixture-Gruppen

Mit Fixture-Gruppen wählen Sie im Patch und unter Programmieren mit einem Klick aus (`GROUP 2` in der Befehlszeile), und jede hat ihren Kanalzug im [Mischer](directo.md#mischer). Sie werden auch in der SHOW verwendet (Farbrad, XY-Feld, Paletten, Dimmer und Bewegungstempo) und in den Zuordnungen.

1. Wählen Sie die Fixtures aus.
2. Drücken Sie **„Gruppe erstellen“** und geben Sie ihren Namen ein (zum Beispiel `Front-Wash` oder `Spots`).

Der Bereich **„Fixture-Gruppen“** links zeigt sie mit ihrer Fixture-Anzahl. Ein Klick auf eine Gruppe wählt ihre Fixtures aus; **Strg+Klick** oder **Umschalt+Klick** fügt sie hinzu. Rechtsklick:

- **„Umbenennen“**.
- **„Durch Auswahl ersetzen“**: Die Gruppe enthält danach die ausgewählten Fixtures.
- **„Auswahl hier speichern…“**: siehe [Auswahlen](patch.md#auswahlen).
- Auswahlmuster, in der Reihenfolge der Gruppe: **„Ungerade auswählen“**, **„Gerade auswählen“**, **„In umgekehrter Reihenfolge auswählen“**, **„Erste Hälfte auswählen“**, **„Zweite Hälfte auswählen“**, **„Von außen zur Mitte auswählen“** und **„Von der Mitte nach außen auswählen“**.
- **„Gruppe löschen“**.

### Auswahlen

Eine **Auswahl** ist ein geordneter Teil einer Gruppe: zum Beispiel die Spots einer Sechsergruppe, von außen zur Mitte genommen. Die Reihenfolge zählt, denn Fächer und FX folgen ihr.

1. Wählen Sie die Fixtures in der gewünschten Reihenfolge aus (oder mit einem Muster aus dem Menü der Gruppe).
2. Drücken Sie das **+** der Gruppe (oder „Auswahl hier speichern…“) und geben Sie einen Namen ein.

Auswahlen erscheinen unter ihrer Gruppe. Ein Klick wählt ihre Fixtures in ihrer Reihenfolge aus, hier und unter Programmieren. Rechtsklick: **„Umbenennen“** und **„Auswahl löschen“**. Der Pfeil links neben der Gruppe blendet ihre Auswahlen aus oder ein.

## 2D-Plan

Der Plan zeigt die Bühne von oben, mit **PUBLIKUM** unten. Er dient zum Auswählen unter Programmieren, zum Ansehen der Ausgabe und für Farbeffekte, die auf dem Plan platziert sind.

Im Kopf des Plans:

- **„Breite (m)“** und **„Tiefe (m)“**: die Größe der Bühne.
- **„Automatisch anordnen“**: verteilt alle Fixtures.

Die Werkzeuge (sie arbeiten mit der Auswahl und in ihrer Reihenfolge):

- **„Live“**: zeigt Farbe, Dimmer, Stroboskop und Gobo jedes Fixtures sowie jeden Pixel der Bars und Matrizen.
- **„Lasso“**: wählt aus, indem Sie um die Fixtures herum zeichnen, statt mit einem Rechteck.
- **„Auswählen“**: Klick oder Rechteck zum Auswählen.
- **„Verschieben“**: zieht die Fixtures (die ganze Auswahl bewegt sich) oder die Ansicht.
- **„Raster“**: „Frei“, 10 cm, 25 cm, 50 cm oder 1 m: wo die Fixtures beim Verschieben landen.
- **„Aussehen“**: wie die ausgewählten Fixtures dargestellt werden: „Rund“, „Quadrat“ oder „Rechteck“.
- **„Anordnen“**: ordnet sie als „Linie“, „Linie, von vorne nach hinten“, „Kreis“, „Raster, nach Zeilen“, „Raster, Zickzack“, „Raster, nach Spalten“ oder „Raster, Zickzack nach Spalten“ an, mit dem **Abstand (cm)** und den **Spalten** daneben.
- **„Ausrichten“**: „Links“, „Mitte“, „Rechts“, „Hinten“, „Mitte“, „Vorne“, „In der Breite verteilen“ und „In der Tiefe verteilen“.
- **15°** (Rechtsklick: andersherum) und **90°**: drehen die Auswahl.
- **„Höhe und Ausrichtung“**: wo die ausgewählten Moving Heads hängen, für [Follow](patch.md#höhe-und-ausrichtung-follow).

Moving Heads haben einen kleinen Pfeil am Rand: Er zeigt, wohin sie schauen. Die Namen sehr dicht beieinanderliegender Fixtures werden ausgeblendet, damit sie sich nicht überlappen: Zoomen Sie hinein, um sie zu lesen.

Das **Mausrad** zoomt, die **rechte** oder die mittlere Maustaste verschieben die Ansicht, und ein Doppelklick mit ihnen zentriert sie. Die Tasten in der Ecke zoomen ebenfalls.

> **Tipp:** Ordnen Sie die Fixtures in der Reihenfolge an, in der die Effekte laufen sollen: Wählen Sie sie von links nach rechts aus und verwenden Sie „Anordnen › Linie“.

### Höhe und Ausrichtung (Follow)

Damit [Follow](programar.md#follow-auf-einen-punkt-zielen) alle Moving Heads richtig auf einen Punkt richtet, muss jeder wissen, wo er sich befindet. Platzieren Sie die Köpfe im Plan dort, wo sie wirklich sind, wählen Sie sie aus und drücken Sie **„Höhe und Ausrichtung“**. Was Sie ändern, gilt sofort für alle ausgewählten und wird mit der Show gespeichert:

| Feld | Was es ist |
|---|---|
| **Höhe** | Wie viele Meter über dem Bühnenboden die Achse des Kopfes liegt (ohne Einstellung: 4 m hängend, 0 m stehend). |
| **Montage** | **„Hängend“** (an einer Stange oder Traverse, kopfüber) oder **„Stehend“** (auf dem Boden). |
| **Ausrichtung** | Wohin der Strahl geht, mit Pan in der Mitte und nach vorn geneigtem Tilt. 0° zeigt zum Publikum und dreht im Uhrzeigersinn; es ist dieselbe Drehung wie bei den Tasten 15° und 90°, und der Pfeil im Plan zeigt sie. |
| **Pan-Bereich** / **Tilt-Bereich** | Die Grade, die jede Achse überstreicht. „Aus der Definition“ verwendet die Werte aus der Fixture-Definition; fehlen sie dort, werden 540° und 270° verwendet, und es erscheint ein Hinweis in Orange. |
| **Pan umgekehrt** / **Tilt umgekehrt** | Wenn sich ein Kopf mit Follow zur falschen Seite dreht oder neigt. Betrifft nur Follow; die Invertierungen des Patch finden Sie unter [Grenzen](patch.md#grenzen). |

„(unterschiedlich)“ bedeutet, dass die ausgewählten Köpfe nicht denselben Wert haben; ändern Sie ihn, übernehmen alle den neuen.

## Grenzen

Wählen Sie Fixtures aus und drücken Sie **„Grenzen“**:

- **Dimmer**, **Pan** und **Tilt**: der erlaubte Bereich, von 0 bis 100 %. Zum Beispiel ein Dimmer, der nicht über 80 % geht, oder ein Tilt, der nicht ins Publikum zielt.
- **„Pan invertieren“**, **„Tilt invertieren“** und **„Pan und Tilt tauschen“** (für kopfüber hängende oder gedrehte Köpfe).
- **„Zurücksetzen“**: keine Grenzen, keine invertierten oder vertauschten Achsen.

Bei einem einzelnen Fixture legen Sie in der Liste **Kanäle** für jeden Kanal fest:

- **Überblendungen**: ob er den Überblendzeiten folgt. Aus: Er springt direkt auf seinen Wert (nützlich bei Farb- und Goborädern).
- **Dimmer**: ob ihn der Grand Master, die Gruppen-Master und die Szenen-Dimmer skalieren.

Die Grenzen wirken auf die Ausgabe: Szenen, Effekte und Live-Änderungen behalten ihre Werte, aber die Ausgabe hält sich an die Grenzen. Bei mehreren Fixtures sehen Sie die Werte des ersten Fixtures, das den jeweiligen Regler hat, und was Sie ändern, gilt für alle, die ihn haben.

## LED-Matrizen und -Streifen

**„LED-Matrix“** patcht eine Matrix oder einen Pixelstreifen auf einmal:

1. **Name**, **Breite (Pixel)** und **Höhe (Pixel)** (Höhe 1 für einen Streifen).
2. **Farben**: die Reihenfolge in jedem Pixel (RGB, GRB, BGR, BRG, GBR, RBG oder RGBW).
3. **Verkabelung**: „Zeilen“, „Zeilen, Zickzack“, „Spalten“ oder „Spalten, Zickzack“.
4. **Pixel 1**: die Ecke, in der es beginnt („Oben links“ usw.).
5. **Universum** und **Adresse**.
6. Prüfen Sie die Zusammenfassung und die Vorschau der **Pixelreihenfolge** und drücken Sie **„Patchen“**.

Es entstehen ein Fixture pro Pixelzeile, eine Gruppe für die Farbeffekte und ihr Platz im Plan. Braucht eine Zeile mehr als ein Universum, machen Sie sie kürzer oder verkabeln Sie andersherum.

## Aus einer anderen Show importieren

**„Importieren“** holt einen Teil einer anderen Show (von DMX Total Control oder QLC+) in diese:

- **„Patch einer anderen Show…“**: ihre Fixtures. Sie kommen auf dieselben Adressen, wenn diese frei sind; sonst auf die nächsten.
- **„Szenen einer anderen Show…“**: ihre Szenen, zugeordnet zu den passenden Fixtures dieser Show (nach Adresse und Modell, nach Name oder nach Modell). Sie kommen in Gruppen mit demselben Namen.
- **„Patch und Szenen einer anderen Show…“**: beides.

## Fixtures löschen

Wählen Sie sie aus und drücken Sie **„Löschen“**. Vorher wird nachgefragt: Ihre Werte werden aus allen Szenen entfernt.

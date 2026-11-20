# Patch

![Die Seite Patch](../img/patch.png)

**Patch** (F5) enthält die Fixtures Ihres Rigs: was sie sind, in welchem Universum und auf welcher Adresse sie liegen, ihre Nummer, die Fixture-Gruppen und wo sie auf der Bühne stehen.

Im Kopf der Seite wählen Sie die Ansicht und finden die Werkzeuge zum Hinzufügen:

- **„Liste“**: alle Fixtures nach Adresse.
- **„Kanäle“**: die 512 Kanäle jedes Universums, mit den Fixtures darüber.
- **„2D-Plan“**: die Bühne von oben gesehen.
- **„Importieren“**, **„LED-Matrix“** und **„Fixtures hinzufügen“**.
- Mit geparkten Fixtures **„N geparkt · Alle entparken“**: wie viele es sind und die Taste, um alle zu entparken (siehe [Kanäle parken](patch.md#kanäle-parken)).

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
4. Unten sehen Sie, welche Adressen belegt werden („Belegt die Adressen 1 bis 20“). Passen sie nicht in den Rest des Universums, gehen sie ins nächste. Hat der Modus weitere *DMX-Breaks* (ein GDTF-Fixture mit zwei DMX-Eingängen), bittet ein Hinweis darum, auch seinen Modus „(DMX break 2)“ an einer eigenen Adresse zu patchen.
5. Drücken Sie **„Fixture patchen“** oder **„N Fixtures patchen“**.

Das Fenster bleibt offen, damit Sie weitere hinzufügen können, mit der Adresse schon auf der nächsten freien. Lässt sich etwas nicht patchen (zum Beispiel weil die Adressen belegt sind), erscheint der Grund unten in Rot. Drücken Sie am Ende **„Schließen“**.

Für einzelne Dimmer wählen Sie **Generic › Generic Dimmer**: Statt eines Modus geben Sie an, wie viele **Kanäle** Sie brauchen.

**„Auf GDTF-Share suchen…“** unten öffnet [GDTF-Share](fixtures.md#gdtf-share): Laden Sie das Fixture herunter und drücken Sie **„Patchen“**, damit es hier ausgewählt ist.

> **Tipp:** Finden Sie Ihr Fixture nicht, suchen Sie es auf GDTF-Share, laden Sie die [Open Fixture Library](fixtures.md#open-fixture-library) herunter, importieren Sie seine GDTF-Datei oder erstellen Sie es auf der Seite [Fixtures](fixtures.md).

## Die Liste

Spalten: Auswahlkästchen, **#** (Nummer), **NAME**, **ADRESSE** (Universum . Adresse), **KANÄLE**, **FIXTURE** (Hersteller und Modell), **MODUS**, **INVERT.** (Pan und Tilt) und die Taste zum Identifizieren. Nach dem Namen kennzeichnet ein **Schild** einen [geschützten Spezialeffekt](patch.md#geschützte-spezialeffekte) und ein **Schloss** ein Fixture mit [geparkten](patch.md#kanäle-parken) Kanälen.

- **„Nach Fixture gruppieren“** gruppiert sie nach Modell, mit einem Kästchen, um alle eines Modells auszuwählen. Ein Klick auf die Kopfzeile eines Modells klappt es ein (blendet seine Fixtures aus) oder wieder auf.
- Ein **Doppelklick** auf ein Fixture bearbeitet es.

## Auswählen

Die Werkzeuge der Leiste arbeiten mit der Auswahl:

- **Klick** wählt aus; **Strg+Klick** fügt hinzu oder entfernt; **Umschalt+Klick** wählt alle Fixtures vom zuletzt angeklickten bis zu diesem aus (in dieser Richtung, die die Reihenfolge der Auswahl ist), und **Strg+Umschalt+Klick** fügt sie hinzu. **„Alle“** und **„Keine“**. Das Kästchen einer Zeile fügt sein Fixture hinzu oder entfernt es (wie Strg+Klick); mit Umschalt fügt es alle ab dem zuletzt angeklickten hinzu.
- **„Identifizieren“**: Die ausgewählten Fixtures blinken weiß, damit Sie sie auf der Bühne finden. **„Identifizieren beenden“** stoppt das (es stoppt auch, wenn Sie die Seite verlassen). Die Taste jeder Zeile identifiziert nur dieses Fixture. Die Liste bleibt, wo sie war. Geschützte Spezialeffekte werden nie identifiziert (ein blinkendes CO2 würde auslösen): Eine Meldung sagt, wie viele ausgelassen wurden.
- **Pan invertieren** und **Tilt invertieren** (die Pfeilsymbole): für die Fixtures, die diese Achsen haben.
- **„Grenzen“**, die Taste mit der **Flamme** (**„Spezialeffekt“**), **„Bearbeiten“**, **„Neu adressieren“**, **„Nummerieren“**, **„Gruppe erstellen“** und **„Löschen“**: werden im Folgenden erklärt.

Ein **Rechtsklick** auf eine Zeile der Liste oder auf ein Fixture im 2D-Plan, ohne die Maus zu bewegen, öffnet das Fixture-Menü; war das Fixture nicht ausgewählt, wird es zuerst ausgewählt. Oben steht, worauf es wirkt: das Fixture (seine Nummer, sein Name, seine Adresse und sein Typ) oder wie viele ausgewählt sind und welche. Darunter, mit denselben Symbolen wie in der Leiste: **„Identifizieren“** (solange Fixtures blinken, **„Identifizieren beenden“**, mit ihrer Anzahl); unter **Einstellungen** **„Grenzen…“**, **„Spezialeffekt…“** (mit dem Schild der Liste, wenn geschützte Effekte dabei sind), **„Bearbeiten…“** (mit einem einzigen Fixture; bei mehreren steht „Nur einzeln“ daneben) und der Schalter **„Vor dem Grand Master schützen“**; unter **Parken**, das rechts anzeigt, wie viele der Auswahl geparkt sind, **„So parken, wie es jetzt ist“**, **„Ausgeschaltet parken“** und, wenn ein Fixture der Auswahl geparkt ist, der Schalter **„Auch im Blackout“** und **„Entparken“**; und unter **Ganze Show**, wenn Fixtures geparkt sind, **„Alle entparken“**, das alle der Show entparkt, nicht nur die ausgewählten. Ein halb gestellter Schalter mit der Anzahl (3/4) bedeutet, dass nur einige der ausgewählten ihn haben: Ein Klick schaltet ihn für alle ein. Bleibt die Maus auf den meisten Einträgen stehen, sagt eine Notiz neben dem Menü, was sie tun.

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

Der Bereich **„Fixture-Gruppen“** links zeigt sie mit ihrer Fixture-Anzahl. Ein Klick auf eine Gruppe wählt ihre Fixtures aus (in der Ansicht „2D-Plan“ isoliert er sie: siehe [Eine Gruppe im Plan isolieren](patch.md#eine-gruppe-im-plan-isolieren)); **Strg+Klick** oder **Umschalt+Klick** fügt sie hinzu. Rechtsklick:

- **„Umbenennen“**.
- **„Nach oben“** und **„Nach unten“**: verschieben sie in der Liste.
- **„Durch Auswahl ersetzen“**: Die Gruppe enthält danach die ausgewählten Fixtures.
- **„Auswahl hier speichern…“**: siehe [Auswahlen](patch.md#auswahlen).
- Auswahlmuster, in der Reihenfolge der Gruppe: **„Ungerade auswählen“**, **„Gerade auswählen“**, **„In umgekehrter Reihenfolge auswählen“**, **„Erste Hälfte auswählen“**, **„Zweite Hälfte auswählen“**, **„Von außen zur Mitte auswählen“** und **„Von der Mitte nach außen auswählen“**.
- **„Gruppe löschen“**.

**Die Reihenfolge der Gruppen** bestimmen Sie: Ziehen Sie eine Gruppe an ihrem Griff (links neben ihrem Namen) an eine andere Stelle der Liste, oder verwenden Sie „Nach oben“ und „Nach unten“. **„A–Z“** im Kopf des Bereichs ordnet sie alphabetisch (Zahlen als Zahlen: „Truss 2“ vor „Truss 10“). Neue Gruppen kommen ans Ende. Die Reihenfolge wird mit der Show gespeichert und gilt ebenso in Programmieren und für die `GROUP`-Nummern der Befehlszeile.

### Auswahlen

Eine **Auswahl** ist ein geordneter Teil einer Gruppe: zum Beispiel die Spots einer Sechsergruppe, von außen zur Mitte genommen. Die Reihenfolge zählt, denn Fächer und FX folgen ihr.

1. Wählen Sie die Fixtures in der gewünschten Reihenfolge aus (oder mit einem Muster aus dem Menü der Gruppe).
2. Drücken Sie das **+** der Gruppe (oder „Auswahl hier speichern…“) und geben Sie einen Namen ein.

Auswahlen erscheinen unter ihrer Gruppe. Ein Klick wählt ihre Fixtures in ihrer Reihenfolge aus, hier und unter Programmieren. Rechtsklick: **„Umbenennen“** und **„Auswahl löschen“**. Der Pfeil links neben der Gruppe blendet ihre Auswahlen aus oder ein.

## 2D-Plan

Der Plan zeigt die Bühne von oben, mit **PUBLIKUM** unten. Er dient zum Auswählen unter Programmieren, zum Ansehen der Ausgabe und für Farbeffekte, die auf dem Plan platziert sind.

Im Kopf des Plans:

- **„Breite (m)“** und **„Tiefe (m)“**: die Größe der Bühne, bis 200 m. Sie vergrößern oder verkleinern die Bühne rechts und vorne; ein Fixture, das außerhalb einer kleineren Bühne liegt, bleibt, wo es ist.
- **„Automatisch anordnen“**: verteilt alle Fixtures.

Außerhalb der Bühne, in der Mitte jeder Seite, gibt es ein **„−“** und ein **„+“**: Sie entfernen 1 m auf dieser Seite oder fügen 1 m hinzu (links, rechts, hinten oder vorne, auf der Publikumsseite); gedrückt gehalten mehrere Meter. Die Fixtures bleiben, wo sie auf der Bühne sind: links und hinten bewegen sie sich mit dieser Seite, ebenso der Punkt, auf den [Follow](programar.md#follow-auf-einen-punkt-zielen) zielt, und der neue Streifen erscheint auf dieser Seite. Solange die Maus darüber ist, ändert sich die Ansicht des Plans nicht und die Buttons bleiben stehen, sodass Sie mehrmals hintereinander klicken können; wenn Sie sie wegbewegen, passt sich der Plan wieder an (hatten Sie gezoomt oder die Ansicht verschoben, bleibt sie so). Das **„−“** einer Seite funktioniert nur, wenn kein Fixture 1 m oder näher daran steht: Verschieben Sie es zuerst (fahren Sie mit der Maus über den Button, um zu sehen, warum).

Die Werkzeuge (sie arbeiten mit der Auswahl und in ihrer Reihenfolge):

- **„Live“**: zeigt Farbe, Dimmer, Stroboskop und Gobo jedes Fixtures sowie jeden Pixel der Bars und Matrizen.
- **„Lasso“**: wählt aus, indem Sie um die Fixtures herum zeichnen, statt mit einem Rechteck.
- **„Auswählen“**: Klick oder Rechteck zum Auswählen.
- **„Verschieben“**: zieht die Fixtures (die ganze Auswahl bewegt sich) oder die Ansicht.
- **„Raster“**: „Frei“, 10 cm, 25 cm, 50 cm oder 1 m: wo die Fixtures beim Verschieben landen.
- **„Aussehen“**: wie die ausgewählten Fixtures dargestellt werden: „Rund“, „Quadrat“ oder „Rechteck“, und ihr **„Symbol“** (siehe [Symbole, Farben und Legende](patch.md#symbole-farben-und-legende)).
- **„Anordnen“**: ordnet sie als „Linie“, „Linie, von vorne nach hinten“, „Kreis“, „Raster, nach Zeilen“, „Raster, Zickzack“, „Raster, nach Spalten“ oder „Raster, Zickzack nach Spalten“ an, mit dem **Abstand (cm)** und den **Spalten** daneben.
- **„Ausrichten“**: „Links“, „Mitte“, „Rechts“, „Hinten“, „Mitte“, „Vorne“, „In der Breite verteilen“ und „In der Tiefe verteilen“.
- **15°** (Rechtsklick: andersherum) und **90°**: drehen die Auswahl.
- **„Höhe und Ausrichtung“**: wo die ausgewählten Moving Heads hängen, für [Follow](patch.md#höhe-und-ausrichtung-follow).

Moving Heads haben einen kleinen Pfeil am Rand (eine Kerbe, wenn der Plan sie klein zeigt): Er zeigt, wohin sie schauen. Die Namen sehr dicht beieinanderliegender Fixtures werden ausgeblendet, damit sie sich nicht überlappen: Zoomen Sie hinein, um sie zu lesen. Ein **Schild** oben links kennzeichnet einen geschützten Spezialeffekt (blass; rot bei scharf geschalteten Effekten; mit einem pulsierenden Ring, während er auslöst), ein **Schloss** oben rechts ein geparktes Fixture.

Das **Mausrad** zoomt, die **rechte** oder die mittlere Maustaste verschieben die Ansicht, und ein Doppelklick mit ihnen zentriert sie. Die Tasten in der Ecke zoomen ebenfalls. Ein Rechtsklick ohne Mausbewegung öffnet das [Fixture-Menü](patch.md#auswählen).

> **Tipp:** Ordnen Sie die Fixtures in der Reihenfolge an, in der die Effekte laufen sollen: Wählen Sie sie von links nach rechts aus und verwenden Sie „Anordnen › Linie“.

### Symbole, Farben und Legende

Jedes Fixture zeigt auf dem Plan das Symbol dessen, was es ist: Moving Head, Scanner, Wash oder PAR, LED-Leiste, Stroboskop, Blinder, Dimmer oder konventionell, Laser, Nebel, Hazer, CO2-Jet, Flamme, Kaltfunken, Konfetti, Seifenblasen, Schnee, Ventilator, Effekt oder andere. Das sagt sein Fixture-Typ, außer der Name seines Modells sagt mehr: Ein „Effect“ namens „CO2 Jet“ ist ein CO2-Jet und ein Dimmer namens „Blinder“ ein Blinder, aber ein Moving Head namens „Beam“ oder „Wash“ bleibt ein Moving Head. Ein generischer Dimmer, der für jedes Fixture stehen kann, bekommt das Symbol seines eigenen Namens („CO2 1“, „FLAME 3“, „BLINDER1“). LED-Leisten und Matrizen zeigen ihre Zellen; die anderen Fixtures mit mehreren Zellen (ein Blinder mit mehreren Lampen, ein Stroboskop mit mehreren Segmenten) haben das Symbol neben ihren Zellen.

Um es von Hand zu wählen, wählen Sie die Fixtures aus und verwenden Sie **„Aussehen“ › „Symbol“**; **„Automatisch“** kehrt zu seinem eigenen zurück. Das gewählte Symbol wird mit der Show gespeichert.

Jeder Fixture-Typ der Show hat außerdem seine Farbe, im Symbol und am Rand, bei jedem Öffnen der Show dieselbe: Zwei Typen mit demselben Symbol unterscheiden sich durch ihre Farbe. Ein von Hand gewähltes Symbol ändert die Farbe eines Fixtures aus der Bibliothek nicht. Live zeigen eingeschaltete Fixtures ihr Licht und ausgeschaltete ihr Symbol.

Die **Legende** oben links im Plan listet die Fixture-Typen mit ihrem Symbol, ihrer Farbe und ihrer Anzahl auf (die generischen Dimmer nach dem, wofür sie stehen). Ein Klick auf einen Typ wählt alle seine Fixtures aus, auch im Plan von Programmieren; **Strg+Klick** oder **Umschalt+Klick** fügt sie der Auswahl hinzu. Ein Klick auf ihren Titel klappt sie zu oder auf, und das wird gemerkt. Die Bühne wird neben oder unter die Legende gelegt, damit diese kein Fixture verdeckt.

### Eine Gruppe im Plan isolieren

In der Ansicht „2D-Plan“ **isoliert** ein Klick auf eine Gruppe im Bereich „Fixture-Gruppen“ diese Gruppe: Die anderen Fixtures sind im Plan gesperrt und blass, und nichts wählt sie aus oder verschiebt sie, weder ein Klick noch das Rechteck, das Lasso, die Legende oder „Verschieben“ (ein Klick auf eines bewirkt nichts, und ein Rechteck kann darauf beginnen). So lässt sich in einem vollen Plan leicht ein Teil einer Gruppe auswählen und daraus kleinere Gruppen bilden. Von der Auswahl bleiben nur die Fixtures der Gruppe, die schon ausgewählt waren. Die isolierte Gruppe hat einen dicken Rand und ein Auge, und neben der Legende des Plans sagt ein Hinweis, welche Gruppe Sie sehen und wie viele Fixtures sie hat.

- **„Alle“** wählt alle Fixtures der Gruppe aus, ebenso **Strg+Klick** auf die Gruppe. **Strg+Klick** auf eine andere Gruppe fügt nur ihre Fixtures hinzu, die auch in der isolierten sind.
- In der Legende wählt ein Klick auf einen Typ nur seine Fixtures in der Gruppe aus; Typen, die dort keines haben, erscheinen blass.
- Ein Klick auf eine andere Gruppe isoliert stattdessen diese; ein Auswahlmuster aus ihrem Menü isoliert zuerst diese Gruppe. Ein Klick auf eine Auswahl wählt immer alle ihre Fixtures aus: Sind einige gesperrt, isoliert er zuerst die Gruppe der Auswahl oder beendet, wenn sie Fixtures anderer Gruppen enthält, die Isolierung.
- Nach **„Gruppe erstellen“** bleibt die Gruppe isoliert: Sie können sie weiter in mehr Gruppen aufteilen.
- Zum Beenden: ein weiterer Klick auf die Gruppe, das **×** des Hinweises oder **Esc**. Es endet auch beim Wechsel zu „Liste“ oder „Kanäle“, beim Öffnen einer anderen Show oder beim Löschen der Gruppe. Es wird nicht mit der Show gespeichert.

In den Ansichten „Liste“ und „Kanäle“ wählt ein Klick auf eine Gruppe weiterhin ihre Fixtures aus, wie bisher.

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
- **„Pan invertieren“**, **„Tilt invertieren“** und **„Pan und Tilt tauschen“** (für kopfüber hängende oder gedrehte Köpfe). Das ist eine physische Korrektur: Sie lässt diese Köpfe sich wie die anderen bewegen, und der Spiegel eines FX berücksichtigt sie nicht (siehe [Der Spiegel](programar.md#der-spiegel)). Bei mehreren Fixtures, von denen nur einige invertiert sind, ist er aus und zeigt „(nur einige)“, wie die Taste der Leiste, und beim Einschalten werden alle invertiert.
- **„Zurücksetzen“**: keine Grenzen, keine invertierten oder vertauschten Achsen.
- **„Vor dem Grand Master schützen“**: Der Grand Master senkt keinen seiner Kanäle ab, ob er nur auf die Intensität oder auf alle wirkt: das Saallicht, ein Medienserver, ein Hazer. Die Gruppen-Master, die Szenen-Dimmer, Blackout und das [Ausblenden auf Schwarz](directo.md#ausblenden-auf-schwarz) senken es weiterhin ab. Bei mehreren Fixtures, von denen nur einige geschützt sind, ist der Schalter aus und zeigt „(nur einige)“; Einschalten schützt alle. Der Eintrag steht auch im [Fixture-Menü](patch.md#auswählen), unter **Einstellungen**, als Schalter: an, wenn alle ausgewählten geschützt sind, und halb, mit der Anzahl (3/4), wenn nur einige es sind; ein Klick auf den halben Schalter schützt alle. In der Liste ein **Anker** nach dem Namen („Vor dem Grand Master geschützt: geht nicht mit ihm herunter“); im 2D-Plan, im Patch und unter Programmieren, ein Anker unten links (bei einer Leiste rechts daneben). „Zurücksetzen“ entfernt den Schutz nicht. Er wird mit der Show gespeichert, in jedem Fixture, und [den Patch einer anderen Show importieren](patch.md#aus-einer-anderen-show-importieren) übernimmt ihn. QLC+ liest ihn nicht: Dort senkt der Grand Master das Fixture ab.

Bei einem einzelnen Fixture legen Sie in der Liste **Kanäle** für jeden Kanal fest:

- **Überblendungen**: ob er den Überblendzeiten folgt. Aus: Er springt direkt auf seinen Wert (nützlich bei Farb- und Goborädern).
- **Dimmer**: ob ihn der Grand Master, die Gruppen-Master und die Szenen-Dimmer skalieren. Der Grand Master skaliert außerdem nur das Licht (siehe [Master](directo.md#master)). Ist das Fixture vor dem Grand Master geschützt, skalieren ihn nur die Gruppen-Master und die Szenen-Dimmer.

Die Grenzen wirken auf die Ausgabe: Szenen, Effekte und Live-Änderungen behalten ihre Werte, aber die Ausgabe hält sich an die Grenzen. Bei einem Pan oder Tilt mit 16 Bit gilt die Grenze für beide Kanäle zusammen, eine langsame Bewegung bleibt also glatt. Bei mehreren Fixtures sehen Sie die Werte des ersten Fixtures, das den jeweiligen Regler hat, und was Sie ändern, gilt für alle, die ihn haben.

## Geschützte Spezialeffekte

Ein **geschützter Spezialeffekt** löst nur bei scharf geschalteten Effekten aus Szenen, Cuelists und Chasern aus, und jede Auslösung dauert höchstens ihre Höchstzeit. Wie man sie scharf schaltet und was sie auslöst, steht unter [Live](directo.md#geschützte-spezialeffekte).

**Welche es sind.** Maschinen für **CO2**, **Flammen**, **Kaltfunken** und **Konfetti** werden von selbst geschützt; die für **Seifenblasen** und **Schnee** nicht (schützen Sie sie von Hand, wenn Sie wollen). Die Art ergibt sich aus dem Fixture-Typ, aus den Wörtern seiner Definition, aus dem Namen eines generischen Dimmers („CO2 1“, „FLAME 3“, „FEUER 2“, „PYRO 1“) oder aus dem im Plan gewählten Symbol. Die automatischen Regeln schützen nur, sie heben den Schutz nie auf: Ein neuer Name oder ein neues Symbol hebt den Schutz eines Fixtures nie auf; ein Fixture, das zu einer dieser Arten wird, wird geschützt, und eine Meldung sagt es („CO2 4 ist jetzt ein geschützter Spezialeffekt“); ein CO2, dessen Schutz Sie von Hand aufgehoben haben, bleibt ungeschützt, auch wenn Sie es „CO2 9“ nennen.

> **Grenzen:** „C02“ mit einer Null wird nicht erkannt, und „FOCO2“ ist kein CO2. Eine nicht erkannte Maschine ist nicht geschützt: Prüfen Sie das Schild in der Liste und schützen Sie sie von Hand.

**„Spezialeffekt“** (die Taste mit der Flamme neben „Grenzen“ oder „Spezialeffekt…“ im Fixture-Menü) öffnet seine Einstellungen für die Auswahl:

- **„Geschützter Spezialeffekt“**: Das Ausschalten fragt vorher nach („Den Schutz entfernen?“). Ohne ihn geht das Fixture wie jeder andere Kanal hinaus: ohne Scharfschalten, ohne Höchstzeit und auch vom Programmer, von Identifizieren, vom Mischer und von den FX.
- **„Höchstzeit einer Auslösung“**: 0,1 bis 30 s (Standard: CO2 3 s, Flammen 2 s, Funken 5 s, Konfetti 2 s, alle anderen 3 s).
- **„Mindestwartezeit zwischen Auslösungen“**: 0 bis 60 s (Standard 1 s), ab dem Loslassen.
- **„Sicherheitskanal“**: **„Keiner“** oder der Kanal des Fixtures, den die Maschine zum Auslösen braucht, mit seinen **„Werte von / bis“** (0 bis 255; „von“ darf nicht größer als „bis“ sein). Dieser Kanal sendet seine Werte nur, während der Effekt scharf ist und auslöst („von“, wenn die Szene ihm keinen Wert gibt); sonst 0. Er lässt sich nur wählen, wenn die ausgewählten Fixtures dieselben Kanäle haben, und ein Fixture mit nur einem Kanal kann keinen haben („… braucht einen weiteren Kanal, um auszulösen“). Wechselt das Fixture den Modus oder die Definition und liegt an seiner Stelle ein anderer Kanal, wird der Sicherheitskanal entfernt, und eine Meldung sagt es („Der Sicherheitskanal von … wurde entfernt“).
- **„Wie seine Art (…)“**: kehrt zum Schutz und zu den Zeiten seiner Art zurück, ohne Sicherheitskanal.

**Welche Kanäle auslösen.** Alle außer Pan, Tilt und dem Sicherheitskanal, welcher Art sie auch sind (Intensität, Effekt, Höhe…): Für sie gilt die Höchstzeit. Pan und Tilt gehen nur bei scharf geschalteten Effekten und aus Szenen hinaus, ohne Zeitgrenze, und entschärft auf 0: Eine Szene, die die Maschine nur ausrichtet, löst sie nicht aus. Eine Maschine, die nur Pan und Tilt hat, löst auf allen Kanälen außer dem Sicherheitskanal aus. Ein Fixture, das in zwei Universen liegt (aus einer QLC+-Show), löst nie aus: Das Fenster warnt davor; verschieben Sie es in ein einziges Universum.

In der Liste ein **Schild** nach dem Namen; im Plan ein Schild oben links: normalerweise blass, rot bei scharf geschalteten Effekten und mit einem pulsierenden Ring, während er auslöst. **Identifizieren** schaltet sie nie ein. [DMX-Pegel](salidas.md#dmx-pegel) und der Live-Plan zeigen, was wirklich hinausgeht: Ein entschärftes CO2 steht auf 0, auch wenn seine Szene läuft.

Der Schutz wird mit der Show gespeichert, in ihrem Abschnitt von DMX Total Control, und [der Import des Patch einer anderen Show](patch.md#aus-einer-anderen-show-importieren) bringt ihn mit. Öffnen Sie eine ältere Show oder eine QLC+-Show, sagt eine Meldung einmal, welche Fixtures jetzt nach ihrer Art geschützte Effekte sind („2 Fixtures dieser Show sind jetzt geschützte Spezialeffekte: …“); auch eine Nebelmaschine, deren Modell „CO2“ nennt, oder ein generischer Dimmer namens „Fuego…“ oder „Fire…“. Zum Ändern nutzen Sie „Spezialeffekt“. QLC+ liest diesen Schutz nicht: In QLC+ geöffnet, löst die Show diese Fixtures von überall aus, ohne Scharfschalten und ohne Höchstzeit. Frühere Versionen von DMX Total Control lesen ihn auch nicht: Sie lösen diese Fixtures ohne Scharfschalten aus und entfernen beim Speichern der Show den von Hand gesetzten Schutz, die Zeiten und den Sicherheitskanal. Verwenden Sie diese Version auf allen Computern der Show.

## Kanäle parken

Ein **geparkter** Kanal geht immer mit einem festen Wert hinaus, über den Szenen, den Cuelists, den FX, dem Programmer, dem Mischer (Full, Flash…), dem Grand Master und Alles stoppen: der Hazer die ganze Nacht auf 30 %, ein defekter Kopf aus und still, das Saallicht. Esc und Blackout entparken ihn nie.

Im [Fixture-Menü](patch.md#auswählen) (Rechtsklick in der Liste oder im Plan), unter **Parken**, das rechts anzeigt, wie viele der Auswahl geparkt sind:

- **„So parken, wie es jetzt ist“**: alle seine Kanäle auf das, was jetzt hinausgeht, also mit dem Grand Master, dem Mischer und den Grenzen dieses Moments. Nicht möglich während des Show-Stopps oder eines Ausblendens auf Schwarz (ihr Licht bliebe für immer geparkt).
- **„Ausgeschaltet parken“**: die Intensitätskanäle (Dimmer und Farben) auf 0 und die anderen, wie sie jetzt sind; ein Fixture ohne Intensitätskanäle (eine Nebelmaschine oder ein Hazer aus der Bibliothek) ganz auf 0. Während des Show-Stopps oder eines Ausblendens auf Schwarz werden die anderen Kanäle auf das geparkt, was die Show darunter sendet.
- **„Auch im Blackout“** (ein Schalter, wenn ein Fixture der Auswahl geparkt ist): Ist er an, behalten seine geparkten Intensitäten beim Blackout ihren Wert. Ist er aus, gehen sie beim Blackout aus; Pan, Tilt, Gobo und die anderen Kanäle, die kein Licht sind, bleiben immer geparkt.
- **„Entparken“** (wenn ein Fixture der Auswahl geparkt ist) und, unter **Ganze Show**, **„Alle entparken“**: alle geparkten Fixtures der Show, nicht nur die ausgewählten (wie die Schaltfläche im Kopf des Patch).

Unter [Programmieren](programar.md#parken) parkt **„Parken“** die Werte, die der Programmer auf den ausgewählten Fixtures hat.

Mit geparkten Fixtures zeigt der Kopf des Patch **„N geparkt · Alle entparken“** (in einem schmalen Fenster das Schloss, die Zahl und die Taste nur mit ihrem Symbol). In der Liste ein **Schloss** nach dem Namen („Geparkt: N Kanäle mit einem festen Wert am Ausgang“); im 2D-Plan, im Patch und unter Programmieren, ein Schloss oben rechts. Unter [DMX-Pegel](salidas.md#dmx-pegel) ein Schloss auf jedem geparkten Kanal.

- Ein geschützter Spezialeffekt kann nur **ausgeschaltet** geparkt werden: „So parken, wie es jetzt ist“ oder „Parken“ unter Programmieren parken ihn auf 0 und sagen es („Ein geschützter Spezialeffekt kann nur ausgeschaltet geparkt werden“). Geparkt löst er nie aus, auch nicht scharf geschaltet; eine durch das Parken abgeschaltete Auslösung steht im Protokoll als „geparkt“. Ein eingeschaltet geparktes Fixture, das geschützt wird, wird ausgeschaltet geparkt; wird sein Schutz später aufgehoben, bleibt es auf 0 geparkt, bis Sie es entparken. Entparkt löst es erst aus, nachdem seine Szene losgelassen und erneut gedrückt wurde.
- Geparkte Kanäle folgen dem Fixture, wenn Sie es neu adressieren. Ein anderer Modus eines Fixtures entfernt seine geparkten Kanäle („Die geparkten Kanäle von Spot 3 wurden entfernt: Der Modus hat sich geändert“), außer bei einem geschützten Effekt, der ausgeschaltet geparkt bleibt. Wird ein Fixture gelöscht, gehen seine mit. Ein geparkter Pan eines Fixtures mit vertauschtem Pan und Tilt geht auf dem Tilt-Kanal hinaus, wie der Pan.
- Der [Show-Stopp](directo.md#show-stopp) gewinnt gegen geparkte Kanäle: Ein aus geparktes Fixture leuchtet auch, und ein geparkter Hazer bleibt geparkt. Ein [Ausblenden auf Schwarz](directo.md#ausblenden-auf-schwarz) nimmt geparkte Kanäle ohne „Auch im Blackout“ herunter.

Geparkte Kanäle werden mit der Show gespeichert, in ihrem Abschnitt von DMX Total Control (der Import des Patch einer anderen Show bringt sie nicht mit). Öffnen Sie eine Show mit geparkten Kanälen, gelten sie sofort, und eine Meldung sagt, welche („2 geparkte Fixtures: HAZER, Spot 3 (Patch)“). Eine andere oder neue Show entfernt sie. Beim Schließen des Programms werden sie losgelassen (die Bühne geht in Ruhe), bleiben aber in der Show. QLC+ parkt keine Kanäle: Dort spielen sie, wie ihre Szenen es sagen, und die Kopie für QLC+ warnt davor („2 geparkte Fixtures: QLC+ parkt keine Kanäle“). Frühere Versionen von DMX Total Control wenden sie nicht an und entfernen sie beim Speichern der Show, ebenso die Einstellungen des Show-Stopps.

> **Nicht mit echten Geräten getestet:** Das Parken wurde nicht mit echten Interfaces, Nodes oder Hazern getestet.

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

- **„Patch einer anderen Show…“**: ihre Fixtures. Sie kommen auf dieselben Adressen, wenn diese frei sind; sonst auf die nächsten. Sie bringen ihren Spezialeffekt-Schutz, ihre Zeiten, ihren Sicherheitskanal und ihr Plan-Symbol mit (was die andere Show sagt, hebt den Schutz eines CO2 nie auf), aber nicht ihre geparkten Kanäle.
- **„Szenen einer anderen Show…“**: ihre Szenen, zugeordnet zu den passenden Fixtures dieser Show (nach Adresse und Modell, nach Name oder nach Modell). Sie kommen in Gruppen mit demselben Namen, mit ihren Optionen in der Gruppe (Live-Größe, -Phase und -Tempo, Freigabe, Flash, Dimmer…). Eine neue Gruppe bringt auch ihr Crossfade und ihren Master mit; eine, die es in dieser Show schon gibt, behält ihre eigenen.
- **„Patch und Szenen einer anderen Show…“**: beides.
- **„Patch aus einer Visualisierungssoftware (CSV)…“**: die Fixture-Tabelle, die Ihre Visualisierungssoftware exportiert; siehe [Einen Patch aus einer Visualisierungssoftware importieren](patch.md#einen-patch-aus-einer-visualisierungssoftware-importieren).

## Einen Patch aus einer Visualisierungssoftware importieren

Wenn Sie das Rig in einer Visualisierungssoftware planen, exportieren Sie deren Fixture-Tabelle (CSV) und importieren Sie sie: DMX Total Control patcht dieselben Fixtures, in dieselben Universen und auf dieselben Adressen, mit ihren Namen, Nummern und ihrem Platz im Plan.

1. Drücken Sie **„Importieren“ › „Patch aus einer Visualisierungssoftware (CSV)…“** und wählen Sie die Datei.
2. Das Fenster **„Patch importieren“** zeigt, wie viele Fixtures die Tabelle hat, von wie vielen Typen und in welchen Universen. Nichts ändert sich, bis Sie **„Importieren“** drücken.
3. Unter **„Fixture-Typen“** zeigt jeder Typ der Tabelle (Fixture, Modus und Kanäle), was daraus wird und wie er in der Bibliothek gefunden wurde: **„Genau“** (dasselbe Modell und derselbe Modus), **„Nach Kanalzahl“** (der Modus mit diesen Kanälen), **„Ähnlicher Name“**, **„Gemerkt“** oder **„Von Hand gewählt“**. Ein **„Ähnlicher Name“** erscheint in Orange, damit Sie ihn prüfen: es ist ein Modell mit ähnlichem Namen, nicht dasselbe. Ist er nicht in der Bibliothek, wird er als **„Generischer Dimmer“** mit seinen Kanälen gepatcht, in Orange: alles bleibt trotzdem auf seinen Adressen. Jeder Typ zeigt auch das Symbol, das seine Fixtures auf dem Plan haben werden; wird er als generischer Dimmer gepatcht, bekommt er das, was sein Name in der Tabelle sagt (ein „CO2 Jet“ erscheint als CO2-Jet; ein „Flame“ als Flamme).
4. **„Ändern“** wählt ein anderes Fixture und einen anderen Modus (**„Aus der Bibliothek wählen…“**), einen generischen Dimmer oder **„Nicht importieren“**. Was Sie von Hand wählen, wird für den nächsten Import dieses Typs gemerkt.
5. Unter **„Optionen“**:
   - **„Auf dem Bühnenplan platzieren“**, mit der **Ansicht**: **„Von oben“** (das Publikum unten im Plan) oder **„Von vorne“** (wie das Publikum das Rig sieht: die höchsten Fixtures oben). Die Ansicht von vorne wird vorgeschlagen, wenn das Rig in verschiedenen Höhen hängt, und die zuletzt benutzte wird gemerkt. Der Plan wächst, damit das Rig hineinpasst (er schrumpft nie). Jeder Import, der sie platziert, zentriert das Rig wieder auf dem Plan, auch nachdem Sie seine Seiten mit „−“ und „+“ geändert haben.
   - **Gruppen**: eine für jeden Fixture-Typ, eine für jede Ebene (wenn die Tabelle mehr als eine hat) und die der Spalte „Groups“ der Tabelle.
6. Unter den Typen stehen die Probleme, vor dem Import: Adressen, die schon ein anderes Fixture oder eine andere Zeile belegt (diese Zeilen werden nicht importiert), Zeilen ohne Universum oder Adresse, Universen über 128, Adressen über 512, ein zweiter DMX-Break (nicht importiert), Fixture-Nummern, die schon ein anderes Fixture hat oder die zu hoch sind (sie bekommen eine andere), ein invertierter Pan oder Tilt, der weder Ja noch Nein ist, und die Universen, die der Show hinzugefügt werden.
7. Drücken Sie **„N Fixtures importieren“**. Eine Meldung zeigt, wie viele neu sind und wie viele aktualisiert wurden, und ob ein Problem bleibt.

**Aus jeder Zeile** werden das Fixture und sein Modus, Universum und Adresse, der Name (die Spalte „Unit“) und die Fixture-Nummer importiert: die der Spalte „Channel“, sonst die der anderen Spalte für die Fixture-Nummer, sonst eine automatische. Die Höhe jedes Moving Heads, ob er steht (in der Tabelle umgedreht) oder hängt, und sein invertierter Pan oder Tilt kommen in seine [Höhe und Ausrichtung](patch.md#höhe-und-ausrichtung-follow), ob Sie sie auf dem Plan platzieren oder nicht.

**Ein erneuter Import** der Tabelle (nachdem Sie das Rig in der Visualisierungssoftware geändert haben) aktualisiert dieselben Fixtures: sie werden an der Kennung jeder Zeile der Tabelle erkannt, oder am gleichen Namen und Fixture, und Modus, Adresse, Name, Nummer und Platz im Plan ändern sich. Die, die nicht mehr in der Tabelle sind, bleiben, außer Sie haken **„Die N vorher importierten Fixtures entfernen, die nicht mehr in der Tabelle sind“** an (darunter stehen ihre Namen). Ein Symbol, das Sie danach von Hand wählen, bleibt erhalten, außer das Fixture, als das sein Typ gepatcht wird, ändert sich.

> **Grenzen:** von jedem Fixture wird nur der erste DMX-Break importiert, und von den Rotationen zählt nur, ob ein Kopf umgedreht ist (die Ausrichtung wird nicht importiert). Von oben gesehen landen übereinander hängende Fixtures zusammen im Plan: um sie getrennt zu sehen, nutzen Sie die Ansicht von vorne.

## Fixtures löschen

Wählen Sie sie aus und drücken Sie **„Löschen“**. Vorher wird nachgefragt: Ihre Werte werden aus allen Szenen entfernt.

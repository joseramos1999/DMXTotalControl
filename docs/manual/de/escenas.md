# Szenen

![Die Seite Szenen](../img/escenas.png)

**Szenen** (F4) ist die Bibliothek für alles, was Sie aufgenommen haben: statische Szenen, Schritte, FX, Farbeffekte, Racks, Superszenen und Cuelists. Links steht die Liste, rechts der Editor der gewählten Szene, je nach Typ verschieden.

Fast alles wird aus dem Programmer erstellt: Neue Effekte übernehmen die unter [Programmieren](programar.md) ausgewählten Fixtures, Schritte und Cues seine Werte.

## Die Szenenliste

- **„Suchen…“**: filtert nach Name (oder nach dem Namen des Ordners).
- **„Ordner“** / **„Gruppen“**: die Szenen nach [Ordnern](escenas.md#ordner) anzeigen oder nach den Live-Gruppen, in denen sie sind („In keiner Gruppe“ für den Rest).
- **Alle einklappen** / **Alle ausklappen**: die Taste rechts.
- **Filter nach Typ**: „Alle“, „Statisch“, „Schritte“, „Cuelists“, „FX“, „Colour FX“, „Racks“ und „Superszenen“.
- **Strg+Klick** und **Umschalt+Klick** wählen mehrere Szenen aus, um sie in einen Ordner zu verschieben („In Ordner…“).

Jede Szene hat einen Streifen in der Farbe ihres Typs. Klicken Sie auf eine, um sie im Editor zu öffnen.

## Eine Szene erstellen

Drücken Sie **„Neu“**:

| Option | Was sie erstellt |
|---|---|
| **Statisch (aus dem Programmer)** | Eine statische Szene mit den Werten des Programmers. |
| **Schritte (Programmer = erster Schritt)** | Eine Schritte-Szene. |
| **Cuelist (Programmer = Cue 1)** | Eine Cuelist. |
| **FX (im Programmer)** | Bringt Sie zu Programmieren › FX: FX werden dort erstellt und mit „Aufnehmen…“ aufgenommen. |
| **Colour FX (ausgewählte Fixtures)** | Ein Farbeffekt auf den ausgewählten Fixtures, in ihrer Reihenfolge. |
| **Mapping auf dem 2D-Plan (ausgewählte Fixtures)** | Ein Farbeffekt, der dort liegt, wo die Fixtures im 2D-Plan stehen. |
| **Effekt-Rack** | Ein leeres Rack. |
| **Superszene (Zeitleiste mit Audio)** | Eine leere Superszene. |
| **Aus einer Datei importieren…** | Szenen und Effekte, die aus einer anderen Show exportiert wurden (siehe [Importieren und exportieren](escenas.md#importieren-und-exportieren)). |

## Der Kopf des Editors

- **Der Name**: Klicken Sie darauf, um ihn zu ändern.
- **Der Ordner** (Ordnersymbol): die Szene in einen Ordner verschieben.
- **„Testen“** / **„Stopp“**: spielt sie jetzt ab, außerhalb der Gruppen. Cuelists haben ihr eigenes GO.
- **„In Gruppe…“**: fügt sie einer Live-Gruppe hinzu.
- **In Schritte umwandeln** (Racks, FX, Colour FX, Move FX, Curve FX und Chaser): nimmt einen Zyklus in eine neue Schritte-Szene auf, nur mit den Kanälen, denen die Szene einen Wert gibt: Die anderen kommen nicht in die Schritte, auch wenn sie einen Standardwert haben (Pan oder Zoom in der Mitte bei vielen Moving Heads).
- **In eine Cuelist umwandeln** (Schritte): siehe [Cuelists](escenas.md#cuelists).
- **Exportieren**, **„Duplizieren“** und **„Löschen“** (die Szene wird auch aus ihren Gruppen und Racks entfernt).

## Wie eine Szene läuft

Unter dem Kopf liegt der Wiedergabebereich. Er zeigt, was für den jeweiligen Typ sinnvoll ist:

- **„Einblendung“** und **„Ausblendung“**.
- **„Zykluszeit“** (oder **„Schrittzeit“** bei einem Colour FX) und **„Tempo“** (von einem Zyklus pro Minute bis zehn pro Sekunde; Doppelklick: ein Zyklus pro Sekunde), mit **×½** und **×2**.
- **„Staffelung“** (statische Szenen und Schritte): Jedes Fixture beginnt seine Überblendung so viel später als das vorherige, in Auswahlreihenfolge.
- **„Gesteuert durch“**: **„Dauer“** (die normalen Zeiten), **„BPM“** (die Zeiten werden zu Beats und folgen dem Tempo der Show), **„BeatGO“** oder **„Puls“** (schaltet mit der Taste BeatGO oder mit den Spitzen des Audioeingangs weiter). Mit BeatGO oder Puls: **„Unterteilungen“** (/2, ×2).
- **„Tap-Rhythmus“** (Sequenzen und Chaser): der Rhythmus, den Sie durch mehrmaliges Drücken ihrer Taste vorgegeben haben ([Live](directo.md#rhythmus-einer-szene-mit-schritten)), in BPM. Solange sie ihn hat, laufen ihre Schritte in diesem Rhythmus statt nach ihren Zeiten. **„Entfernen“** kehrt zu ihren Zeiten zurück.
- **„Start“**: **„Läuft“** oder **„Pausiert, erneut drücken zum Abspielen“**.
- **„Läuft“**: **„Immer“** oder **„N-mal“**, **„dann“** **„Freigeben“** oder **„Letztes Bild halten“**, **„und abspielen“** „Nichts“, „Nächste Szene der Gruppe“, „Vorherige Szene der Gruppe“ oder jede andere Szene.

## Szenentypen

### Statisch

Speichert feste Werte. Zeigt, wie viele Kanäle und Fixtures sie hat und welche Paletten sie verwendet („Verwendet diese Paletten: ändern sie sich, ändert sich auch die Szene“).

So ändern Sie sie:

1. Drücken Sie **„Im Programmer bearbeiten“**: Sie wird unter Programmieren geladen.
2. Ändern Sie, was Sie möchten.
3. Gehen Sie zurück zu Szenen und drücken Sie **„Programmer in dieser Szene speichern“**.

Speichern Sie FX in einer statischen Szene, wird sie zu einer FX-Szene.

### Schritte

Eine Folge von Looks, jeder mit seiner **Einblendung**, seiner **Haltezeit** und seiner **Ausblendung**.

- **„Schritt hinzufügen“**: Der Programmer wird zu einem neuen Schritt nach dem ausgewählten.
- **„In Schritt speichern“**: speichert den Programmer im ausgewählten Schritt.
- **„Schritt laden“** (oder Doppelklick auf einen Schritt): lädt ihn in den Programmer, um ihn zu sehen und zu ändern.
- **Duplizieren**, **▲ ▼** zum Umordnen und **Löschen** des Schritts.
- Die Taste **∞** der Haltezeit lässt den Schritt auf ein GO warten („Bis GO“).

**Strg+Klick** und **Umschalt+Klick** wählen mehrere Schritte aus: Ändern Sie eine Zeit, ändert sie sich bei allen. Schritte wenden Paletten als Wert an, nicht per Referenz.

### FX

Eine FX-Szene hat **Grundwerte** und eine oder mehrere **FX-Ebenen**. Sie entsteht, wenn Sie aus dem Programmer mit FX aufnehmen (siehe [FX](programar.md#fx)).

- Links die Liste der FX: Parameter, Fixtures und Tempo. Der Papierkorb entfernt eine Ebene.
- Rechts die Regler der gewählten Ebene (dieselben wie unter Programmieren) und ihre animierte Vorschau. Änderungen wirken live, während die Szene läuft.
- **„Verknüpfen“**: Verknüpfte Ebenen teilen Tempo und Timing.
- Machen einige Fixtures den FX auf eigene Weise, erscheint ein Hinweis: Ändern Sie hier Form, Größe, Richtung oder Phase, tun danach alle dasselbe.
- Um Ebenen hinzuzufügen oder die Grundwerte zu ändern: **„Im Programmer bearbeiten“** und danach **„Programmer in dieser Szene speichern“**.

### Colour FX und Mappings

Ein **Colour FX** ist ein Pixeleffekt (über 60: Verläufe, Feuer, Meteore, Stroboskope, Wellen…), der die Fixtures in Auswahlreihenfolge durchläuft.

- **„Effekt“**: welcher.
- **„Wirkt auf“**: „Farbe (RGB)“, „Weiß“, „Amber“, „UV“, „Dimmer (Wert)“ oder „Shutter“.
- **„Farben“**: bis zu 5, je nach Effekt. Klicken Sie auf eine Farbe, um sie zu ändern (Farbfelder oder Farbton), oder „Keine Farbe“.
- **„Text“**: bei Texteffekten der Text, der über die Fixtures läuft.
- **„Bild“** und **„Animation“**: bei Bildeffekten die Datei und „Statisch“, „Seitlich scrollen“, „Auf und ab scrollen“ oder „Animiert (GIF)“.
- **„Anordnung“**: **„In einer Reihe“** (in Auswahlreihenfolge) oder **„Auf dem 2D-Plan“** (dort, wo die Fixtures stehen).
- **„Parameter“**: die eigenen Parameter jedes Effekts.
- **„Auswahl des Programmers verwenden“**: ersetzt seine Fixtures durch die ausgewählten.

Oben sehen Sie die tatsächliche Farbe jedes Kopfes, während der Effekt läuft (drücken Sie „Testen“, um sie zu sehen).

Ein **Mapping auf dem 2D-Plan** ist ein Colour FX, der schon mit der Anordnung auf dem Plan entsteht: Effekt, Text oder Bild liegen so auf der Bühne, wie die Fixtures stehen.

### Move FX und Curve FX

Das sind Effekte aus dem früheren Modul: **Move FX** bewegen Pan/Tilt in einer Form, und **Curve FX** geben Kanälen eine Welle (Sinus, Dreieck, Rampen, Rechteck, Zufall, Stroboskop…). Sie finden sie in alten Shows oder in Shows von QLC+; über „Neu“ werden sie nicht mehr erstellt.

Sie laufen weiter wie bisher und haben ihren Editor (Form, Breite, Höhe, Drehung, Position, Phase, Verteilung auf die Fixtures, symmetrisch, relativ…). Ein Hinweis bietet **„In FX umwandeln“** an: Sie wechseln zum FX-Editor, auch im Programmer, und Gruppen, SHOW-Tasten und Zuordnungen behalten sie.

### Effekt-Rack

Mehrere Szenen und Effekte, die zusammen wie eine einzige laufen, jede auf ihren eigenen Fixtures.

- Klicken Sie unter **„Hinzufügen“** (rechts) auf eine Szene, um sie ins Rack zu legen.
- Unter **„Im Rack (N)“** entfernt das **×** sie.

Ein Rack kann keine Szene enthalten, die es selbst abspielt.

### Chaser

Chaser aus QLC+-Shows laufen normal, haben aber keinen Editor. Sie können sie mit der Umwandeln-Taste im Kopf in Schritte umwandeln.

## Superszenen

Eine **Superszene** ist eine Zeitleiste mit Spuren für Szenen und Audio. Sie wird wie jede Szene gestartet (in einer Gruppe, in der SHOW, mit einer Zuordnung). Ihr Editor füllt die ganze Seite; die Listentaste oben links führt zurück zur Szenenliste.

### Die Werkzeugleiste

- **„Abspielen“** / **„Stopp“** (**Leertaste**): ab dem Cursor. **⏮** geht zum Anfang zurück. Daneben sehen Sie die Zeit (und den Takt).
- **„Zeit“** / **„Takte“**: das Raster. Bei Takten: die **BPM**, die Taktart (**4/4**, **3/4**, **2/4**) und **„Takt 1 hier“**, damit der erste Takt am Cursor beginnt und zur Musik passt.
- **„Einrasten“**: Blöcke und Punkte rasten am Raster ein.
- **„Ende hier“**: Die Superszene endet am Cursor (das **×** entfernt die Marke: Sie endet mit ihrem letzten Block). **„Schleife“**: beginnt von vorn.
- **„+ Spur“** und **„+ Audio“** (eine Audiodatei auf der ausgewählten Spur, am Cursor).
- **„Statische Werte“**: **„Werte des Programmers übernehmen“** (sie laufen, solange die Superszene läuft) oder **„Statische Werte löschen“**.
- **Zoom** − und +.

### Szenen auf die Zeitleiste legen

1. Drücken Sie **„+ Spur“**, wenn es noch keine gibt.
2. Ziehen Sie eine Szene aus der Liste links auf eine Spur. Oder wählen Sie eine Spur aus, setzen Sie den Cursor und **doppelklicken** Sie auf die Szene: Sie wird am Cursor eingefügt.

Blöcke dürfen sich überlappen.

### Mit den Blöcken arbeiten

- **Ziehen** Sie einen Block, um ihn zu verschieben, auch auf eine andere Spur. Ziehen Sie seine **Ränder**, um seine Dauer zu ändern.
- Bei ausgewähltem Block sind die **Griffe** an den oberen Ecken seine Ein- und Ausblendung.
- Verlängern Sie einen Block, **wiederholt sich die Szene in einer Schleife** (bei jedem Durchlauf sind Markierungen zu sehen) oder sie wird **gestreckt**, sodass ein Zyklus den ganzen Block dauert. Das ändern Sie im Menü des Blocks oder in der unteren Leiste („Schleife“ / „Gestreckt“).
- **Rechtsklick**: „Szene im Block wiederholen“ / „Szene auf den Block strecken“, „Duplizieren“, „Sperren“ / „Entsperren“, „Szene öffnen“ und „Löschen“.
- Ein **Doppelklick** auf einen Block öffnet seine Szene. **Entf** löscht den ausgewählten Block.
- Die untere Leiste zeigt den ausgewählten Block: **Start**, **Länge**, **Einblendung**, **Ausblendung**.

### Audio

Audioblöcke (wav, flac, ogg, aiff und mp3, wenn das System es unterstützt) zeigen ihre **Wellenform**, die im Hintergrund berechnet wird. Zum Hinzufügen brauchen Sie eine nicht gesperrte Spur.

### Spuren und Automation

Im Kopf jeder Spur:

- **M**: stummschalten (die Spur läuft nicht).
- **Schloss**: sperren (ihre Blöcke und Punkte bewegen sich nicht).
- **D**: die Automationslinie des **Dimmers** (bei einer Audiospur die Lautstärke).
- **φ**: die Automationslinie der **Phase** ihrer Effekte.
- **⋯**: „Umbenennen“ (auch per Doppelklick auf den Namen), „Nach oben“, „Nach unten“, „Automation dieser Bahn löschen“ und „Spur löschen“.
- Der Pfeil rechts blendet die Spur aus oder ein.

Auf einer Automationslinie: **Klick** fügt einen Punkt hinzu, **Ziehen** verschiebt ihn, **Doppelklick** löscht ihn und **Rechtsklick** wechselt zwischen gerader Linie und weicher Kurve. Ohne Punkte steht der Dimmer auf voll und die Phase auf 0.

### Auf der Zeitleiste navigieren

- Klick oder Ziehen auf dem **Lineal** bewegt den Cursor; ein **Doppelklick** auf das Lineal spielt ab dort ab. Ein Klick auf eine leere Spur bewegt den Cursor ebenfalls.
- **Strg+Mausrad**: Zoom. **Umschalt+Mausrad**: in der Zeit scrollen. Das Mausrad allein scrollt durch die Spuren.

## Cuelists

Eine **Cuelist** ist eine Liste von Cues, die mit GO weiterschalten. Sie arbeitet mit **Tracking**: Jeder Cue speichert **nur, was sich ändert**, und der Rest bleibt von den vorherigen Cues. Ein GO ändert nur die Kanäle, die sich ändern: keine Einbrüche und kein erneutes Anfahren dessen, was unberührt bleibt.

Cuelists werden aus dem Programmer aufgenommen, unter [Playbacks](playbacks.md) bedient und hier bearbeitet.

### Optionen der Cuelist

Oben im Editor:

- **„Tracking“**: An: Jeder Cue speichert nur, was sich ändert. Aus: Jeder Cue steht für sich.
- **„Nach dem letzten Cue“**: **„Schleife“** (zurück zum ersten), **„Freigeben“** oder **„Bleiben“**.
- **„Freigabezeit“**: die Überblendung beim Freigeben der Cuelist.
- **„Bei Freigabe zurücksetzen“**: Nach dem Freigeben beginnt GO wieder beim ersten Cue.
- **„Freigabe durch anderes GO“**: ein Song, der endet, wenn der nächste beginnt.
- **„Überblendung neuer Cues“**: die Überblendung des ersten aufgenommenen Cues; die folgenden übernehmen die Zeiten des vorherigen.
- **„Zurück“**: die Zeiten von BACK: **„Cue-Zeiten“** oder **„Feste Zeit“**.
- **„Priorität“**.
- **„GO im Takt“**: „Aus“, „BeatGO“ oder „Puls“; mit BeatGO oder Puls kommt bei jeder Unterteilung ein GO.
- **„Tap-Rhythmus“**: der Rhythmus, den Sie durch mehrmaliges Drücken ihrer Taste vorgegeben haben, in BPM: ein GO auf jeden Schlag. Solange sie ihn hat, zählen ihre Zeiten für Folgen und Warten nicht. **„Entfernen“** kehrt zu ihren Zeiten zurück.

In einem schmalen Fenster wird ein Teil der Optionen eingeklappt: **„Mehr Optionen“** zeigt sie.

### Die Cue-Tabelle

| Spalte | Was sie ist |
|---|---|
| **CUE** | Die Nummer. Dezimalstellen sind erlaubt: 2.5 liegt zwischen 2 und 3. Doppelklick, um sie zu ändern. Ein Stift zeigt an, dass der Cue in den Programmer geladen ist. |
| **NAME** | Sein Name. |
| **AUSLÖSER** | **GO** (wartet auf ein GO), **Folgen** (wenn der vorherige Cue fertig ist, plus eine Zeit) oder **Warten** (eine Zeit nach dem GO des vorherigen Cues). |
| **VERZÖG.** | Wie lange der Cue wartet, bevor seine Überblendung beginnt. |
| **ÜBERBL.** | Die Überblendzeit. |
| **AUSBL.** | Die Überblendung der Dimmer, die abwärts gehen. „=“ ist gleich der Überblendung. |
| **ÄNDERUNGEN** | Wie viele Kanäle er ändert, sowie die Markierungen **FX**, **[R]** (freigegebene Werte) und das Palettensymbol, wenn er Paletten verwendet. |
| **S** | **Überspringen** (nur dieser Cue): Was er ändert, geht nicht in die folgenden Cues über. |
| **B** | **Block**: Aufnehmen und Aktualisieren speichern auch die Werte, die den übernommenen gleichen. |
| **NOTIZ** | Eine Notiz für Sie. |

Um eine Zeit zu ändern, klicken Sie darauf und geben Sie die Sekunden ein, oder verwenden Sie das **Mausrad** über der angeklickten Zeit: ±0,1 s (mit Umschalt 1 s). Mit **Strg+Klick** oder **Umschalt+Klick** wählen Sie mehrere Cues aus und ändern ihre Zeiten, ihren Auslöser oder ihre Markierungen gleichzeitig.

Der laufende Cue trägt einen grünen Balken mit dem Fortschritt der Überblendung, der nächste einen grünen Rand. Oben sehen Sie den Status („Q 3 · Strophe ▸ 4“ oder „Freigegeben · GO startet 1“) und die Tasten **Zurück**, **GO** und **Freigeben**, dieselben wie bei seinem Playback.

### Cues aufnehmen und bearbeiten

- **„Cue N aufnehmen“**: Der Programmer wird zu einem neuen Cue nach dem ausgewählten, nur mit dem, was sich gegenüber den übernommenen Werten ändert.
- **„Aktualisieren ▾“**: führt den Programmer mit dem ausgewählten Cue zusammen: **„Cue N aktualisieren (auch für die folgenden Cues)“** oder **„Nur dieser Cue (die folgenden Cues behalten ihren Look)“**.
- **„Laden ▾“** (oder Doppelklick auf einen Cue): lädt ihn in den Programmer: **„Was die Bühne bei Cue N zeigt“** oder **„Nur, was Cue N ändert“**. Aktualisieren Sie ihn danach über die Leiste „Cue … wird bearbeitet“ unter Programmieren.
- **„Entfernen ▾“**: mit den Kanälen des Programmers **„Kanäle des Programmers aus Cue N entfernen (sie werden wieder übernommen)“** oder **„Kanäle des Programmers in Cue N [freigeben]“** (ab diesem Cue kehren sie zu dem zurück, was darunter liegt: andere Szenen oder ihr Ruhewert).
- **Kopieren** ans Ende: **„Kopieren, was Cue N ändert“** oder **„Ganzen Look von Cue N kopieren (Block-Cue)“**.
- **„Nummer…“**: eine andere Nummer für den ausgewählten Cue. Das Verschieben eines Cues ändert, was die folgenden Cues übernehmen.
- **Löschen**: Ist **„Look der folgenden Cues beibehalten“** angehakt, geht, was der Cue ändert, in den nächsten über, und die folgenden sehen gleich aus.

In einer schmalen Liste (oder in einer Sprache mit langen Wörtern) zeigen „Aktualisieren“, „Laden“ und „Entfernen“ nur ihr Symbol und ihren Pfeil **▾**; fahren Sie mit der Maus darüber, um ihren Namen zu sehen.

Der Rechtsklick auf einen Cue bietet **„Zu Cue N gehen“**, die beiden Arten zu laden, **„Mit dem Programmer aktualisieren“**, **„Nummer…“** und **„Löschen…“**.

Cues können **Paletten per Referenz** verwenden und **FX-Ebenen** enthalten, die von Cue zu Cue übernommen werden.

### Die Tracking-Tabelle

**„Tracking-Tabelle“** zeigt, was die Bühne beim ausgewählten Cue enthält, Fixture für Fixture („Cue N auf der Bühne“). Ist das Fenster breit, erscheint sie von selbst neben den Cues.

- Die farbigen Kacheln zeigen, was jeder Wert ist: **„Änderungen“** (dieser Cue ändert ihn), **„Übernommen“** (stammt von einem früheren Cue, der angegeben wird: „Q2“), **„Block“** und **„Freigegeben“** (durchgestrichen, mit [R]).
- Oben sehen Sie die Paletten, die der Cue verwendet.
- Klicken Sie auf Kanäle, um sie zu wählen, und drücken Sie **„Freigeben“** (setzt sie ab diesem Cue auf [RELEASE]) oder **„Entfernen“** (sie werden wieder von den vorherigen Cues übernommen).

### Schritte in eine Cuelist umwandeln

In einer Schritte-Szene wandelt die Taste **„In eine Cuelist umwandeln“** im Kopf jeden Schritt in einen Cue mit dem um, was er gegenüber dem vorherigen ändert; Schritte mit Zeit werden mit **Warten** verkettet. Ihre Gruppen, SHOW-Tasten und Zuordnungen spielen danach die Cuelist ab. Die Schritte entfallen.

## Ordner

Ordner sortieren die Szenenliste (auch in der SHOW, in den Superszenen, in den Racks und beim Hinzufügen von Szenen zu einer Gruppe).

- **Erstellen**: die Ordnertaste mit **+** neben der Suche oder **„Neuer Ordner…“** im Menü einer Szene.
- **Szenen hineinlegen**: Ziehen Sie sie auf den Ordner oder wählen Sie Rechtsklick › **„In Ordner verschieben“**. Funktioniert mit mehreren ausgewählten Szenen.
- **Herausnehmen**: Rechtsklick › **„Aus dem Ordner nehmen“**.
- **Umbenennen oder löschen**: die Taste **⋯** des Ordners (oder Rechtsklick): **„Ordner umbenennen“** und **„Ordner löschen (Szenen bleiben)“**.
- Klicken Sie auf den Kopf eines Ordners, um ihn ein- oder auszuklappen.

Ordner werden mit der Show gespeichert.

## Importieren und exportieren

- **Exportieren**: Die Exporttaste im Kopf speichert die Szene in einer `.dtcfx`-Datei („Effekte von DMX Total Control“), um sie in anderen Shows zu laden. Funktioniert auch mit Racks.
- **Importieren**: **„Neu“ › „Aus einer Datei importieren…“**. Die Fixtures werden denen dieser Show zugeordnet; fehlt eines aus der Datei, wird sein Teil ausgelassen, und ein Hinweis erscheint.
- **Die Szenen einer ganzen anderen Show**: unter [Patch › Importieren](patch.md#aus-einer-anderen-show-importieren), „Szenen einer anderen Show…“.

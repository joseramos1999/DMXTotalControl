# Live

![Die Seite Live](../img/directo.png)

**Live** (F1) ist die Seite, auf der Sie während der Show Szenen starten. Die Szenen sind in **Gruppen** geordnet, also in Spalten, die parallel arbeiten: Jede Gruppe hat ihre Szenen, ihren Modus (exklusiv oder mischen), ihre Überblendzeit und ihren Master.

Im Kopf der Seite finden Sie:

- **◀ ▶**: vorherige oder nächste Szene in jeder laufenden Gruppe (auch mit Strg+← und Strg+→).
- **„Pause“** / **„Fortsetzen“**: friert alles ein, was läuft, und setzt es fort.
- **„Gruppen freigeben“**: stoppt die Szenen aller Gruppen.
- **„Mischer“**: wechselt zur Ansicht des [Mischers](directo.md#mischer). Ein orangefarbener Punkt zeigt an, dass der Mischer die Ausgabe verändert.
- **„Neue Gruppe“**: fügt eine Spalte hinzu.

## Gruppen

### Gruppen erstellen und organisieren

1. Drücken Sie **„Neue Gruppe“** (oder das Feld „Gruppe hinzufügen“ am Ende der Spalten).
2. Klicken Sie auf den Namen der Spalte, um ihn zu ändern (zum Beispiel `Wash`, `Spots`, `Stroboskope`, `Effekte`), und drücken Sie Eingabe.

Die Zahnradtaste jeder Spalte öffnet ihr Menü: **„Farbe ändern“** (wechselt zur nächsten Farbe), **„Nach links“**, **„Nach rechts“** und **„Gruppe löschen“**. Sie können die Gruppe auch am Griff links neben ihrem Namen ziehen und auf einer anderen Gruppe ablegen, um deren Platz einzunehmen.

Der Pfeil **‹** neben dem Namen **klappt** die Gruppe **ein**: Sie wird zu einem schmalen Streifen mit ihrem Namen und der Zahl der laufenden Szenen, um Platz für die anderen zu schaffen. Ihre Szenen laufen weiter. Der Pfeil **›** auf dem Streifen klappt sie wieder auf. Das wird mit der Show gespeichert.

### Szenen hinzufügen und ordnen

1. Drücken Sie **„Szenen hinzufügen“** am Ende der Spalte.
2. Klicken Sie in der Liste auf die gewünschten Szenen (sie werden mit ✓ markiert). Sie können nach Name oder Ordner suchen und sie nach Ordnern oder Gruppen anzeigen.
3. Drücken Sie **„N hinzufügen“**. Ein Doppelklick auf eine Szene fügt sie direkt hinzu.

Zum Umordnen fahren Sie mit der Maus über eine Kachel und ziehen sie an ihrem Griff (rechts auf der Kachel): Legen Sie sie auf einer anderen Szene ab, um sie davor einzufügen, oder in der Lücke der Spalte, um sie ans Ende zu setzen. Sie können sie in einer **anderen Gruppe** ablegen, dann wandert sie mit ihren Optionen dorthin. Im Menü der Kachel (Rechtsklick) gibt es außerdem **„Nach oben“**, **„Nach unten“** und **„Aus der Gruppe entfernen“**.

> **Tipp:** Beim Aufnehmen unter Programmieren können Sie die Szene mit „Zur Gruppe hinzufügen“ direkt in eine Gruppe legen. Siehe [Aufnehmen](programar.md#aufnehmen).

## Szenen starten

Klicken Sie auf eine Kachel, um die Szene zu starten; klicken Sie erneut, um sie freizugeben. Die laufende Kachel färbt sich in der Farbe der Gruppe und hat einen pulsierenden Punkt.

Auf jeder Kachel sehen Sie:

- Den **Namen** und darunter den **Typ** der Szene. Bei einer laufenden Cuelist den aktuellen Cue („Q 3 · Strophe“). Dahinter den [Tap-Rhythmus](directo.md#rhythmus-einer-szene-mit-schritten), falls vorhanden („120 BPM“).
- **P7**, **P2**…: die Priorität, nur wenn sie nicht die normale ist (5). In Orange, wenn sie höher als 5 ist.
- Einen **Blitz**: Es ist eine Flash-Szene.
- Einen **zweigeteilten Balken** unten: eine Szene mit Schritten oder eine laufende Cuelist; er wechselt bei jedem Schritt, in ihrem Rhythmus.

### Exklusiv oder mischen

Die Taste jeder Spalte wechselt zwischen:

- **„Exklusiv“**: Es läuft immer nur eine Szene. Starten Sie eine andere, wird zur neuen übergeblendet.
- **„Mischen“**: Die Szenen addieren sich; mehrere können gleichzeitig laufen.

### Überblendzeit

Die Auswahlliste neben „Exklusiv“/„Mischen“ ist die Überblendzeit der Gruppe:

- **„Pro Szene“**: Jede Szene verwendet ihre eigene Ein- und Ausblendung.
- **„Schnitt“**: ohne Überblendung.
- **0,5 s bis 10 s**: diese Zeit für alle Szenen der Gruppe, statt ihrer eigenen.

### Vorherige und nächste Szene

Die Tasten **◀ ▶** jeder Spalte starten die vorherige oder nächste Szene der Gruppe, in der Reihenfolge der Spalte. Die im Kopf der Seite tun dasselbe in allen laufenden Gruppen.

### Flash-Szenen

Eine Szene mit **Flash** läuft nur, solange Sie ihre Kachel gedrückt halten. Sie wird in ihren [Eigenschaften](directo.md#szeneneigenschaften) mit „Flash (läuft, solange gedrückt)“ aktiviert.

### Cuelists in einer Gruppe

Eine Cuelist in einer Gruppe ist eine Kachel wie jede andere: Ein Klick schaltet sie ein und aus. Ihr Menü (Rechtsklick) bietet zusätzlich **„GO (nächster Cue)“**, **„Zurück (Cue davor)“** und **„Cuelist freigeben“**. Um sie bequem mit GO zu bedienen, legen Sie sie auf ein [Playback](playbacks.md).

### Rhythmus einer Szene mit Schritten

Szenen mit Schritten (Sequenzen und Chaser) und Cuelists folgen dem Rhythmus, in dem Sie ihre Kachel drücken:

- Ein Klick startet sie oder gibt sie frei, wie immer.
- Drücken Sie bei laufender Szene ihre Kachel **mehrmals hintereinander** im Takt der Musik, mit weniger als 2 Sekunden zwischen den Drücken. Jeder Druck geht zum nächsten Schritt (bei einer Cuelist gibt er ein GO), und die Szene läuft danach von selbst in diesem Rhythmus weiter: ein Schritt auf jeden Schlag, statt ihrer eigenen Zeiten. Der Rhythmus ist der Durchschnitt der letzten Drücke.
- Es zählen nur Drücke bei laufender Szene: Der Druck, der sie startet, beginnt die Zählung. So bekommt keine Szene einen Rhythmus, wenn Sie in einer exklusiven Gruppe die Szene wechseln und zurückkehren.
- Nach einer Pause von mehr als 2 Sekunden gibt ein Klick sie frei. Um sie direkt nach dem Vorgeben des Rhythmus freizugeben, warten Sie 2 Sekunden.
- Abgespielt wird der Rhythmus, den Sie vorgegeben haben, auch bei gesetztem **Live-Tempo**; ändern Sie danach das Tempo, wird der Rhythmus mit ihm schneller oder langsamer. In Pause geben die Drücke keine Schritte.
- Der Rhythmus wird mit der Szene gespeichert und gehört zu ihr: Sie folgt ihm auch, wenn sie in einer Superszene oder auf einem Playback läuft. In ihren [Eigenschaften](directo.md#szeneneigenschaften) (und unter [Szenen](escenas.md#wie-eine-szene-läuft)) können Sie ihn eintippen oder **„Entfernen“** drücken, damit sie zu ihren Zeiten zurückkehrt.

Das funktioniert genauso mit ihren Tasten in der [SHOW](show.md), mit ihren Zuordnungen von Controllern und Tasten im Verhalten „Umschalten“ und mit der [Fernsteuerung](remoto.md).

#### GO und Stop

Die Tasten **GO** und **Stop** in der oberen Leiste (und im Bereich „Rhythmus“ der SHOW) ändern, was ein Druck auf eine beliebige Szene bewirkt, für jeden, der sie drückt:

- **GO** an: Ein Druck auf eine Szene **startet** sie; bei einer schon laufenden gibt jeder Druck einen Schritt (bei einer Cuelist ein GO), und mehrere hintereinander geben ihren Rhythmus vor. Sie wird nie gestoppt, auch wenn Sie länger als 2 Sekunden nicht drücken: So geben Sie das Tempo in Ruhe vor.
- **Stop** an: Ein Druck auf eine Szene **stoppt** sie. Eine Szene, die nicht läuft, bleibt, wie sie ist.
- Beide aus (drücken Sie die eingeschaltete Taste erneut): wie immer, ein Klick startet die Szene oder gibt sie frei.

Flash-Szenen funktionieren weiter, solange Sie sie gedrückt halten, in jedem Modus.

Während sie läuft, trägt jede Kachel einer Szene mit Schritten oder einer Cuelist unten einen zweigeteilten **Rhythmusbalken**: Bei jedem Schritt leuchtet erst die eine, dann die andere Hälfte auf (bei einer Cuelist bei jedem Cue). So sehen Sie, in welchem Rhythmus jede läuft. Ihn tragen auch ihre SHOW-Tasten, ihre Playback-Streifen und ihre Tasten auf dem Smartphone.

## Master

- **Gruppen-Master**: Der Schieberegler „Master“ unten in jeder Spalte skaliert die Intensität aller ihrer Szenen.
- **Grand Master**: Der „GM“ in der oberen Leiste skaliert die ganze Ausgabe. Unter Optionen legen Sie fest, ob er nur auf die Intensitätskanäle wirkt oder auf alle.
- **Live-Dimmer**: Jede Szene hat außerdem ihren eigenen Dimmer, in ihren Eigenschaften, über dem Gruppen-Master.

Die Master und der Grand Master wirken nicht auf Kanäle, bei denen die Option „Dimmer“ in den [Grenzen des Fixtures](patch.md#grenzen) ausgeschaltet ist.

## Szeneneigenschaften

Klicken Sie mit der rechten Maustaste auf eine Kachel und wählen Sie **„Eigenschaften…“**.

| Feld | Was es tut |
|---|---|
| **Name** | Der Name der Szene. |
| **Einblendung** / **Ausblendung** | Ihre Zeiten, in Sekunden. Hat die Gruppe eine Überblendzeit gesetzt, wird diese verwendet. |
| **Priorität (0-10)** | Welche Szene gewinnt, wenn zwei denselben Kanal steuern (siehe [Prioritäten](directo.md#prioritäten)). |
| **Beim Start freigeben** | Was beim Start gestoppt wird: [Freigabemodi](directo.md#freigabemodi-und-schutz). |
| **Geschützt vor Freigaben durch** | Welche Freigaben sie ignoriert. |
| **Flash (läuft, solange gedrückt)** | Macht sie zur Flash-Szene. |
| **Live-Dimmer** | Ihre Intensität, über dem Gruppen-Master. |
| **Tap-Rhythmus** | Schritte und Cuelists: der mit ihrer Kachel vorgegebene [Rhythmus](directo.md#rhythmus-einer-szene-mit-schritten), in BPM. Tippen Sie ihn ein oder drücken Sie „Entfernen“, um zu ihren Zeiten zurückzukehren. Wirkt sofort. |
| **Live-Tempo** | Von ×0,1 bis ×10 (Szenen, die sich bewegen: Schritte, FX, Cuelists…). |
| **Live-Phase** / **Live-Größe** | Die Phase (0–359°) und die Größe (0–200 %) der FX. |
| **Überblendtempo** | Beschleunigt oder verlangsamt ihre Überblendungen, von ×0,1 bis ×10. |

Die Live-Schieberegler wirken sofort, während Sie sie bewegen, und ändern, wie die Szene in dieser Gruppe läuft, nicht die aufgenommene Szene. Die übrigen Felder werden mit **„Anwenden“** übernommen.

Dasselbe Menü der Kachel enthält **„Unter Szenen öffnen“**, **„Im Programmer bearbeiten“** (statische Szenen) und das Untermenü **„Priorität: N“**, um die Priorität zu ändern, ohne die Eigenschaften zu öffnen.

## Prioritäten

Jede Szene hat eine Priorität von 0 bis 10; neue Szenen bekommen die, die Sie unter [Optionen](opciones.md#wiedergabe) festlegen (standardmäßig 5).

- Die Szene mit der **höheren Priorität bestimmt alle Kanäle, die sie berührt**, Nullen eingeschlossen: Eine rote Szene mit Priorität 8 über einer blauen mit Priorität 5 ergibt Rot, nicht Magenta.
- Bei **gleicher Priorität** werden die Intensitätskanäle (Dimmer, RGB…) gemischt, und der höchste Wert gewinnt; bei allen anderen Kanälen gewinnt die zuletzt gestartete Szene.
- Ändern Sie die Priorität einer laufenden Szene, gilt das sofort.
- Wird eine Szene mit höherer Priorität mit Überblendung freigegeben, blendet sie zu dem über, was darunter liegt.
- **Der Programmer hat Vorrang vor allem**, Intensitäten eingeschlossen. Unter Optionen können Sie die Priorität der Live-Änderungen senken.

## Freigabemodi und Schutz

**Beim Start freigeben** legt fest, welche Szenen gestoppt werden, wenn diese startet:

- **„Gruppenstandard“**: in einer exklusiven Gruppe die anderen ihrer Gruppe; in einer Mischen-Gruppe keine.
- **„Keine“**: stoppt nichts.
- **„Die eigene Gruppe“**: die anderen Szenen ihrer Gruppe.
- **„Alle Szenen“**: alle anderen.
- **„Die anderen Gruppen“**: die Szenen der anderen Gruppen.

**Geschützt vor Freigaben durch** lässt die Szene diese Freigaben ignorieren:

- **„Keine“**: Sie wird wie immer freigegeben.
- **„Alles (nur manueller Stopp)“**: Nur Sie selbst stoppen sie.
- **„Die eigene Gruppe“**: ignoriert die Freigaben ihrer Gruppe.
- **„Die anderen Gruppen“**: ignoriert die Freigaben aus anderen Gruppen.

> **Tipp:** Geben Sie einer Stimmungsszene den Schutz „Alles (nur manueller Stopp)“ und starten Sie darüber Szenen mit „Alle Szenen“: Die Stimmung bleibt.

## Pause, freigeben und stoppen

- **„Pause“** friert alles ein, was läuft. **„Fortsetzen“** setzt es fort.
- **„Gruppen freigeben“** stoppt die Szenen aller Gruppen; das Quadrat jeder Spalte nur die dieser Gruppe.
- **„Alles stoppen“** (Esc) in der oberen Leiste stoppt alles, was läuft, auch außerhalb der Gruppen. Den Programmer leert es nicht.
- **„Blackout“** (Strg+B) schaltet die Ausgabe auf Schwarz, ohne etwas zu stoppen.

## Mischer

Der Mischer wirkt auf die Ausgabe, über jeder Szene und über den Live-Änderungen. Drücken Sie **„Mischer“**, um ihn zu sehen: Es gibt einen Kanalzug für **„Alle Fixtures“** und einen für jede [Fixture-Gruppe](patch.md#fixture-gruppen).

Jeder Kanalzug hat:

- Einen **Dimmer-Fader** mit seinem Prozentwert.
- **„Voll“**: die Gruppe voll. **„Blackout“**: die Gruppe auf Schwarz.
- **„Farbton“** und **„Sättigung“**: tönen die Farbe. **„Farbe zurücksetzen“** entfernt die Tönung.
- **„Stroboskop“** und sein Tempo, von 0,5 bis 20 Hz.
- **„Flash“**: voll, solange gedrückt. **„Flash exkl.“**: diese Gruppe voll und der Rest schwarz, solange gedrückt.
- **„Voll exkl.“**: diese Gruppe voll und der Rest schwarz.

Strg+Klick auf eine Flash-Taste lässt sie bis zum nächsten Klick eingerastet. **„Mischer zurücksetzen“** setzt alles auf den Normalzustand zurück. Der Rand eines Kanalzugs wird bei einem Flash oder Voll orange und bei Blackout rot.

> **Tipp:** Der Mischer kann auch ein Bereich der [SHOW](show.md) sein, und seine Fader und Tasten lassen sich einem MIDI-Controller [zuordnen](mapeos.md).

## Bedienelemente in der SHOW und auf Controllern

Die Szenenkacheln, die Master der Gruppen, die Tasten ◀ ▶ und Freigeben jeder Gruppe, die Tasten im Kopf (◀ ▶, „Pause“ und „Gruppen freigeben“) und die Bedienelemente des Mischers:

- Mit **Alt+Klick** werden sie auf die gerade geöffnete SHOW-Seite kopiert (siehe [Show](show.md#jedes-bedienelement-in-die-show-holen)).
- Im Lernmodus lassen sie sich einem Controller zuordnen (siehe [Zuordnungen](mapeos.md#lernmodus)).

# Live

![Die Seite Live](../img/directo.png)

**Live** (F1) ist die Seite, auf der Sie während der Show Szenen starten. Die Szenen sind in **Gruppen** geordnet, also in Spalten, die parallel arbeiten: Jede Gruppe hat ihre Szenen, ihren Modus (exklusiv oder mischen), ihre Überblendzeit und ihren Master.

Im Kopf der Seite finden Sie:

- **◀ ▶**: vorherige oder nächste Szene in jeder laufenden Gruppe (auch mit Strg+← und Strg+→).
- **„Pause“** / **„Fortsetzen“**: friert alles ein, was läuft, und setzt es fort.
- **„Gruppen freigeben“**: stoppt die Szenen aller Gruppen.
- **„Gruppen einklappen“** / **„Gruppen ausklappen“**: klappt alle Spalten auf einmal ein, um alle zu sehen; sind schon alle eingeklappt, klappt es sie aus.
- **„Mischer“**: wechselt zur Ansicht des [Mischers](directo.md#mischer). Ein orangefarbener Punkt zeigt an, dass der Mischer die Ausgabe verändert.
- **„Neue Gruppe“**: fügt eine Spalte hinzu.

## Gruppen

### Gruppen erstellen und organisieren

1. Drücken Sie **„Neue Gruppe“** (oder das Feld „Gruppe hinzufügen“ am Ende der Spalten).
2. Klicken Sie auf den Namen der Spalte, um ihn zu ändern (zum Beispiel `Wash`, `Spots`, `Stroboskope`, `Effekte`), und drücken Sie Eingabe.

Die Zahnradtaste jeder Spalte öffnet ihr Menü: **„Farbe ändern“** (wechselt zur nächsten Farbe), **„Nach links“**, **„Nach rechts“** und **„Gruppe löschen“**. Sie können die Gruppe auch am Griff links neben ihrem Namen ziehen und auf einer anderen Gruppe ablegen, um deren Platz einzunehmen.

Der Pfeil **‹** neben dem Namen **klappt** die Gruppe **ein**: Sie wird zu einem schmalen Streifen mit ihrem Namen und der Zahl der laufenden Szenen, um Platz für die anderen zu schaffen. Ihre Szenen laufen weiter. Der Pfeil **›** auf dem Streifen klappt sie wieder auf. Das wird mit der Show gespeichert. Um alle auf einmal ein- oder auszuklappen, nutzen Sie **„Gruppen einklappen“** / **„Gruppen ausklappen“** in der Kopfzeile.

### Szenen hinzufügen und ordnen

1. Drücken Sie **„Szenen hinzufügen“** am Ende der Spalte.
2. Klicken Sie in der Liste auf die gewünschten Szenen (sie werden mit ✓ markiert). Sie können nach Name oder Ordner suchen und sie nach Ordnern oder Gruppen anzeigen.
3. Drücken Sie **„N hinzufügen“**. Ein Doppelklick auf eine Szene fügt sie direkt hinzu.

Zum Umordnen fahren Sie mit der Maus über eine Kachel und ziehen sie an ihrem Griff (rechts auf der Kachel): Legen Sie sie auf einer anderen Szene ab, um sie davor einzufügen, oder in der Lücke der Spalte, um sie ans Ende zu setzen. Sie können sie in einer **anderen Gruppe** ablegen, dann wandert sie mit ihren Optionen dorthin. Im Menü der Kachel (Rechtsklick) gibt es außerdem **„Nach oben“**, **„Nach unten“** und **„Aus der Gruppe entfernen“**.

> **Tipp:** Beim Aufnehmen unter Programmieren können Sie die Szene mit „Zur Gruppe hinzufügen“ direkt in eine Gruppe legen. Siehe [Aufnehmen](programar.md#aufnehmen).

## Szenen starten

Klicken Sie auf eine Kachel, um die Szene zu starten (mit **GO**, dem Modus, in dem das Programm startet). Um sie freizugeben, drücken Sie das Quadrat im Kopf der Gruppe („Diese Gruppe freigeben“) oder wechseln Sie zu **Stop** und drücken Sie sie. Die laufende Kachel färbt sich in der Farbe der Gruppe und hat einen pulsierenden Punkt.

Auf jeder Kachel sehen Sie:

- Den **Namen** und darunter den **Typ** der Szene. Bei einer laufenden Cuelist den aktuellen Cue („Q 3 · Strophe“). Dahinter den [Tap-Rhythmus](directo.md#rhythmus-einer-szene-mit-schritten), falls vorhanden („120 BPM“).
- **P7**, **P2**…: die Priorität, nur wenn sie nicht die normale ist (5). In Orange, wenn sie höher als 5 ist.
- Einen **Blitz**: Es ist eine Flash-Szene.
- Einen **zweigeteilten Balken** unten: eine laufende Szene mit Schritten, Cuelist oder FX auf „BPM der Show“; er wechselt bei jedem Schritt (bei einem FX bei jedem Schlag), in ihrem Rhythmus.

### Exklusiv oder mischen

Die Taste jeder Spalte wechselt zwischen:

- **„Exklusiv“**: Es läuft immer nur eine Szene. Starten Sie eine andere, wird zur neuen übergeblendet. Die [Fader des SHOW](show.md#fader), die Szenen abspielen, sind die Ausnahme: Sie fügen ihre hinzu, Sie können also mehrere derselben Gruppe gleichzeitig hochziehen.
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

Eine Cuelist in einer Gruppe ist eine Kachel wie jede andere: Ein Klick startet sie (mit **Stop** stoppt er sie). Ihr Menü (Rechtsklick) bietet zusätzlich **„GO (nächster Cue)“**, **„Zurück (Cue davor)“** und **„Cuelist freigeben“**. Um sie bequem mit GO zu bedienen, legen Sie sie auf ein [Playback](playbacks.md).

### Rhythmus einer Szene mit Schritten

Szenen mit Schritten (Sequenzen und Chaser), Cuelists und FX-Szenen mit FX auf „BPM der Show“ folgen dem Rhythmus, in dem Sie ihre Kachel drücken:

- Ein Klick startet sie (mit **GO**, dem Modus, in dem das Programm startet; siehe unten).
- Drücken Sie bei laufender Szene ihre Kachel **mehrmals hintereinander** im Takt der Musik, mit weniger als 2 Sekunden zwischen den Drücken. Jeder Druck geht zum nächsten Schritt (bei einer Cuelist gibt er ein GO), und die Szene läuft danach von selbst in diesem Rhythmus weiter: ein Schritt auf jeden Schlag, statt ihrer eigenen Zeiten. Der Rhythmus ist der Durchschnitt der letzten Drücke. Ein Druck, der etwas zu spät kommt, kurz nach dem Schritt dieses Schlags (weniger als ein halber Schlag), ist genau dieser Schritt: Er gibt keinen weiteren, er bringt nur den Rhythmus wieder in den Takt.
- Es zählen nur Drücke bei laufender Szene: Der Druck, der sie startet, beginnt die Zählung. So bekommt keine Szene einen Rhythmus, wenn Sie in einer exklusiven Gruppe die Szene wechseln und zurückkehren.
- Nach einer Pause von mehr als 2 Sekunden gibt ein Klick einen Schritt, ohne ihren Rhythmus zu ändern, und die Szene läuft weiter. Um sie zu stoppen, wechseln Sie zu **Stop** und drücken Sie sie.
- Abgespielt wird der Rhythmus, den Sie vorgegeben haben, auch bei gesetztem **Live-Tempo**; ändern Sie danach das Tempo, wird der Rhythmus mit ihm schneller oder langsamer. In Pause geben die Drücke keine Schritte.
- Der Rhythmus wird mit der Szene gespeichert und gehört zu ihr: Sie folgt ihm auch, wenn sie in einer Superszene oder auf einem Playback läuft. In ihren [Eigenschaften](directo.md#szeneneigenschaften) (und unter [Szenen](escenas.md#wie-eine-szene-läuft)) können Sie ihn eintippen oder **„Entfernen“** drücken, damit sie zu ihren Zeiten zurückkehrt.
- Bei einer **FX-Szene** ist es der Rhythmus ihrer FX auf **„BPM der Show“**: Der erste Druck einer Reihe startet ihren Zyklus neu (wie beim Starten), und jeder weitere bringt sie zum nächsten Schlag, einen Zyklus weiter mal ihrem Multiplikator (ein Lauflicht geht zum nächsten Fixture; bei ×¼ einen Viertelzyklus weiter). Mehrere hintereinander geben ihnen einen Zyklus auf jeden Schlag (mal ihrem Multiplikator) statt der BPM der Show. FX mit einem eigenen Tempo ändern sich nicht. Ohne Tap-Rhythmus oder nach „Entfernen“ folgen sie den BPM der Show. Wenn Sie „BPM der Show“ bei allen ihren FX ausschalten, verliert die Szene ihren Tap-Rhythmus.

Das funktioniert genauso mit ihren Tasten in der [SHOW](show.md), mit ihren Zuordnungen von Controllern und Tasten im Verhalten „Umschalten“ und mit der [Fernsteuerung](remoto.md).

#### GO und Stop

Die Tasten **GO** und **Stop** in der oberen Leiste (und im Bereich „Rhythmus“ der SHOW) legen fest, was ein Druck auf eine beliebige Szene bewirkt, für jeden, der sie drückt. Eine der beiden ist immer eingeschaltet: Das Programm startet immer in **GO**, und ein Druck auf die andere wechselt den Modus:

- **GO** an: Ein Druck auf eine Szene **startet** sie; bei einer schon laufenden gibt jeder Druck einen Schritt (bei einer Cuelist ein GO; bei einer FX-Szene startet der erste den Zyklus ihrer FX auf „BPM der Show“ neu, und die weiteren bringen sie zum nächsten Schlag), und mehrere hintereinander geben ihren Rhythmus vor. Sie wird nie gestoppt, auch wenn Sie länger als 2 Sekunden nicht drücken: So geben Sie das Tempo in Ruhe vor.
- **Stop** an: Ein Druck auf eine Szene **stoppt** sie. Eine Szene, die nicht läuft, bleibt, wie sie ist.
- Um die Szenen einer Gruppe freizugeben, ohne den Modus zu wechseln, drücken Sie das Quadrat in ihrem Kopf („Diese Gruppe freigeben“); **„Alles stoppen“** (Esc) stoppt alles.

Flash-Szenen funktionieren weiter, solange Sie sie gedrückt halten, in jedem Modus.

Während sie läuft, trägt jede Kachel einer Szene mit Schritten, einer Cuelist oder einer FX-Szene mit FX auf „BPM der Show“ unten einen zweigeteilten **Rhythmusbalken**: Bei jedem Schritt leuchtet erst die eine, dann die andere Hälfte auf (bei einer Cuelist bei jedem Cue; bei einer FX-Szene bei jedem Schlag, einem Zyklus dieser FX bei ×1). So sehen Sie, in welchem Rhythmus jede läuft. Ihn tragen auch ihre SHOW-Tasten, ihre Playback-Streifen und ihre Tasten auf dem Smartphone.

## Master

- **Gruppen-Master**: Der Schieberegler „Master“ unten in jeder Spalte skaliert die Intensität aller ihrer Szenen.
- **Grand Master**: Der „GM“ in der oberen Leiste skaliert das Licht der ganzen Ausgabe: die Dimmer und, bei Fixtures ohne Dimmer, deren Farben. Die Farbmischung (Cyan, Magenta, Gelb, Farbton und Sättigung) lässt er unberührt, ebenso die Farben eines Fixtures oder Kopfes mit eigenem Dimmer, die mit diesem Dimmer abnehmen. Unter Optionen legen Sie fest, ob er nur auf die Intensität wirkt oder auf alle Kanäle.
- **Live-Dimmer**: Jede Szene hat außerdem ihren eigenen Dimmer, in ihren Eigenschaften, über dem Gruppen-Master.

Die Master und der Grand Master wirken nicht auf Kanäle, bei denen die Option „Dimmer“ in den [Grenzen des Fixtures](patch.md#grenzen) ausgeschaltet ist. Ist es der Dimmer eines Fixtures, zählt er nicht mehr als dessen eigener Dimmer: Der Grand Master senkt dann seine Farben.

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
| **Tap-Rhythmus** | Schritte, Cuelists und FX-Szenen mit FX auf „BPM der Show“: der mit ihrer Kachel vorgegebene [Rhythmus](directo.md#rhythmus-einer-szene-mit-schritten), in BPM. Tippen Sie ihn ein oder drücken Sie „Entfernen“, um zu ihren Zeiten zurückzukehren (bei einer FX-Szene zu den BPM der Show). Wirkt sofort. |
| **Live-Tempo** | Von ×0,1 bis ×10 (Szenen, die sich bewegen: Schritte, FX, Cuelists…). |
| **Live-Phase** / **Live-Größe** | Die Phase (0–359°) und die Größe (0–200 %) der FX. |
| **Überblendtempo** | Beschleunigt oder verlangsamt ihre Überblendungen, von ×0,1 bis ×10. |

Die Live-Schieberegler wirken sofort, während Sie sie bewegen, und ändern, wie die Szene in dieser Gruppe läuft, nicht die aufgenommene Szene. Die übrigen Felder werden mit **„Anwenden“** übernommen.

Dasselbe Menü der Kachel enthält **„Unter Szenen öffnen“**, **„Im Programmer bearbeiten“** (statische Szenen) und das Untermenü **„Priorität: N“**, um die Priorität zu ändern, ohne die Eigenschaften zu öffnen.

## Prioritäten

Jede Szene hat eine Priorität von 0 bis 10; neue Szenen bekommen die, die Sie unter [Optionen](opciones.md#wiedergabe) festlegen (standardmäßig 5).

- Die Szene mit der **höheren Priorität bestimmt alle Kanäle, die sie berührt**, Nullen eingeschlossen: Eine rote Szene mit Priorität 8 über einer blauen mit Priorität 5 ergibt Rot, nicht Magenta.
- Bei **gleicher Priorität** werden die Intensitätskanäle (Dimmer, RGB…) gemischt, und der höchste Wert gewinnt; bei allen anderen Kanälen gewinnt die **letzte Änderung**: die zuletzt gestartete Szene oder der letzte GO einer Cuelist, der diesen Kanal geändert hat. Was eine Cuelist nur aus früheren Cues behält, holt sie sich nicht zurück.
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

Startet ein [Fader des SHOW](show.md#fader) sie, gibt sie nichts frei, egal in welchem Modus: Sie kommt hinzu. Mit ihrer Taste gibt sie wie gewohnt frei. Und solange dieser Fader oben ist, stoppt sie nichts außer ihn ganz herunterzuziehen.

**Geschützt vor Freigaben durch** lässt die Szene diese Freigaben ignorieren:

- **„Keine“**: Sie wird wie immer freigegeben.
- **„Alles (nur manueller Stopp)“**: Nur Sie selbst stoppen sie.
- **„Die eigene Gruppe“**: ignoriert die Freigaben ihrer Gruppe.
- **„Die anderen Gruppen“**: ignoriert die Freigaben aus anderen Gruppen.

> **Tipp:** Geben Sie einer Stimmungsszene den Schutz „Alles (nur manueller Stopp)“ und starten Sie darüber Szenen mit „Alle Szenen“: Die Stimmung bleibt.

## Pause, freigeben und stoppen

- **„Pause“** friert alles ein, was läuft. **„Fortsetzen“** setzt es fort.
- **„Gruppen freigeben“** stoppt die Szenen aller Gruppen; das Quadrat jeder Spalte nur die dieser Gruppe. 10 s lang startet [„Zurückholen, was lief“](directo.md#zurückholen-was-lief) wieder, was „Gruppen freigeben“ gestoppt hat (seine Taste oder ein Pad oder eine SHOW-Taste in „Alle Gruppen“); was in nur einer Gruppe (ihr Quadrat oder ein Pad oder eine SHOW-Taste dieser Gruppe) oder von einer Aktion einer Cue freigegeben wird, lässt sich nicht zurückholen.
- **„Alles stoppen“** (Esc) in der oberen Leiste stoppt alles, was läuft, auch außerhalb der Gruppen. Den Programmer leert es nicht. Weder „Alles stoppen“ noch „Gruppen freigeben“ stoppt die Szenen, die ein hochgezogener [Fader des SHOW](show.md#fader) hält: nur ihn ganz herunterzuziehen. Es entschärft auch die [Spezialeffekte](directo.md#geschützte-spezialeffekte), und 10 s lang können Sie [zurückholen, was lief](directo.md#zurückholen-was-lief). Mit [„Alles stoppen mit Ausblenden“](directo.md#alles-stoppen-mit-ausblenden) blenden die Funktionen aus, bevor sie stoppen.
- **„Blackout“** (Strg+B) schaltet die Ausgabe auf Schwarz, ohne etwas zu stoppen. Die Köpfe bewegen sich nicht: Pan, Tilt, Zoom, Fokus und die anderen Kanäle, die kein Licht sind, bleiben, wo sie sind. Er entschärft auch die Spezialeffekte. Um das Licht langsam herunterzunehmen und im Blackout zu enden, nutzen Sie ein [„Ausblenden auf Schwarz“](directo.md#ausblenden-auf-schwarz).

## Sicherheit während der Show

Damit ein Fehler oder ein Ausfall des Computers die Show nicht verdirbt: der [Show-Modus](directo.md#show-modus), die [geschützten Spezialeffekte](directo.md#geschützte-spezialeffekte), die [geparkten Kanäle](directo.md#geparkte-kanäle), der [Show-Stopp](directo.md#show-stopp), [Zurückholen, was lief](directo.md#zurückholen-was-lief), [Alles stoppen mit Ausblenden](directo.md#alles-stoppen-mit-ausblenden) und das [Ausblenden auf Schwarz](directo.md#ausblenden-auf-schwarz). Ihre Einstellungen stehen unter [Optionen › Sicherheit während der Show](opciones.md#sicherheit-während-der-show).

> **Wichtig:** Der Computer darf nie die einzige Sicherheit einer CO2-, Flammen-, Kaltfunken- oder Konfettimaschine sein. Stellen Sie die Maschinen und die DMX-Nodes so ein, dass sie sich bei Verlust des DMX-Signals abschalten (und nicht den letzten Wert halten), sehen Sie einen Not-Aus oder einen Totmannschalter in Hardware vor, wo die Maschine einen braucht, und halten Sie sich an die Vorschriften des Ortes und die Anweisungen des Herstellers. Hängt das Programm oder geht der Computer aus, kann keine Software eine Auslösung abschalten: Das kann nur die Technik.

### Show-Modus

Das **Schild** in der oberen Leiste, gleich nach dem Schloss, schaltet den **Show-Modus** mit einem Klick ein oder aus. Eingeschaltet ist es orange, und die zweite Zeile des Titels beginnt mit „Show-Modus ·“. Das Sperren der SHOW (auch beim Start mit „…und gesperrt“) schaltet ihn von selbst ein; das Entsperren schaltet ihn nicht aus. Bei gesperrter SHOW ist das Schild zu sehen, lässt sich aber nicht anklicken. Es ist eine Einstellung des Computers, nicht der Show.

Mit eingeschaltetem Show-Modus verlangen diese Aktionen, eine Taste 1,5 s gedrückt zu halten („Halten zum Schließen“, „Halten zum Öffnen“ oder „Halten zum Fortfahren“), auch ohne ungespeicherte Änderungen:

- das Programm schließen (das × des Fensters, Alt+F4, die Taskleiste) und ein Update installieren;
- **Neu** und **Öffnen** (auch Strg+N und Strg+O), die letzten Shows, die Beispiele und die Übungsshow der Anleitung;
- eine Show aus dem Windows-Explorer öffnen.

Zu frühes Loslassen, **„Abbrechen“** oder Esc brechen ab; Esc stoppt in diesem Fenster nie die Show. Leertaste oder Eingabetaste gedrückt zu halten geht auch (die Tastenwiederholung zählt nicht). Danach kommt bei ungespeicherten Änderungen wie immer „Änderungen speichern?“. Wenn Windows herunterfährt oder Sie sich bei eingeschaltetem Show-Modus abmelden, kann die Frage das Herunterfahren aufhalten: Schließen Sie das Programm vorher.

Wechselt der Laptop im Show-Modus auf den Akku, erscheint auf allen Seiten (auch gesperrt und im Vollbild) ein roter Balken „Akkubetrieb: Schließen Sie das Ladegerät an“, bis das Ladegerät wieder dran ist; bei 20 % oder weniger kommt „Akku bei N %“ dazu. Der Akku wird alle 30 s und bei jeder Energiemeldung von Windows geprüft. Ohne Show-Modus sagt es eine Meldung einmal (und noch einmal, wenn er fast leer ist), und die [Optionen](opciones.md#sicherheit-während-der-show) zeigen es an.

Hat der Windows-Energiesparplan das selektive USB-Energiesparen eingeschaltet, warnt beim Start ein blauer Balken, dass das DMX-Interface stoppen kann, mit **„So schalten Sie es aus“** (öffnet [Selektives USB-Energiesparen](opciones.md#selektives-usb-energiesparen)) und einem ×, das ihn bis zum nächsten Start ausblendet. Das Programm liest diese Windows-Einstellungen nur: Es ändert sie nie.

### Geschützte Spezialeffekte

Ein **geschützter Spezialeffekt** ist ein Fixture, das nicht versehentlich auslösen kann. CO2-, Flammen-, Kaltfunken- und Konfettimaschinen sind es von selbst, und jedes Fixture kann es von Hand werden (siehe [Patch › Geschützte Spezialeffekte](patch.md#geschützte-spezialeffekte)). Sie tragen ein Schild in der Liste und im Plan des Patch.

- **Entschärft** (so startet das Programm immer) senden alle ihre Kanäle 0, egal was läuft.
- **Scharf geschaltet** gehen sie nur aus **Szenen** (auch Flash), **Cuelists**, **Chasern**, **Sequenzen**, **Superszenen** und den Grundwerten von **FX-Szenen** hinaus. So lösen Sie sie mit einer SHOW-Taste im Flash-Modus (sie löst aus, solange Sie sie halten), einer Kachel unter Live, einem Pad der APC, einem Playback oder dem [Smartphone](remoto.md#sicherheit-auf-dem-smartphone) aus.
- **Nie**, auch nicht scharf geschaltet, lösen sie aus: der Programmer, Identifizieren, Hervorheben, der Mischer (Full, Flash, Farbe, Strobe), die FX und ihre Makros, Move FX, Colour FX und Plan-Mappings, Curve FX, Skripte und auch kein DMX-Eingang, der zum Ausgang durchgereicht wird. Der Grand Master skaliert sie nicht.
- Die [Grenzen](patch.md#grenzen) zählen nur während der Auslösung: In Ruhe geht immer 0 hinaus. Invertieren Sie die Kanäle einer Effektmaschine nicht: Ein invertierter Auslösekanal löst nur mit einem Wert aus, der invertiert einschaltet.
- Ihre Pan- und Tilt-Kanäle (die eines Flammenkopfs) gehen nur scharf geschaltet und aus diesen Funktionen hinaus, ohne Höchstzeit; entschärft auf 0. Alle anderen außer dem Sicherheitskanal lösen aus, und für sie gilt die Höchstzeit (siehe [Patch › Geschützte Spezialeffekte](patch.md#geschützte-spezialeffekte)).

#### Effekte scharf schalten

Die Taste **„Effekte scharf schalten“** (mit einer Flamme) steht in der oberen Leiste neben „Blackout“, nur wenn die Show geschützte Effekte hat, auch bei gesperrter SHOW. Ein Klick schaltet sie scharf, ein weiterer entschärft sie. Scharf geschaltet wird die Taste rot und zeigt die Zeit, bis sie sich von selbst entschärfen („SCHARF · 9:59“), und ihr Rand pulsiert, während einer auslöst. In einem schmaleren Fenster zeigt sie „Scharf schalten“ oder nur die Flamme: Unter 1450 px Breite zeigen „Effekte scharf schalten“, „Blackout“ und „Alles stoppen“ nur ihr Symbol, und unter etwa 1200 px verschwindet die Zahl der laufenden Funktionen, und der Grand Master wird kürzer.

Scharf geschaltet erscheint auf allen Seiten (auch gesperrt und im Vollbild) ein großer roter Balken **„EFFEKTE SCHARF“** mit dem, was auslöst („Löst aus: CO2 1“), oder dem, was auf das Loslassen wartet, und einer Taste **„Entschärfen“**.

Der Pfeil **▾** daneben zeigt, ob sie scharf sind und wie lange es dauert, bis sie sich von selbst entschärfen, **„Lösen gerade aus“**, **„Warten auf das Loslassen“** (lassen Sie seine Szene, seine Taste oder seinen Fader los, auf 0, dann kann er auslösen), **„Letzte Auslösungen“** (Name · Uhrzeit · Sekunden · wie sie endete: losgelassen, bei der Höchstzeit abgeschaltet, entschärft…), **„Protokoll öffnen“** und **„Einstellungen jedes Effekts: Patch“** (diese beiden nicht bei gesperrter SHOW).

Sie lassen sich nicht scharf schalten, wenn kein Fixture geschützt ist, bei eingeschaltetem Blackout („Blackout ist an: Schalten Sie ihn aus, um die Spezialeffekte scharf zu schalten“) und bei eingeschaltetem Show-Stopp. Scharf schalten können Sie sie auch mit einer [SHOW-Taste](show.md#aktionen-einer-taste), einer [Zuordnung](mapeos.md#steuert) oder dem [Smartphone](remoto.md#sicherheit-auf-dem-smartphone).

#### Höchstzeit und Mindestwartezeit

- Eine Auslösung dauert, solange ihre Szene, ihre Taste oder ihr Fader sie gibt, höchstens ihre **Höchstzeit** (Standard: CO2 3 s, Flammen 2 s, Funken 5 s, Konfetti 2 s). Dann wird sie abgeschaltet, auch wenn Sie weiter drücken, und kommt erst wieder, wenn Sie loslassen.
- Zwischen zwei Auslösungen liegt eine **Mindestwartezeit** (Standard 1 s) ab dem Loslassen. Ein Druck während der Wartezeit löst nicht später aus: Lassen Sie los und drücken Sie erneut.
- Scharf schalten, während eine Szene oder ein Fader dem Effekt schon einen Wert gibt, löst ihn nicht aus: Er wartet, bis Sie loslassen. Entschärfen, während einer auslöst, schaltet ihn ab, auch wenn Sie sofort wieder scharf schalten (er kommt erst nach dem Loslassen zurück); ein Steuerelement, das nur scharf schaltet, tut nichts, wenn sie schon scharf sind.
- Der **Sicherheitskanal**, wenn die Maschine einen hat, sendet seine Werte nur, während der Effekt scharf ist und auslöst; sonst 0. Allein löst er die Maschine nie aus.
- Sendet das Programm keine Frames mehr (weil es sehr beschäftigt ist), schaltet jedes Universum eine Auslösung trotzdem zu ihrer Zeit ab, höchstens 40 ms zu spät.

#### Wann sie entschärft werden

Sie entschärfen sich von selbst:

- mit **Alles stoppen** (Esc, seine Taste, ein Pad, eine SHOW-Taste oder das Smartphone) und mit dem **Blackout** (Strg+B, seine Taste, eine SHOW-Taste, ein Pad oder das Smartphone; nicht dem „Blackout“ eines Kanalzugs des Mischers);
- mit dem **Show-Stopp**, einem **Ausblenden auf Schwarz** (bei seinem Beginn) und **Zurückholen, was lief**;
- wenn Sie **eine andere Show** öffnen oder beginnen, und wenn Sie das Programm **schließen**;
- nach einigen **Minuten ohne Auslösung** (Standard 10: „Unbenutzte Spezialeffekte entschärfen nach“ in den [Optionen](opciones.md#sicherheit-während-der-show)), nie während einer auslöst;
- wenn der letzte geschützte Effekt entfernt oder nicht mehr geschützt wird, und **von Hand**.

Eine Meldung sagt es („Spezialeffekte entschärft: Alles stoppen“). Sie werden nie scharf gespeichert. Ein SHOW-Fader, der eine Szene mit einem CO2 hält, hält sie nach Esc weiter, aber das CO2 bleibt auf 0, bis Sie den Fader ganz herunterziehen.

#### Das Protokoll der Auslösungen

Jede Sitzung schreibt eine Datei in `%LOCALAPPDATA%\DMXTotalControl\DMX Total Control\effects-log` („Protokoll öffnen“ öffnet den Ordner): jede Auslösung mit ihrer Uhrzeit, der Nummer und dem Namen des Fixtures, wie lange sie dauerte und wie sie endete (losgelassen, bei der Höchstzeit abgeschaltet, entschärft, Show-Stopp, geparkt, entfernt), und wann die Effekte scharf geschaltet und entschärft wurden, und warum.

> **Nicht mit echten Geräten getestet:** Nichts davon wurde mit CO2-, Flammen-, Funken- oder Konfettimaschinen getestet, mit ihren Sicherheitskanälen, mit Art-Net- oder sACN-Nodes oder damit, was sie bei Verlust des DMX-Signals tun. Probieren Sie es mit Ihren Geräten aus, ohne Publikum und mit ungeladener Maschine, bevor Sie es in einer Show einsetzen.

### Geparkte Kanäle

Ein **geparkter** Kanal geht immer mit einem festen Wert hinaus, über den Szenen, dem Programmer, dem Mischer, dem Grand Master und Alles stoppen: der Hazer die ganze Nacht auf 30 %, ein defekter Kopf aus und still. Esc und Blackout entparken ihn nie; beim Blackout gehen seine Intensitäten aus, außer mit „Auch im Blackout“. Geparkt wird im [Patch](patch.md#kanäle-parken) und unter [Programmieren](programar.md#parken). Öffnen Sie eine Show mit geparkten Kanälen, gelten sie sofort, und eine Meldung sagt, welche („2 geparkte Fixtures: HAZER, Spot 3 (Patch)“).

### Show-Stopp

Der **Show-Stopp** ist ein Sicherheitslicht, um die Show anzuhalten und den Leuten Licht zu geben: Über allem setzt er offenes Weiß auf die Lichter, schaltet Stroboskope und Laser dunkel, hält die Köpfe still und entschärft die Spezialeffekte.

Um ihn **auszulösen oder aufzuheben**, drücken Sie **zweimal die Pause-Taste** (innerhalb von 2 s; nicht in einer Show, die sie zuordnet, siehe [Zuordnungen](mapeos.md#tastatur)), oder **halten Sie 1 s lang** eine SHOW-Taste „Show-Stopp (1 s halten)“, ein [zugeordnetes](mapeos.md#steuert) Pad oder eine zugeordnete Taste oder die Hand in der Leiste des [Smartphones](remoto.md#sicherheit-auf-dem-smartphone). Ein kürzerer Druck tut nichts, und ein einziger Druck auf die Pause-Taste warnt nur („Show-Stopp: Erneut Pause drücken, um ihn auszulösen“). Während Sie halten, füllt sich ein oranger Balken „Show-Stopp: weiter halten…“. Ist er an, erscheint auf allen Seiten (auch gesperrt und im Vollbild) ein großer roter Balken **„SHOW-STOPP“** mit **„Halten zum Aufheben“** (eine Sekunde halten). Verliert das Programm die Tastatur, während Sie eine zugeordnete Taste halten (ein anderes Fenster, eine Meldung von Windows), wird sie losgelassen, und nichts ändert sich.

Solange er an ist:

- werden die Spezialeffekte entschärft („…entschärft: Show-Stopp“) und lassen sich nicht scharf schalten („Der Show-Stopp ist an: Heben Sie ihn auf, um die Spezialeffekte scharf zu schalten“);
- gehen die Lichter (Moving Heads, Scanner, PARs und Washes, LED-Leisten, Blinder und Dimmer) der gewählten Gruppen auf **offenes Weiß** im gewählten Pegel: der Dimmer auf den Pegel, Rot, Grün, Blau und Weiß voll (ohne Dimmer die Farben auf den Pegel), CMY, Amber und UV auf 0, der Shutter offen, das Farbrad auf Weiß oder offen und die eingebauten Programme auf „keine Funktion“;
- gehen die Stroboskope aus (ihre Intensitäten auf 0, oder alle ihre Kanäle, wenn sie keine haben) und die Laser auf allen Kanälen auf 0 (ein DMX-Laser ist bei 0 aus: Prüfen Sie Ihren);
- öffnen sich alle Shutter (kein Strobe; ein Strobe-Kanal „langsam bis schnell“ geht auf 0), und das Strobe des Mischers stoppt;
- bleiben die Moving Heads, wo sie sind (Pan und Tilt, auch die feinen);
- bleiben Nebel, Hazer, Ventilatoren, Seifenblasen, Schnee, andere Effekte, Gobos, Prismen und Zoom, wie sie sind, und die geschützten Effekte auf 0.

Er gewinnt gegen den Programmer, den Grand Master, den Blackout (der Rest bleibt schwarz) und geparkte Kanäle: Ein aus geparktes Fixture leuchtet auch, und ein geparkter Hazer bleibt geparkt. Die [Grenzen](patch.md#grenzen) gelten weiter (ein invertierter Dimmer, ein Maximum). Welche Fixtures er beleuchtet und mit welchem Pegel, wählen Sie unter [Optionen › Show-Stopp](opciones.md#show-stopp).

- **Alles stoppen** (Esc) während des Show-Stopps stoppt die Funktionen darunter, und der Show-Stopp bleibt.
- Wenn Sie ihn **aufheben**, ist die Show so, wie ihre Funktionen jetzt spielen (sie liefen darunter weiter), und das Strobe des Mischers kommt zurück. Die Effekte bleiben entschärft: Schalten Sie sie von Hand scharf; ein gehaltenes CO2 muss vorher losgelassen werden.
- **Neu**, **Öffnen** und das Schließen des Programms heben ihn auf. Er wird nie eingeschaltet gespeichert: Lösen Sie ihn nach einem unerwarteten Beenden oder einem Neustart erneut aus.
- Das Effektprotokoll der Sitzung schreibt „Show-Stopp an“ und „Show-Stopp aufgehoben“.

> **Nicht mit echten Geräten getestet:** das offene Weiß und die Shutter-Werte echter Fixtures, ob ein Laser bei 0 wirklich dunkel ist, die Pause-Taste echter Tastaturen, die LEDs der APC und das Smartphone über ein echtes WLAN.

### Zurückholen, was lief

Nach **Alles stoppen** (Esc, seine Taste, ein Pad, eine SHOW-Taste oder das Smartphone) oder **„Gruppen freigeben“** (alle Gruppen, nicht nur eine) sagt ein blauer Balken unter der oberen Leiste, was gestoppt wurde („Gestoppt: 4 Szenen und 1 Cuelist · Zurückholen, was lief (Strg+Z)“; „Freigegeben: …“ nach „Gruppen freigeben“), mit der Taste **„Zurückholen“**, einem Streifen, der sich in 10 Sekunden leert, und einem ×, das ihn bis zum nächsten Alles stoppen ausblendet. Er erscheint auch bei gesperrter SHOW. **Strg+Z** tut dasselbe unter Live und in der SHOW (nicht, während Sie sie bearbeiten oder schreiben), und der Balken sagt „(Strg+Z)“ nur, wo es funktioniert. „Zurückholen“ übergeht ein Tippen in seiner ersten halben Sekunde (ein zweites Paniktippen auf Alles stoppen).

Zurück kommen, mit einer Blende von 0,5 s:

- die Szene, die jede Gruppe spielte;
- die Szenen, die aus SHOW-Tasten, aus Szenen oder aus Playbacks gestartet wurden;
- die Cuelists, auf dem Cue, auf dem sie waren, mit dem Pegel ihres Playbacks; eine pausierte Cuelist kommt pausiert zurück.

FX und Schritte beginnen ihren Zyklus neu, Funktionen in Beats nutzen ihre eigene Blende, und „Timecode“-Cuelists folgen ihrem Timecode. Nicht zurück kommen: gehaltene Flashs (unter Live, in der SHOW, auf Pads oder in Playbacks), die Szenen, die ein SHOW-Fader hält (sie wurden nie gestoppt), der Programmer, der Mischer, der Grand Master, das Scharfschalten der Effekte, geparkte Kanäle und der Show-Stopp.

Es geht einmal, und eine Meldung sagt, was zurückkam („Zurückgeholt: 4 Szenen und 1 Cuelist“). Ein weiteres Alles stoppen innerhalb der 10 s kommt zum ersten dazu (nicht das zweite Esc während des Ausblendens von „Alles stoppen mit Ausblenden“); „Gruppen freigeben“ hält nur die Gruppen fest, und ein Alles stoppen danach kommt dazu. Inzwischen gelöschte Funktionen werden übersprungen, und eine neue oder geöffnete Show löscht es.

Zurückholen **schaltet die Spezialeffekte nie scharf**: Waren sie scharf (nach „Gruppen freigeben“, das nicht entschärft, oder wenn Sie sie nach Alles stoppen wieder scharf geschaltet haben), entschärft es sie zuerst („Spezialeffekte entschärft: Zurückholen, was lief“). Was mit einem CO2 zurückkommt, wartet, bis Sie loslassen, und bleibt auf 0, auch nach erneutem Scharfschalten.

### Alles stoppen mit Ausblenden

Mit **„Alles stoppen mit Ausblenden“** in den [Optionen](opciones.md#sicherheit-während-der-show) (0 bis 5 s; 0, sofort, wie bisher) blendet Alles stoppen die Funktionen in dieser Zeit aus. Szenen, die SHOW-Fader halten, und geparkte Kanäle bleiben; geschützte Effekte gehen sofort aus (sie werden entschärft). Während des Ausblendens hat die Taste „Alles stoppen“ einen Streifen und sagt „Blendet aus: erneut Esc stoppt sofort“: **Erneut Esc** stoppt alles sofort, auch was Sie während des Ausblendens gestartet haben. Danach können Sie zurückholen, was das erste Esc gestoppt hat; was Sie während des Ausblendens gestartet haben, kommt nicht zurück.

### Ausblenden auf Schwarz

Ein **„Ausblenden auf Schwarz“** (eine [SHOW-Taste](show.md#aktionen-einer-taste) oder eine [Zuordnung](mapeos.md#steuert), mit ihrer Zeit: Standard 3 s) nimmt die Intensitäten am Ausgang in dieser Zeit herunter und schaltet dann den Blackout ein, zum Beispiel am Ende eines Sets. Darunter ist das Licht wieder voll: Schalten Sie den Blackout aus, kommt es zurück.

- Es wirkt über dem Grand Master und auf geparkte Kanäle ohne „Auch im Blackout“.
- Es berührt keine geparkten Kanäle mit „Auch im Blackout“, kein Pan, kein Tilt und keine anderen Kanäle, die kein Licht sind, und keine geschützten Effekte.
- Bei seinem Beginn entschärft es die Spezialeffekte.
- Ein weiterer Druck oder der Blackout beenden es sofort. Bei schon eingeschaltetem Blackout tut es nichts.
- Während es ausblendet, hat die Taste „Blackout“ einen Streifen und sagt „Ausblenden auf Schwarz: am Ende kommt der Blackout (Blackout jetzt: sofort)“.

> **Nicht mit echten Geräten getestet:** der Show-Modus (Akku, wach gehaltener Computer, USB-Energiesparen), Zurückholen, Alles stoppen mit Ausblenden und Ausblenden auf Schwarz wurden nicht mit echten Laptops, DMX-Interfaces, APC-LEDs oder Effektmaschinen getestet.

## Mischer

Der Mischer wirkt auf die Ausgabe, über jeder Szene und über den Live-Änderungen. Drücken Sie **„Mischer“**, um ihn zu sehen: Es gibt einen Kanalzug für **„Alle Fixtures“** und einen für jede [Fixture-Gruppe](patch.md#fixture-gruppen).

Jeder Kanalzug hat:

- Einen **Dimmer-Fader** mit seinem Prozentwert.
- **„Voll“**: die Gruppe voll. **„Blackout“**: die Gruppe auf Schwarz, außer geschützten Spezialeffekten (keine Taste des Mischers schaltet sie aus oder entschärft sie).
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

Solange eine Zuordnung „Die ausgewählte Szene“ verwendet, oder im Lernmodus, hat die Kachel der ausgewählten Szene (die Sie zuletzt gedrückt haben) einen dünnen Balken an ihrem unteren Rand (siehe [Zuordnungen](mapeos.md#die-ausgewählte-szene)).

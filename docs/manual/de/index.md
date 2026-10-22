# Handbuch von DMX Total Control

DMX Total Control ist ein freies Programm, mit dem Sie DMX-Beleuchtung von einem Windows-Computer aus steuern. Es ist für den Live-Einsatz gedacht: Sie starten Szenen in Gruppen, programmieren schnell mit dem Programmer, fahren Cuelists mit GO auf den Playbacks und bauen sich, wenn Sie möchten, Ihren eigenen Touchscreen (die SHOW), den Sie auch vom Smartphone aus bedienen.

Mit DMX Total Control können Sie:

- Die Fixtures Ihres Rigs patchen, mit einer Bibliothek aus Tausenden von Definitionen, dazu die Open Fixture Library, der GDTF-Import und Ihre eigenen Fixtures.
- Den ausgewählten Fixtures Dimmer, Farbe, Position, Gobos und jeden anderen Kanal geben, mit Paletten, FX und einer Befehlszeile.
- Statische Szenen, Schritte, FX, Superszenen mit Zeitleiste und Audio sowie Cuelists mit Tracking aufnehmen.
- Sie unter Live in Gruppen starten, auf den Playbacks mit GO oder aus Ihrer SHOW.
- DMX über DMX-USB-Interfaces, Art-Net oder sACN ausgeben und alles über MIDI, OSC, DMX-Eingang oder die Tastatur steuern.

Shows werden in `.qxw`-Dateien gespeichert, demselben Format, das QLC+ verwendet. Sie können also mit QLC+ erstellte Shows öffnen. Ist das Programm schon geöffnet und öffnen Sie eine Show im Explorer (Doppelklick auf die Datei), öffnet sie sich im selben Fenster, das in den Vordergrund kommt (vorher wird nach den ungespeicherten Änderungen gefragt).

## So öffnen Sie dieses Handbuch

Drücken Sie **F12** oder die Taste **„?“** in der oberen Leiste. Das Handbuch öffnet sich rechts im Fenster, auf der Seite, die die Seite erklärt, auf der Sie gerade sind: Wenn Sie unter Programmieren sind, öffnet es sich bei [Programmieren](programar.md). Die Links führen zu anderen Seiten des Handbuchs; im Handbuch können Sie zur vorherigen Seite oder zu diesem „Inhalt“ zurückkehren. Es öffnet sich auch über Optionen › Hilfe › **„Handbuch öffnen“**.

## Die Anleitung und die Beispielshows

- **Die Einstiegsanleitung**: eine Karte unten rechts, die Sie in fünf Schritten durch die Seiten führt (die Fixtures, ein Look, eine Szene, Live und die Playbacks). Sie erscheint beim ersten Start des Programms; danach über Optionen › Hilfe › **„Anleitung starten“**. Die Seiten lassen sich benutzen, während sie offen ist. Siehe [Erste Schritte](primeros-pasos.md).
- **Die Beispielshows**: fertige Shows zum Öffnen und Ausprobieren. Sie finden sie im Pfeil **▾** neben „Öffnen“, unter **„Beispiele“**:
  - **Primeros pasos**: die Übungsshow der Anleitung, mit 4 PARs, 2 Moving Heads und der leeren Gruppe „Mis escenas“.
  - **Bolo pequeño**: ein kleiner Gig mit PARs vorne und hinten, Stroboskopen und Nebelmaschine. Farben, Effekte und Grundlicht in drei Gruppen, und eine SHOW-Seite, die daraus gebaut ist.
  - **Concierto con cabezas móviles**: ein Konzert mit Farb- und Positionspaletten, einem gespiegelten Kreis und einem Song als Cuelist auf den Playbacks.
  - **Sala con playbacks**: ein Club mit einer Cuelist pro Song auf der ersten Playback-Seite und einer SHOW mit diesen Playbacks.

Die Beispiele öffnen sich wie jede andere Show: Wenn Sie Änderungen speichern, verwenden Sie „Speichern unter…“, damit das Beispiel bleibt, wie es war.

## Das Programmfenster

Das Fenster hat drei Teile: die obere Leiste, die Seitenleiste mit den Seiten und die Seite, die Sie gerade benutzen.

### Die Seiten

| Seite | Taste | Wofür |
|---|---|---|
| Live | F1 | Szenen in Gruppen starten und den Mischer benutzen |
| Show | F2 | Ihr Show-Bildschirm: Tasten, Fader, Drehregler, Gruppen, Uhr… |
| Playbacks | F10 | Fader mit GO, Pause und Flash für Cuelists und Szenen |
| Programmieren | F3 | Fixtures auswählen, ihnen Werte, FX und Paletten geben und aufnehmen |
| Szenen | F4 | Szenen, Effekte, Superszenen und Cuelists erstellen und bearbeiten |
| Patch | F5 | Die Fixtures des Rigs, ihre Adressen, Gruppen und der 2D-Plan |
| Fixtures | F6 | Eigene Fixture-Definitionen erstellen |
| Ausgänge | F7 | Universen und DMX-Interfaces |
| Zuordnungen | F8 | MIDI-Controller, OSC, DMX-Eingang und Tastatur |
| Optionen | F9 | Programmeinstellungen, Fixture-Bibliothek und Updates |

**F11** schaltet das Fenster in den Vollbildmodus oder zurück auf seine Größe. Ist eine Seite gesperrt, trägt sie in der Seitenleiste ein Schloss, und beim Öffnen wird das Passwort abgefragt, falls es eines gibt (siehe [Optionen](opciones.md#show-und-sperre)).

Unten in der Seitenleiste stehen die Programmversion und die **CPU**-Auslastung des Programms („CPU 12 %“, alle Kerne des Computers zusammen). Über 70 % wird sie orange, über 90 % rot: Die Ausgabe kann aus dem Takt geraten. Schließen Sie andere Programme oder stoppen Sie einige Effekte.

### Die obere Leiste

Von links nach rechts:

- **Die Show**: ihr Name. Ein Punkt **●** davor zeigt ungespeicherte Änderungen an; ist das automatische Speichern aktiv, sehen Sie auch die Uhrzeit der letzten Wiederherstellungskopie („Wiederherstellungskopie …“).
- **Datei**: „Neue Show (Strg+N)“, „Öffnen (Strg+O)“, der Pfeil **▾** mit den letzten Shows und „Beispiele“, „Speichern (Strg+S)“ und „Speichern unter…“ (Strg+Umschalt+S).
- **Rhythmus**: **„GO“** und **„Stop“** ändern, was die Szenentasten tun (unter Live, in der SHOW, in den Zuordnungen und auf dem Smartphone). Mit **GO** an startet ein Druck auf eine Szene diese, und die folgenden Drücke geben ihren Rhythmus vor, ohne sie je zu stoppen; mit **Stop** an stoppt ein Druck auf eine Szene diese; sind beide aus, gilt das normale Verhalten ([Live](directo.md#rhythmus-einer-szene-mit-schritten)). Jede Taste einer Szene mit Schritten oder einer laufenden Cuelist (unter Live, in der SHOW, auf den Playbacks und auf dem Smartphone) enthält einen **zweigeteilten Balken**, der bei jedem Schritt erst die eine, dann die andere Hälfte aufleuchten lässt: So sehen Sie den Rhythmus jeder einzelnen. Danach kommen **„BeatGO“** (nächste Unterteilung der Szenen, die mit BeatGO laufen, auch mit der Taste G) und **„Puls“**, der den Audioeingang des Computers abhört (auswählbar unter [Optionen](opciones.md#audio)): Jede Spitze, die die Linie des Pegelmessers überschreitet, bewegt die Szenen, die mit Puls laufen. Ziehen Sie die Linie des Pegelmessers, um die Schwelle zu ändern. Das Tempo der Show, also das der FX und der Szenen im BPM-Modus, ändern Sie mit einem BPM-Fader oder -Drehregler der [SHOW](show.md) oder mit den [Zuordnungen](mapeos.md).
- **Das Schloss**: sperrt Bildschirme, damit sich nichts versehentlich ändert (siehe [Optionen](opciones.md#show-und-sperre)).
- **Die Zielscheibe**: der Lernmodus, um MIDI- oder OSC-Bedienelemente oder Tasten zuzuordnen (siehe [Zuordnungen](mapeos.md#lernmodus)). Ihre grüne Kontrollleuchte blinkt bei jeder eingehenden Nachricht.
- **„N laufen“**: wie viele Szenen und Effekte gerade laufen.
- **GM**: der Grand Master. Unter 100 % wird er orange.
- **„Blackout“** (Strg+B): schaltet alles auf Schwarz, solange er an ist.
- **„Alles stoppen“** (Esc): stoppt alles, was läuft. Der Programmer wird nicht geleert.
- **„?“**: öffnet dieses Handbuch (F12).

In einem schmalen Fenster oder auf einem kleinen Bildschirm wird die Leiste kompakter: Der Showname wird gekürzt, „Speichern unter…“ erscheint nur als Symbol, „N laufen“ nur als Zahl, und in den schmalsten Fenstern zeigen „Blackout“ und „Alles stoppen“ nur ihr Symbol. Fahren Sie mit der Maus über ein Symbol, um zu sehen, was es ist. Wirkt alles zu klein oder zu groß, ändern Sie die **Größe der Oberfläche** unter [Optionen](opciones.md#allgemein).

Wenn Sie das Programm schließen oder eine andere Show öffnen oder anlegen und es ungespeicherte Änderungen gibt, fragt das Programm „Änderungen speichern?“ mit „Abbrechen“, „Nicht speichern“ und „Speichern“. Beim Schließen stoppt das Programm zuerst alle Szenen, die Playbacks und den Programmer und lässt die Ausgänge die Bühne in Ruhe senden: Viele Geräte behalten das Letzte, was sie empfangen haben.

Verwendet die Show, die Sie öffnen, fehlende Dateien (ihre Sounds, Videos oder Bilder; zum Beispiel bei einer Show von einem anderen Computer), erscheint ein Hinweis mit der Liste. Die Show läuft ohne sie. Mit **„In einem Ordner suchen…“** wählen Sie den Ordner, in dem sie liegen: Sie werden nach ihrem Namen gesucht, auch in den Unterordnern, und die Show verwendet sie dann (speichern Sie die Show, damit das so bleibt).

> **Tipp:** Wurde das Programm mit ungespeicherten Änderungen unerwartet beendet, bietet es beim nächsten Start „Ungespeicherte Show wiederherstellen?“ an, mit dem Namen der Show und der Uhrzeit der Kopie. Drücken Sie „Wiederherstellen“ und speichern Sie die Show (eine unbenannte Show unter einem Namen).

## So ist dieses Handbuch aufgebaut

Beginnen Sie mit **Erste Schritte**, wenn Sie das Programm zum ersten Mal benutzen. Die übrigen Seiten erklären je eine Seite des Programms und lassen sich in beliebiger Reihenfolge lesen.

1. [Erste Schritte](primeros-pasos.md): eine vollständige erste Sitzung, vom Patch bis zum Speichern der Show.
2. [Live](directo.md): Gruppen, exklusiv oder mischen, Überblendung, Master, Freigaben, Flash und Mischer.
3. [Show](show.md): der Touchscreen, seine Bereiche, Tasten, Fader, Drehregler, Farbrad, XY-Feld, Bearbeitung und Sperre.
4. [Playbacks](playbacks.md): Playback-Seiten, GO, Pause/Zurück, Flash, Pegel, Aufnehmen auf ein Playback und APC-Controller.
5. [Programmieren](programar.md): Auswahl, Gruppen, 2D-Plan, Dimmer, Farbe, Position, Features, Fader, Paletten, FX, Fächer, Blind, Hervorheben und Lokalisieren, Befehlszeile, Aufnehmen und Aktualisieren.
6. [Szenen](escenas.md): alle Szenentypen, Superszenen, Cuelists mit Tracking, Ordner, Import und Export.
7. [Patch](patch.md): Fixtures hinzufügen, Adressen, Universen, Nummern, Fixture-Gruppen, 2D-Plan, Grenzen, LED-Matrizen und Import aus einer anderen Show.
8. [Fixtures](fixtures.md): die Bibliothek, der Fixture-Editor, die Open Fixture Library, GDTF und Ihre Fixtures.
9. [Ausgänge](salidas.md): Universen, DMX-USB-Interfaces, Art-Net und sACN sowie der Monitor für die DMX-Pegel.
10. [Zuordnungen](mapeos.md): MIDI, OSC, DMX-Eingang und Tastatur, der Lernmodus und die Vorlagen für Akai APC.
11. [Optionen](opciones.md): Sprache, Design, Größe, Start, Sperre, automatisches Speichern, Prioritäten, Fernsteuerung, Bibliothek, Updates.
12. [Fernsteuerung](remoto.md): die SHOW auf dem Smartphone oder Tablet.
13. [Tastenkürzel](atajos.md): alle Tasten und Kombinationen.

## Wörter, die Ihnen oft begegnen

- **Fixture**: jedes gepatchte Lichtgerät (ein PAR, ein Moving Head, ein LED-Streifen…).
- **Universum**: 512 DMX-Kanäle. Jedes Universum wird über ein Interface ausgegeben.
- **Programmer**: Hier geben Sie den ausgewählten Fixtures Werte. Was im Programmer ist, hat Vorrang vor allem, bis Sie ihn leeren.
- **Szene**: was Sie aufnehmen, um es später abzuspielen. Es gibt mehrere Typen: statisch, Schritte, FX, Superszene, Cuelist… (siehe [Szenen](escenas.md)).
- **Gruppe**: eine Spalte mit Szenen unter Live. In einer exklusiven Gruppe läuft immer nur eine Szene.
- **Fixture-Gruppe**: eine Zusammenstellung von Fixtures, um sie auf einmal auszuwählen (sie werden im [Patch](patch.md#fixture-gruppen) erstellt).
- **Cuelist**: eine Liste von Cues, die mit GO weiterschalten. Jeder Cue speichert nur, was sich ändert.
- **Playback**: ein Fader mit seinen Tasten (GO, Pause/Zurück, Flash), der eine Cuelist oder eine Szene abspielt.
- **Palette**: ein gespeicherter Wert (eine Farbe, eine Position…), dem die Szenen per Referenz folgen: Ändern Sie die Palette, ändern sich die Szenen, die sie verwenden.
- **Priorität**: von 0 bis 10. Steuern zwei Szenen denselben Kanal, gewinnt die mit der höheren Priorität.
- **SHOW**: der Bedienbildschirm, den Sie selbst bauen.

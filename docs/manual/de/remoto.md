# Fernsteuerung

Die **Fernsteuerung** zeigt die Seiten der [SHOW](show.md) im Browser eines Smartphones oder Tablets, das mit demselben WLAN verbunden ist wie der Computer. Sie müssen nichts installieren: Sie funktioniert in beide Richtungen, was Sie auf dem Smartphone berühren, ändert sich auf dem Computer, und was sich auf dem Computer ändert, sehen Sie auf dem Smartphone.

## Aktivieren

1. Öffnen Sie auf dem Computer **Optionen** (F9) › **Fernsteuerung**.
2. Aktivieren Sie **„Fernsteuerung an“**.
3. Wenn Sie möchten, geben Sie eine **PIN** ein. Ohne PIN kann jeder im Netzwerk die Fernsteuerung nutzen.
4. Darunter erscheinen die **Adressen** des Computers (zum Beispiel `http://192.168.1.20:8800/`), eine für jedes Netzwerk, mit dem er verbunden ist. Die Kopiertaste legt die Adresse in die Zwischenablage.

## Smartphone oder Tablet verbinden

1. Verbinden Sie das Smartphone mit **demselben WLAN** wie den Computer.
2. Öffnen Sie den Browser und geben Sie eine der Adressen ein.
3. Gibt es eine PIN, geben Sie sie ein und drücken Sie **OK**. Sie wird auf jedem Gerät nur einmal abgefragt.

Unter Optionen sehen Sie, wie viele Geräte verbunden sind.

> **Tipp:** Legen Sie die Seite auf den Startbildschirm des Smartphones: Sie öffnet sich dann wie eine App, mit dem Symbol des Programms.

> **Tipp:** Verbindet sich das Smartphone nicht, prüfen Sie, ob beide im selben Netzwerk sind und ob die Windows-Firewall DMX Total Control den Netzwerkzugriff erlaubt. Nutzt ein anderes Programm den Port, ändern Sie ihn unter Optionen (der Hinweis erscheint in Rot).

## Was Sie sehen

Oben ist die Leiste der Fernsteuerung:

- Der **Name der Show** und die **Tabs** der SHOW-Seiten. Das Smartphone merkt sich die zuletzt geöffnete Seite.
- Ein **Punkt**, der anzeigt, ob eine Verbindung zum Computer besteht.
- **BO**: Blackout.
- Die Taste für den **Vollbildmodus**.

Darunter die Seite mit ihren Bereichen, wie auf dem Computer:

- **Tasten**: werden mit dem Finger gedrückt, auch Flash-Tasten (solange der Finger darauf liegt). Sie leuchten, wenn sie aktiv sind.
- **Fader**: werden gezogen; die mit einer **Flash**-Taste haben sie darunter.
- **Drehregler und Räder**: werden bewegt, indem Sie den Finger nach oben und unten ziehen. Fader und Drehregler folgen demselben [Bereich](show.md#fader) wie auf dem Computer und zeigen den tatsächlichen Wert.
- **Gruppen**: ihre Szenen als Tasten und ihr Master.
- **Playbacks**: die Streifen mit Name, laufendem und nächstem Cue, **GO**, **Zurück**, Fader und **Flash**. Bereiche, die der aktuellen Seite folgen, haben ◀ ▶ zum Wechseln.
- **Farbrad** und **XY-Feld**, mit ihren Tasten **Freigeben** und **Mitte**.
- **Uhr**, **Rhythmus** (**GO**, **Stop** und **BeatGO**; die Tasten von Szenen mit Schritten, Cuelists oder FX auf „BPM der Show“ enthalten ihren Rhythmusbalken), **Notizen**, **Läuft gerade** und **Bild**.

**Timer**, **Live-Mischer** und **DMX-Pegel** erscheinen als „Nur am Computer“.

Hat ein Playback etwas zugewiesen, fügt die Fernsteuerung eine Seite **„Playbacks“** mit den Streifen der aktuellen Playback-Seite hinzu.

Die Fernsteuerung verwendet dasselbe [Design](opciones.md#allgemein) wie das Programm und erscheint in der Sprache des Smartphones, wenn diese Spanisch, Portugiesisch, Französisch, Italienisch oder Deutsch ist (sonst auf Englisch).

## Wenn die Verbindung abreißt

Verliert das Smartphone die Verbindung, erscheint „Keine Verbindung mit DMX Total Control. Neuer Versuch…“, und es verbindet sich von selbst neu. Was der Finger gedrückt hielt (ein Flash, eine Taste), wird auf dem Computer losgelassen, damit nichts eingeschaltet bleibt.

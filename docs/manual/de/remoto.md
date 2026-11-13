# Fernsteuerung

Die **Fernsteuerung** zeigt die Seiten der [SHOW](show.md) im Browser eines Smartphones oder Tablets, das mit demselben WLAN verbunden ist wie der Computer. Sie müssen nichts installieren: Sie funktioniert in beide Richtungen, was Sie auf dem Smartphone berühren, ändert sich auf dem Computer, und was sich auf dem Computer ändert, sehen Sie auf dem Smartphone.

## Aktivieren

1. Öffnen Sie auf dem Computer **Optionen** (F9) › **Fernsteuerung**.
2. Aktivieren Sie **„Fernsteuerung an“**.
3. Wenn Sie möchten, geben Sie eine **PIN** ein. Ohne PIN kann jeder im Netzwerk die Fernsteuerung nutzen.
4. Darunter erscheinen die **Adressen** des Computers (zum Beispiel `http://192.168.1.20:8800/`), eine für jedes Netzwerk, mit dem er verbunden ist, mit dem Namen des Netzwerks davor. Das WLAN steht zuerst. Die Kopiertaste legt die Adresse in die Zwischenablage. Hat der Computer einen **Namen im Netzwerk**, ist die erste Adresse die des Namens, zum Beispiel `http://dmxtotalcontrol.local:8800/` (weiter unten).

## Smartphone oder Tablet verbinden

1. Verbinden Sie das Smartphone mit **demselben WLAN** wie den Computer.
2. Öffnen Sie den Browser und geben Sie eine der Adressen ein, oder scannen Sie den QR-Code (weiter unten).
3. Gibt es eine PIN, geben Sie sie ein und drücken Sie **OK**. Sie wird auf jedem Gerät nur einmal abgefragt.

Unter Optionen sehen Sie, wie viele Geräte verbunden sind.

> **Tipp:** Legen Sie die Seite auf den Startbildschirm des Smartphones: Sie öffnet sich dann wie eine App, mit dem Symbol des Programms.

> **Tipp:** Verbindet sich das Smartphone nicht, prüfen Sie, ob beide im selben Netzwerk sind und ob die Windows-Firewall DMX Total Control den Netzwerkzugriff erlaubt. Nutzt ein anderes Programm den Port, ändern Sie ihn unter Optionen (der Hinweis erscheint in Rot).

> **Tipp:** Beim ersten Einschalten der Fernsteuerung fragt Windows, ob DMX Total Control das Netzwerk nutzen darf. Erlauben Sie es in **privaten** Netzwerken. Führt Windows das WLAN als **öffentliches** Netzwerk, erreicht das Smartphone den Computer nicht: Stellen Sie es in den Netzwerkeinstellungen von Windows auf privat.

> **Tipp:** Lässt das WLAN des Veranstaltungsorts die Geräte einander nicht sehen, schalten Sie am Computer den **mobilen Hotspot** von Windows ein und verbinden Sie das Smartphone damit. Die Fernsteuerung ist dann immer unter `http://192.168.137.1:8800/` erreichbar.

## Den QR-Code scannen

Bei eingeschalteter Fernsteuerung erscheint unter den Adressen ein **QR-Code**. Damit öffnet sich die Fernsteuerung, ohne etwas einzugeben:

1. Verbinden Sie das Smartphone oder Tablet mit **demselben WLAN** wie den Computer.
2. Richten Sie die Kamera auf den Code.
3. Tippen Sie auf den Link, der erscheint.

- Der Code enthält die Adresse mit dem farbigen Rahmen: anfangs die erste numerische Adresse, die auf jedem Smartphone funktioniert. Für eine andere klicken Sie auf ihre Zeile oder auf ihre QR-Code-Taste.
- Gibt es eine PIN, steckt sie auch im Code: Das Smartphone kommt ohne Abfrage hinein. Die PIN bleibt nicht in der Adresszeile des Browsers stehen.
- Deshalb kann mit PIN jeder hinein, der den Code fotografiert.
- **„Größer“** zeigt den Code groß, um ihn aus ein paar Metern zu scannen.
- Im Design **Nacht** ist der Code gedämpft, damit er nicht blendet.

## Über den Namen finden

Der Router kann dem Computer von einem Tag auf den anderen eine andere Adresse geben. Der Name ändert sich nicht: Bei eingeschalteter Fernsteuerung öffnet das Smartphone `http://dmxtotalcontrol.local:8800/`, auch wenn die Adresse eine andere ist. So funktioniert das Symbol auf dem Startbildschirm weiter.

Öffnen Sie die Fernsteuerung mit dem QR-Code oder einer numerischen Adresse, prüft das Smartphone im Hintergrund, ob es den Computer über den Namen erreicht. Wenn ja, wechselt es von selbst zu `http://dmxtotalcontrol.local:8800/`, auf derselben Seite und ohne nach der PIN zu fragen. Legen Sie das Symbol erst nach diesem Wechsel auf den Startbildschirm: Die Adresszeile zeigt dann den Namen.

- Unter Optionen steht die Adresse **„Über den Namen“** an erster Stelle.
- Den Namen ändern Sie unter **Name im Netzwerk**. Erlaubt sind Buchstaben, Ziffern und Bindestriche; Großbuchstaben werden zu Kleinbuchstaben und Leerzeichen zu Bindestrichen. Leer: kein Name.
- Es funktioniert auf iPhone und iPad sowie auf Smartphones ab Android 12. Auf älteren verwenden Sie eine numerische Adresse oder den QR-Code.
- Es funktioniert nicht, wenn auf dem Smartphone ein VPN aktiv ist oder es mobile Daten nutzt.
- Erreicht das Smartphone den Computer nicht über den Namen, bleibt es ohne Hinweis bei der numerischen Adresse.
- Antwortet der Name später nicht mehr, kehrt das Smartphone zur numerischen Adresse zurück, sofern diese antwortet.
- Nutzt ein anderer Computer im Netzwerk den Namen schon (zum Beispiel ein anderes DMX Total Control), nimmt dieser den nächsten, `dmxtotalcontrol-2`, und Optionen zeigt das unter dem Namen an.
- Ist der Netzwerkport 5353 nicht verfügbar oder nutzen andere Computer den Namen und `-2` bis `-9` schon, erscheint ein Hinweis in Rot. Die numerischen Adressen funktionieren weiter.

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
- **Playbacks**: die Streifen mit Name, laufendem und nächstem Cue, **GO**, **Zurück**, Fader und **Flash**. Ein A/B-Fader zeigt „A/B“ und wie weit seine Überblendung ist, mit gestricheltem Rand. Bereiche, die der aktuellen Seite folgen, haben ◀ ▶ zum Wechseln.
- **Farbrad** und **XY-Feld**, mit ihren Tasten **Freigeben** und **Mitte**.
- **Uhr**, **Rhythmus** (**GO**, **Stop** und **BeatGO**; die Tasten von Szenen mit Schritten, Cuelists oder FX auf „BPM der Show“ enthalten ihren Rhythmusbalken), **Notizen**, **Läuft gerade** und **Bild**.
- **Timer**: derselbe wie auf dem Computer, mit **Starten** / **Pause** und **Zurücksetzen**. Starten Sie ihn auf dem Smartphone, läuft er auch auf dem Computer. Halten Sie ihn auf dem Computer an, stoppt er auf allen Smartphones. Als Countdown wird er in der letzten Minute orange und bei null rot.
- **DMX-Pegel**: die 512 Kanäle des gewählten Universums, wie auf dem Computer: Je höher der Wert, desto heller das Feld. Das Smartphone erhält sie etwa 8-mal pro Sekunde, nur solange diese Seite auf dem Bildschirm ist.
- **Live-Mischer**: dieselben Kanalzüge wie der [Mischer](directo.md#mischer) des Computers, nebeneinander: „Alle Fixtures“ und einer für jede Fixture-Gruppe. Passen sie nicht auf den Bildschirm, wischen Sie seitwärts. Jeder Kanalzug hat den Dimmer, **Voll**, **Blackout**, **Farbton**, **Sättigung**, **Farbe zurücksetzen**, **Stroboskop** mit seinem Tempo, **Flash**, **Flash exkl.** und **Voll exkl.**
  - **Flash** und **Flash exkl.** wirken, solange der Finger darauf liegt. Sie werden losgelassen, wenn das Smartphone die Verbindung verliert, sich sperrt oder Sie die App wechseln.
  - Beim Loslassen gehen sie aus, auch wenn sie auf dem Computer eingerastet waren (Strg+Klick).
  - Der Rand eines Kanalzugs wird bei einem Flash oder Voll orange und bei Blackout rot.
  - Was Sie auf dem Smartphone ändern, ändert sich auf dem Computer, und umgekehrt.

Hat ein Playback etwas zugewiesen, fügt die Fernsteuerung eine Seite **„Playbacks“** mit den Streifen der aktuellen Playback-Seite hinzu.

Die Fernsteuerung verwendet dasselbe [Design](opciones.md#allgemein) wie das Programm und erscheint in der Sprache des Smartphones, wenn diese Spanisch, Portugiesisch, Französisch, Italienisch oder Deutsch ist (sonst auf Englisch).

## Wenn die Verbindung abreißt

Verliert das Smartphone die Verbindung, erscheint „Keine Verbindung mit DMX Total Control. Neuer Versuch…“, und es verbindet sich von selbst neu. Was der Finger gedrückt hielt (ein Flash, eine Taste), wird auf dem Computer losgelassen, damit nichts eingeschaltet bleibt.

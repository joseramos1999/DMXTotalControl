# Erste Schritte

![Die Einstiegsanleitung auf der Seite Patch](../img/primeros-pasos.png)

## Die Anleitung

Beim ersten Start des Programms erscheint die **Einstiegsanleitung**: eine Karte unten rechts mit fünf Schritten (Ihre Fixtures, ein Look, eine Szene, Live und die Playbacks). **„Weiter“** und **„Zurück“** wechseln den Schritt und bringen Sie zur jeweiligen Seite; die Seiten lassen sich benutzen, während die Karte offen ist. Sie schließen sie mit dem **✕** und öffnen sie wieder über Optionen › Hilfe › **„Anleitung starten“**.

Im ersten Schritt öffnet **„Übungsshow öffnen“** das Beispiel „Primeros pasos“ (haben Sie ungespeicherte Änderungen, fragt das Programm vorher, ob sie gespeichert werden sollen). Es enthält bereits vier LED-PARs RGBW in der Fixture-Gruppe `PARs`, zwei Moving Heads in `Móviles` und die leere Gruppe „Mis escenas“ unter Live, bereit für Ihre Szenen.

## Die vollständige Sitzung

In dieser ersten Sitzung gehen Sie den ganzen Weg: den Ausgang verbinden, einige Fixtures patchen, im Programmer einen Look bauen, ihn als Szene in einer Gruppe aufnehmen, ihn unter Live starten, eine Cuelist auf ein Playback aufnehmen und die Show speichern. Das dauert etwa zwanzig Minuten.

Wir verwenden Fixtures aus der generischen Bibliothek: vier LED-PARs RGBW und zwei Moving Heads. Haben Sie andere Geräte, suchen Sie statt der generischen deren Hersteller und Modell.

> **Tipp:** Sie können dieser Anleitung auch ohne angeschlossenes DMX-Interface folgen. Was ausgegeben wird, sehen Sie unter Ausgänge › „DMX-Pegel“ und im 2D-Plan unter Programmieren.

> **Tipp:** Mit der geöffneten Übungsshow haben Sie die Fixtures und die Gruppe schon: Springen Sie zu Schritt 4 und verwenden Sie die Fixture-Gruppe `Móviles`, wo hier `Köpfe` steht.

## 1. Den Ausgang verbinden

1. Schließen Sie Ihr DMX-USB-Interface an den Computer an (oder sorgen Sie dafür, dass der Computer im selben Netzwerk ist wie Ihr Art-Net- oder sACN-Node).
2. Öffnen Sie **Ausgänge** (F7).
3. Öffnen Sie in der Zeile von Universum 1 die Auswahlliste, in der „Nicht verbunden“ steht, und wählen Sie Ihr Interface.
4. Erscheint es nicht, drücken Sie **„Neu suchen“**.

Das verbundene Universum wird grün markiert. Mehr dazu unter [Ausgänge](salidas.md).

## 2. Einige Fixtures patchen

1. Öffnen Sie **Patch** (F5) und drücken Sie **„Fixtures hinzufügen“**.
2. Wählen Sie unter **Hersteller** **Generic** (Sie können in „Hersteller suchen…“ tippen).
3. Wählen Sie unter **Modell** **LED PAR RGBW**.
4. Im Bereich **Patch** rechts:
   - **Modus**: „5ch DRGBW“.
   - **Anzahl**: 4.
   - **Universum**: 1. Die **Adresse** wird automatisch auf die erste freie gesetzt.
   - Lassen Sie **„Gruppe daraus erstellen“** angehakt und geben Sie unter **Gruppe** den Namen `PARs` ein.
5. Drücken Sie **„4 Fixtures patchen“**. Unten sehen Sie, welche Adressen sie belegen.
6. Wählen Sie jetzt, ohne das Fenster zu schließen, **Moving Head Spot**, Modus „11ch Standard“, Anzahl 2, Gruppe `Köpfe`, und drücken Sie **„2 Fixtures patchen“**.
7. Drücken Sie **„Schließen“**.

> **Tipp:** Stellen Sie an jedem Gerät dieselbe DMX-Adresse ein, die die Liste im Patch zeigt. Um ein Gerät auf der Bühne zu finden, wählen Sie es aus und drücken Sie **„Identifizieren“**: Es blinkt weiß.

## 3. Eine Gruppe unter Live erstellen

Szenen werden unter Live gestartet, geordnet in Gruppen. Erstellen Sie eine, bevor Sie aufnehmen:

1. Öffnen Sie **Live** (F1) und drücken Sie **„Neue Gruppe“**.
2. Klicken Sie auf den Namen der Spalte („Gruppe 1“), löschen Sie ihn, geben Sie `Mis escenas` ein und drücken Sie Eingabe.
3. Wählen Sie in der Auswahlliste für die Überblendzeit der Spalte (dort steht „Pro Szene“) **2 s**.

Die Gruppe ist **„Exklusiv“**: Es läuft immer nur eine Szene, und beim Drücken einer anderen wird übergeblendet.

## 4. Einen Look im Programmer bauen

1. Öffnen Sie **Programmieren** (F3).
2. Klicken Sie im Fixture-Bereich auf die Beschriftung der Gruppe **PARs**: Die vier werden ausgewählt.
3. Drücken Sie im Bereich **Schnell** neben dem Fader **Dimmer** auf **100%**.
4. Klicken Sie auf das **rote** Farbfeld.

Die PARs leuchten rot, sobald Sie es berühren: Was Sie in den Programmer geben, wird sofort ausgegeben, über allem anderen.

> **Tipp:** Dasselbe mit der Befehlszeile: Geben Sie `GROUP 1 @ FULL` ein und drücken Sie Eingabe. `GROUP 1` ist die erste Fixture-Gruppe. Siehe [die Befehlszeile](programar.md#befehlszeile).

## 5. Die Szene in die Gruppe aufnehmen

1. Drücken Sie **„Aufnehmen…“** (oben rechts).
2. Lassen Sie unter **Aufnehmen in** **„Neue Szene“** stehen.
3. Geben Sie unter **Name** `Rot` ein.
4. Wählen Sie unter **Zur Gruppe hinzufügen** **Mis escenas**.
5. Drücken Sie **„Aufnehmen“**.

Nehmen Sie einen zweiten Look auf: Klicken Sie bei ausgewählten PARs und Dimmer auf 100 % auf das **blaue** Farbfeld, drücken Sie **„Aufnehmen…“**, Name `Blau`, Gruppe **Mis escenas** und **„Aufnehmen“**.

Bevor Sie zu Live wechseln, drücken Sie **„Alles zurücksetzen“**, um den Programmer zu leeren. Solange der Programmer Werte enthält, haben sie Vorrang vor den Szenen, und Sie würden nicht sehen, was Sie starten.

> **Tipp:** Haken Sie im Aufnahmefenster **„Programmer nach dem Aufnehmen leeren“** an, dann leert sich der Programmer bei jeder Aufnahme von selbst. Die Option wird gespeichert.

## 6. Die Szenen unter Live starten

1. Öffnen Sie **Live** (F1).
2. Drücken Sie **Rot**: Die Kachel leuchtet in der Farbe der Gruppe, und die PARs werden rot.
3. Drücken Sie **Blau**: Die Gruppe blendet in 2 Sekunden von Rot nach Blau über.
4. Bewegen Sie den **Master** der Spalte, um die Intensität der ganzen Gruppe zu senken.
5. Um die Szene freizugeben, drücken Sie das Quadrat im Kopf der Spalte („Diese Gruppe freigeben“), oder wechseln Sie in der oberen Leiste zu **Stop**, drücken Sie **Blau** und kehren Sie zu **GO** zurück.

Mehr unter [Live](directo.md).

## 7. Eine Cuelist auf ein Playback aufnehmen

Cuelists schalten mit GO von Cue zu Cue. Am schnellsten erstellen Sie eine, indem Sie direkt auf ein Playback aufnehmen.

1. Klicken Sie unter **Programmieren** auf die Beschriftung **Köpfe**.
2. Drücken Sie **„Beam on“** (Dimmer voll und Shutter offen) und **„Zentrieren“** (Pan und Tilt in die Mitte).
3. Öffnen Sie **Playbacks** (F10) und drücken Sie **„Aufnehmen“**: Die Taste blinkt rot.
4. Klicken Sie auf das leere Playback **1**: Eine neue Cuelist mit Cue 1 wird erstellt.
5. Gehen Sie zurück zu **Programmieren**, öffnen Sie **Features**, Tab **Position**, und ziehen Sie das Fadenkreuz des XY-Felds an eine andere Stelle.
6. Drücken Sie unter **Playbacks** **„Aufnehmen“** und klicken Sie erneut auf Playback 1: Cue 2 wird aufgenommen, nur mit dem, was sich geändert hat (die Position).
7. Gehen Sie zurück zu **Programmieren** und drücken Sie **„Alles zurücksetzen“**.
8. Klicken Sie unter **Playbacks** auf den Namen von Playback 1, um es auszuwählen, und drücken Sie die **Leertaste** (oder die große Taste **GO**): Cue 1 läuft. Drücken Sie noch einmal: Es geht zu Cue 2.

Der Fader des Playbacks ist seine Intensität: Ziehen Sie ihn von null hoch, startet die Cuelist. Mehr unter [Playbacks](playbacks.md).

## 8. Die Show speichern

1. Drücken Sie **Strg+S** (oder das Diskettensymbol in der oberen Leiste).
2. Da es das erste Mal ist, wählen Sie Ordner und Namen: Die Show wird als `.qxw`-Datei gespeichert.

Ab jetzt speichert Strg+S in dieselbe Datei und behält die Version vor dem ersten Speichern jeder Sitzung als `.bak`. Ist das automatische Speichern aktiv (siehe [Optionen](opciones.md#automatisches-speichern)), speichert das Programm alle paar Minuten eine separate Wiederherstellungskopie, ohne Ihre Datei anzutasten: Wird es unerwartet beendet, bietet es an, die Änderungen wiederherzustellen.

## Und jetzt…

- Bauen Sie Ihren Bedienbildschirm in der [Show](show.md).
- Lernen Sie, [Paletten](programar.md#paletten) und [FX](programar.md#fx) im Programmer zu verwenden.
- Bearbeiten Sie die Cues, ihre Zeiten und ihr Tracking unter [Szenen](escenas.md#cuelists).
- Schließen Sie einen MIDI-Controller unter [Zuordnungen](mapeos.md) an.

# Ausgänge

![Die Seite Ausgänge](../img/salidas.png)

**Ausgänge** (F7) verbindet jedes **Universum** der Show (512 DMX-Kanäle) mit einem **Interface**: einem DMX-USB-Interface, einem Art-Net-Node oder einem sACN-Ausgang. Außerdem enthält die Seite den Monitor für die [DMX-Pegel](salidas.md#dmx-pegel), der live zeigt, was ausgegeben wird.

Im Kopf:

- **„Universen“** / **„DMX-Pegel“**: die beiden Ansichten der Seite.
- **„Neu suchen“**: sucht erneut nach angeschlossenen Interfaces.
- **„Universum entfernen“** und **„Universum hinzufügen“**.

## Ein Universum verbinden

1. Schließen Sie das Interface an den Computer an (oder ans Netzwerk, bei Art-Net und sACN).
2. Erscheint es nicht unter **„Erkannte Ausgänge“**, drücken Sie **„Neu suchen“**.
3. Öffnen Sie in der Zeile des Universums die Auswahlliste (dort steht „Nicht verbunden“) und wählen Sie den Ausgang.

Jeder Ausgang erscheint mit dem Namen seines Typs und dem des Interfaces oder Netzwerks: „DMX USB — …“, „ArtNet — …“, „E1.31 — …“ (E1.31 ist sACN). Das verbundene Universum wird grün markiert. Zum Trennen wählen Sie „Nicht verbunden“.

Die Verbindung wird für den nächsten Programmstart gespeichert.

> **Tipp:** DMX-USB-Interfaces mit FTDI-Chips brauchen deren Treiber. Haben Sie sie nicht, bietet der Installer des Programms am Ende die Seite zum Herunterladen an.

## Universen

- **„Universum hinzufügen“** fügt eines am Ende hinzu. Beim Patchen wählen Sie, in welches Universum jedes Fixture kommt.
- Klicken Sie auf den **Namen** eines Universums, um ihn zu ändern (der Name wird mit der Show gespeichert).
- **„Universum entfernen“** entfernt das **letzte** (eines bleibt immer). Sind darin Fixtures gepatcht, geht das nicht: Verschieben Sie sie vorher im Patch in ein anderes Universum oder löschen Sie sie. Ist es der Eingang eines Controllers, wird vorher nachgefragt, denn dieser Controller funktioniert dann nicht mehr, bis Sie ihn unter Zuordnungen wieder aktivieren.

## DMX-Pegel

**„DMX-Pegel“** zeigt die 512 Kanäle eines Universums so, wie sie das Programm verlassen, mit allem angewendet (Szenen, Programmer, Mischer, Grand Master, Grenzen…):

- Wählen Sie das **Universum** in der Auswahlliste.
- Jedes Feld enthält die Kanalnummer und ihren Wert (0–255). **„In % anzeigen“** wechselt zu Prozent.
- Felder mit Rand gehören zu einem Fixture. Fahren Sie mit der Maus über einen Kanal, um zu sehen, zu welchem Fixture er gehört („Name — Kanal N“).

> **Tipp:** Reagiert ein Fixture nicht, schauen Sie hier, ob sich sein Kanal bewegt. Bewegt er sich, liegt das Problem im Kabel, in der Adresse des Geräts oder im Interface; wenn nicht, im Patch oder in dem, was Sie starten.

Derselbe Monitor kann auch ein Bereich der [SHOW](show.md) sein („DMX-Pegel“).

## DMX-Eingang

Ein Interface mit DMX-Eingang (oder ein daran angeschlossenes Pult) kann das Programm steuern: Es wird unter [Zuordnungen](mapeos.md#geräte) als Gerät aktiviert.

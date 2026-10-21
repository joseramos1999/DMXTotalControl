# Fixture

![La pagina Fixture](../img/fixtures.png)

Per patchare un apparecchio, il programma ha bisogno della sua **definizione**: quali canali ha, cosa fa ciascuno e in quali modalità lavora. Questa pagina spiega da dove vengono le definizioni e come creare le tue nella pagina **Fixture** (F6).

## La libreria di fixture

Quando aggiungi fixture nel [Patch](patch.md#aggiungere-fixture) scegli tra tutte queste definizioni:

- **La libreria integrata**: migliaia di fixture di molti produttori, in formato `.qxf`.
- **Le generiche**: 16 fixture di base nel produttore **Generic** (PAR LED RGB, RGBA, RGBW, RGBWA, RGBWA+UV e bianco, barra LED a 8 segmenti, teste spot, beam e wash, scanner, strobo, blinder, laser, macchina del fumo e hazer), più il **Generic Dimmer**.
- **La Open Fixture Library**, se la scarichi.
- **Le fixture GDTF** che importi.
- **Le tue fixture**: quelle che crei in questa pagina o importi.

In [Opzioni › Libreria di fixture](opciones.md#libreria-di-fixture) si vede quante definizioni sono disponibili e c'è tutto il necessario per ampliarla.

> **Consiglio:** perché Programma, gli FX e le palette riconoscano la funzione di ogni canale (dimmer, rosso, pan, gobo…), la definizione deve riportare il gruppo o il preset del canale. Quelle della libreria lo riportano; nelle tue, scegli bene il preset di ogni canale.

## Creare una fixture

La pagina **Fixture** è il creatore di fixture. A sinistra, **«Le mie fixture»** elenca quelle che hai creato; a destra, l'editor.

1. Premi **«Nuova fixture»**, o **«Parti dalla libreria…»** per iniziare da una copia di una che esiste già (si chiama uguale con «(copia)»).
2. In **Fixture**, scrivi il **Produttore** (per esempio `Generic` o la marca), il **Modello** (`PAR 18x10W RGBWA`) e scegli il **Tipo**.
3. Aggiungi i **canali** (vedi sotto).
4. Prepara le **modalità**.
5. Premi **«Salva fixture»**.

La fixture è subito disponibile nel Patch, sotto il suo produttore. In alto si vede il suo stato: «Modifiche non salvate», «Salvata nelle tue fixture» o «Non ancora salvata». Se apri un'altra fixture con modifiche non salvate, viene chiesto se scartarle («Continua a modificare» / «Scarta modifiche»).

### Canali

In **Canali**, il tasto **«Aggiungi»** offre i canali più comuni (dimmer, rosso, verde, blu, bianco, ambra, UV, pan e pan fine, tilt e tilt fine, strobo, ruota colori, ruota gobo, velocità pan/tilt) e **«Canale personalizzato»**.

In ogni canale:

- Il suo **nome** e il suo **preset** (il menu a tendina): il preset dice cosa fa il canale. Cambiare preset cambia anche il nome, a meno che tu non gliene abbia dato uno tuo.
- Il cestino lo elimina.

Un canale **«Personalizzato»** ha inoltre:

- **Gruppo**: la sua funzione generale (intensità, colore, gobo, fascio…).
- **Colore**: in un canale di intensità, quale colore è (rosso, verde…) o «Nessuno» per un dimmer.
- **Fine**: è il canale fine (16 bit) di un altro.
- **«Intervalli (N)»**: cosa fa ogni intervallo di valori DMX, per esempio 0-7 Chiuso, 8-255 Strobo. Premi **«Aggiungi intervallo»** e scrivi la sua **Descrizione**. Gli intervalli non si possono sovrapporre. Sono i preset che poi vedrai come tasti in Programma.

I nuovi canali si aggiungono alla fine della modalità che hai aperta.

### Modalità

In **Modalità**, **«Aggiungi»** crea una modalità con tutti i canali. Ogni scheda è una modalità:

- **Nome** della modalità e il cestino per eliminarla (ne resta sempre una).
- L'elenco è l'**ordine DMX** della modalità: spunta i canali che usa e spostali con **▲ ▼** fino alla loro posizione. Il numero a sinistra è il suo canale DMX all'interno della modalità.

### Eliminare una tua fixture

Apri la fixture in «Le mie fixture» e premi **«Elimina»**. Scompare dalla libreria al prossimo avvio.

Le tue fixture si salvano come file `.qxf` nella cartella delle fixture utente (in Opzioni › «Cartella delle fixture utente» › «Apri cartella»).

## Open Fixture Library

La Open Fixture Library è una libreria aperta con migliaia di fixture aggiuntive (licenza MIT).

1. Apri **Opzioni** (F9).
2. In **Libreria di fixture › Open Fixture Library**, premi **«Scarica»**. Serve internet.
3. Aspetta che finisca la barra di avanzamento («Annulla» la interrompe).

Alla fine viene indicato quante fixture sono state scaricate e quante sono nuove. Poi, nello stesso punto si vede quante ne ha e quando è stata scaricata.

Per aggiornarla, premi **«Aggiorna»** (lo stesso tasto). Se in Opzioni è attivo «Cerca aggiornamenti all'avvio», il programma controlla una volta al giorno su GitHub se la libreria ha fixture nuove o migliorate dal tuo download, ti avvisa ed evidenzia il tasto; «Cerca aggiornamenti» lo controlla subito. L'aggiornamento:

- aggiunge le fixture nuove (sono elencate sotto);
- cambia quelle migliorate, anche quelle già patchate (se la loro nuova modalità non ci sta dove sono, restano con la versione precedente finché non le patchi di nuovo);
- conserva quelle che la libreria non ha più, nel caso uno show le usi;
- non tocca le tue fixture né quelle della libreria integrata: se hanno lo stesso produttore e modello, resta la tua (o quella integrata).

Un download che non si può leggere, o che si interrompe, non cambia niente.

## Importare GDTF

GDTF è il formato aperto che i produttori pubblicano su GDTF-Share (con un account gratuito).

1. Scarica i `.gdtf` dei tuoi apparecchi (o lo `.zip` del download multiplo di GDTF-Share). Il tasto **«GDTF-Share»** di Opzioni apre il sito.
2. In **Opzioni › Libreria di fixture › Fixture GDTF**, premi **«Importa GDTF…»** e scegli i file, le cartelle o lo `.zip`.
3. Vengono convertiti in background, con una barra di avanzamento. **«Ferma»** interrompe l'importazione.

Alla fine viene indicato quante sono state convertite e, se qualcuna non si è potuta leggere, perché. Se la stessa fixture è già in un'altra libreria, quella GDTF viene aggiunta come «(GDTF)» e puoi usarle entrambe. Importarla di nuovo la aggiorna.

Di ogni modalità si usa il primo *DMX break*; i canali virtuali (senza DMX) vengono omessi.

## Importare file di fixture

**Opzioni › Importa file di fixture › «Importa…»** accetta definizioni `.qxf` (il formato di QLC+), file `.gdtf` o uno `.zip` che li contenga. Vengono aggiunte alla libreria e viene indicato quante sono nuove.

### Importare le mie fixture di QLC+

Se hai creato fixture in QLC+ su questo computer, **Opzioni › «Importa le mie fixture di QLC+» › «Importa»** le copia tra le tue fixture.

## Formati che non si importano

Le librerie di altri programmi in formati chiusi o cifrati non si possono importare. La maggior parte delle fixture si trova nella Open Fixture Library o su GDTF-Share; quelle che mancano, creale in questa pagina.

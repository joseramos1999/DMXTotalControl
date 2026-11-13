# Fixture

![La pagina Fixture](../img/fixtures.png)

Per patchare un apparecchio, il programma ha bisogno della sua **definizione**: quali canali ha, cosa fa ciascuno e in quali modalità lavora. Questa pagina spiega da dove vengono le definizioni e come creare le tue nella pagina **Fixture** (F6).

## La libreria di fixture

Quando aggiungi fixture nel [Patch](patch.md#aggiungere-fixture) scegli tra tutte queste definizioni:

- **La libreria integrata**: migliaia di fixture di molti produttori, in formato `.qxf`.
- **Le generiche**: 16 fixture di base nel produttore **Generic** (PAR LED RGB, RGBA, RGBW, RGBWA, RGBWA+UV e bianco, barra LED a 8 segmenti, teste spot, beam e wash, scanner, strobo, blinder, laser, macchina del fumo e hazer), più il **Generic Dimmer**.
- **La Open Fixture Library**, se la scarichi.
- **Le fixture GDTF** che scarichi da GDTF-Share o importi.
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

## GDTF-Share

GDTF-Share è il sito dove i produttori pubblicano le loro fixture in GDTF. Il programma lo consulta con il tuo account gratuito.

Si apre con **«GDTF-Share…»** in **Opzioni › Libreria di fixture › Fixture GDTF**, o con **«Cerca su GDTF-Share…»** in **Patch › Aggiungi fixture**. In basso, **«gdtf-share.com»** apre il suo sito.

### L'account

Premi **«Accedi»** e scrivi il nome utente e la password di gdtf-share.com. Vengono inviati solo a gdtf-share.com. Se non hai un account, **«Crea un account gratuito»** apre il suo sito.

- Senza **«Ricorda su questo computer»**, la password resta solo in memoria, finché chiudi il programma: viene chiesta una volta per sessione.
- Con questa opzione, viene salvata cifrata per il tuo utente Windows: chi usa quell'account Windows può accedere con essa.
- Il nome utente viene sempre ricordato.

Dopo l'accesso, il tasto mostra il tuo nome utente. Il suo menu ha **«Esci»** (la password salvata resta per la prossima volta) e, se l'hai salvata, **«Dimentica la password salvata»**, che la cancella dal computer. Dopo l'uscita con la password salvata, il tasto **«Accedi»** apre quel menu: **«Accedi»** e **«Dimentica la password salvata»**.

### Cercare

L'elenco di GDTF-Share viene salvato sul computer e funziona senza internet. La sua data è in alto («Elenco del…»). Viene scaricato di nuovo quando ha più di 20 ore, o con **«Aggiorna l'elenco»** (le frecce). Se GDTF-Share invia un elenco vuoto, resta quello precedente.

- Scrivi parte del produttore o del nome, in qualsiasi ordine.
- I filtri: **«Tutte»**, **«Dei produttori»** (caricate dal produttore stesso), **«Nella mia libreria»** e **«Aggiornamenti»** (quelle della tua libreria con una revisione più recente).
- A sinistra, i produttori di ciò che trovi e quante fixture ha ciascuno.
- Ogni fixture indica chi l'ha caricata (**Produttore** o **Utente**), la valutazione, e se è **Nella libreria**, ha una **Revisione più recente** o l'hai importata **Da un file**.
- Se ne mostrano al massimo 400: scrivi di più per restringere l'elenco.

### Scaricare e importare

Scegli una fixture: i suoi dettagli sono a destra (chi l'ha caricata, la revisione, la versione GDTF, la dimensione e i canali di ogni modalità).

1. Per una revisione precedente, sceglila in **Revisioni**.
2. Premi **«Scarica e importa»**. Viene scaricata e convertita come con «Importa GDTF», con una barra di avanzamento. **«Annulla»** la ferma. Se ne scarichi diverse insieme, quelle già arrivate vengono importate comunque. Il `.gdtf` originale viene conservato (vedi «Converti di nuovo», più sotto).
3. Da Aggiungi fixture, **«Patchala»** sceglie la fixture da patchare (anche una che era già nella tua libreria).

Se la fixture è già nella tua libreria, il tasto è **«Aggiorna a questa revisione»** o **«Scarica di nuovo»**. Se hai un altro caricamento della stessa fixture, un avviso lo dice: la nuova lo sostituisce.

### Revisioni più recenti

Una fixture della tua libreria ha una revisione più recente quando GDTF-Share ha una revisione successiva della stessa fixture. Contano quelle che hai scaricato da GDTF-Share e quelle importate da un file la cui ultima revisione ha lo stesso testo di una sola revisione di GDTF-Share.

- In GDTF-Share, **«Aggiornamenti»** le elenca e **«Aggiorna tutte»** scarica la revisione più recente di ciascuna. Le fixture patchate con esse la ricevono.
- In **Opzioni › Libreria di fixture**, la riga **«Revisioni più recenti su GDTF-Share»** dice quante sono, e **«Aggiorna»** apre GDTF-Share su «Aggiornamenti».
- Con la password salvata e **«Cerca aggiornamenti all'avvio»** attivo ([Opzioni](opciones.md#aggiornamenti)), il programma accede da solo una volta al giorno all'avvio, scarica l'elenco e avvisa se le fixture che hai scaricato da GDTF-Share hanno revisioni più recenti. Ogni revisione viene segnalata una volta. Se non ci riesce, non dice nulla.

### Converti di nuovo

Il programma conserva il `.gdtf` di ogni fixture che scarichi da GDTF-Share. **«Converti di nuovo»**, in **Opzioni › Libreria di fixture › Fixture scaricate da GDTF-Share**, le converte di nuovo con la versione del programma che hai, senza scaricarle: così ricevono i miglioramenti di una nuova versione. Le fixture patchate con esse ricevono la modifica. La riga compare quando ci sono fixture scaricate da GDTF-Share.

## Importare GDTF

GDTF è il formato aperto che i produttori pubblicano su GDTF-Share (con un account gratuito).

1. Scarica i `.gdtf` dei tuoi apparecchi da gdtf-share.com (o lo `.zip` del suo download multiplo). Per cercarli e scaricarli dal programma, vedi GDTF-Share, più sopra.
2. In **Opzioni › Libreria di fixture › Fixture GDTF**, premi **«Importa GDTF…»** e scegli i file, le cartelle o lo `.zip`.
3. Vengono convertiti in background, con una barra di avanzamento. **«Ferma»** interrompe l'importazione.

Alla fine viene indicato quante sono state convertite e, se qualcuna non si è potuta leggere, perché. Se la stessa fixture è già in un'altra libreria, quella GDTF viene aggiunta come «(GDTF)» e puoi usarle entrambe. Importarla di nuovo la aggiorna.

Una modalità con più *DMX break* (due ingressi DMX, o una parte su un altro universo) diventa più modalità. Il primo break mantiene il nome della modalità. Ognuno degli altri è un'altra modalità, per esempio «Standard (DMX break 2)», con solo i suoi canali, dall'indirizzo 1. Patchala come un'altra fixture, al suo indirizzo: **«Aggiungi fixture»** te lo ricorda quando scegli la modalità.

I canali virtuali (senza DMX) vengono omessi. Non servono: una fixture RGB senza dimmer si accende con i suoi colori, e il grand master li abbassa.

Alcuni canali cambiano funzione secondo il valore di un altro canale (per esempio, la rotazione del prisma gira solo con il prisma su «rotazione»). In questi canali si omettono le funzioni che non fanno nulla e, dove ce ne sono diverse, ogni tratto porta i loro nomi insieme (per esempio «0 deg / Stop»).

Per vedere queste modifiche su una fixture importata prima, importa di nuovo il suo `.gdtf`. Per quelle scaricate da GDTF-Share, premi «Converti di nuovo».

## Importare file di fixture

**Opzioni › Importa file di fixture › «Importa…»** accetta definizioni `.qxf` (il formato di QLC+), file `.gdtf` o uno `.zip` che li contenga. Vengono aggiunte alla libreria e viene indicato quante sono nuove.

### Importare le mie fixture di QLC+

Se hai creato fixture in QLC+ su questo computer, **Opzioni › «Importa le mie fixture di QLC+» › «Importa»** le copia tra le tue fixture.

## Formati che non si importano

Le librerie di altri programmi in formati chiusi o cifrati non si possono importare. La maggior parte delle fixture si trova nella Open Fixture Library o su GDTF-Share, che puoi consultare dal programma; quelle che mancano, creale in questa pagina.

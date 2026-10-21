# Manuale di DMX Total Control

DMX Total Control è un programma libero per controllare l'illuminazione DMX da un computer con Windows. È pensato per il live: avvii le scene per gruppi, programmi in fretta con il programmatore, gestisci le cuelist con GO nei playback e, se vuoi, costruisci la tua schermata touch (lo SHOW) e la usi anche dal telefono.

Con DMX Total Control puoi:

- Patchare le fixture del tuo impianto con una libreria di migliaia di definizioni, più la Open Fixture Library, l'importazione GDTF e le tue fixture.
- Dare dimmer, colore, posizione, gobo e qualsiasi altro canale alle fixture selezionate, con palette, FX e una riga di comando.
- Registrare scene statiche, passi, FX, super scene con timeline e audio, e cuelist con tracking.
- Avviarle in Live per gruppi, nei Playback con GO o dal tuo SHOW.
- Inviare il DMX tramite interfacce DMX USB, Art-Net o sACN, e controllare tutto da MIDI, OSC, ingresso DMX o tastiera.

Gli show si salvano in file `.qxw`, lo stesso formato che usa QLC+, quindi puoi aprire show fatti con QLC+.

## Come aprire questo manuale

Premi **F12** o il tasto **«?»** della barra superiore. Il manuale si apre sul lato destro della finestra, alla pagina che spiega la pagina in cui ti trovi: se sei in Programma, si apre su [Programma](programar.md). I link portano ad altre pagine del manuale; dal pannello puoi tornare alla pagina precedente o a questo «Indice». Si apre anche da Opzioni › Aiuto › **«Apri il manuale»**.

## La guida e gli show di esempio

- **La guida ai primi passi**: una scheda in basso a destra che ti accompagna tra le pagine in cinque passi (le fixture, un look, una scena, Live e i playback). Compare la prima volta che apri il programma; dopo, in Opzioni › Aiuto › **«Avvia la guida»**. Le pagine si possono usare mentre è aperta. Vedi [Primi passi](primeros-pasos.md).
- **Gli show di esempio**: show già pronti da aprire per fare prove. Si trovano nella freccia **▾** accanto ad «Apri», in **«Esempi»**:
  - **Primeros pasos**: lo show di prova della guida, con 4 PAR, 2 teste mobili e il gruppo «Mis escenas» vuoto.
  - **Bolo pequeño**: PAR davanti e dietro, strobo e macchina del fumo. Colori, effetti e generali in tre gruppi, e una pagina dello SHOW fatta con essi.
  - **Concierto con cabezas móviles**: palette di colore e di posizione, un cerchio a specchio e una canzone come cuelist nei playback.
  - **Sala con playbacks**: una cuelist per canzone nella prima pagina di playback e uno SHOW con quei playback.

Gli esempi si aprono come qualsiasi show: se salvi delle modifiche, usa «Salva come…» per lasciare l'esempio com'era.

## La finestra del programma

La finestra ha tre parti: la barra superiore, la barra laterale con le pagine e la pagina che stai usando.

### Le pagine

| Pagina | Tasto | A cosa serve |
|---|---|---|
| Live | F1 | Avviare le scene per gruppi e usare il mixer |
| Show | F2 | La tua schermata dello show: tasti, fader, manopole, gruppi, orologio… |
| Playback | F10 | Fader con GO, pausa e flash per cuelist e scene |
| Programma | F3 | Selezionare fixture, dare loro valori, FX e palette, e registrare |
| Scene | F4 | Creare e modificare scene, effetti, super scene e cuelist |
| Patch | F5 | Le fixture dell'impianto, i loro indirizzi, gruppi e pianta 2D |
| Fixture | F6 | Creare le tue definizioni di fixture |
| Uscite | F7 | Universi e interfacce DMX |
| Mappature | F8 | Controller MIDI, OSC, ingresso DMX e tastiera |
| Opzioni | F9 | Impostazioni del programma, libreria di fixture e aggiornamenti |

**F11** porta la finestra a schermo intero o la riporta alla sua dimensione. Se una pagina è bloccata, nella barra laterale ha un lucchetto e quando la apri viene chiesta la password, se c'è (vedi [Opzioni](opciones.md#show-e-blocco)).

In fondo alla barra laterale ci sono la versione del programma e la **CPU** che usa («CPU 12 %», di tutti i core del computer insieme). Sopra il 70 % diventa arancione, e sopra il 90 %, rossa: l'uscita può perdere il ritmo. Chiudi altri programmi o ferma alcuni effetti.

### La barra superiore

Da sinistra a destra:

- **Lo show**: il suo nome. Un punto **●** davanti avvisa che ci sono modifiche non salvate; se il salvataggio automatico è attivo, si vede anche l'ora dell'ultimo («salvato automaticamente…»).
- **File**: «Nuovo show (Ctrl+N)», «Apri (Ctrl+O)», la freccia **▾** con gli show recenti e gli «Esempi», «Salva (Ctrl+S)» e «Salva come…» (Ctrl+Maiusc+S).
- **Ritmo**: **«GO»** e **«Stop»** cambiano ciò che fanno i tasti delle scene (in Live, nello SHOW, nelle mappature e sul telefono). Con **GO** acceso, premere una scena la avvia e le pressioni successive ne battono il ritmo, senza mai fermarla; con **Stop** acceso, premere una scena la ferma; con entrambi spenti, come sempre ([Live](directo.md#ritmo-di-una-scena-a-passi)). Ogni tasto di una scena a passi o di una cuelist in riproduzione (in Live, nello SHOW, nei playback e sul telefono) contiene una **barra divisa in due** che accende un lato e poi l'altro a ogni passo: così vedi il ritmo di ciascuna. Poi **«BeatGO»** (divisione successiva delle scene guidate da BeatGO, anche con il tasto G) e **«Impulso»**, che ascolta l'ingresso audio del computer (si sceglie in [Opzioni](opciones.md#audio)): ogni picco che supera la linea del misuratore fa avanzare le scene guidate da Impulso. Trascina la linea del misuratore per cambiare la soglia. Il tempo dello show, quello degli FX e delle scene in modalità BPM, si cambia con un fader o una manopola di BPM dello [SHOW](show.md) o con le [mappature](mapeos.md).
- **Il lucchetto**: blocca le schermate perché niente cambi per errore (vedi [Opzioni](opciones.md#show-e-blocco)).
- **Il bersaglio**: la modalità Apprendi, per assegnare controlli MIDI, OSC o tasti (vedi [Mappature](mapeos.md#modalità-apprendi)). La sua spia verde lampeggia a ogni messaggio in arrivo.
- **«N in esecuzione»**: quante scene ed effetti sono in riproduzione.
- **GM**: il grand master. Sotto il 100 % diventa arancione.
- **«Blackout»** (Ctrl+B): manda tutto al buio finché è acceso.
- **«Ferma tutto»** (Esc): ferma tutto ciò che è in riproduzione. Il programmatore non viene svuotato.
- **«?»**: apre questo manuale (F12).

In una finestra stretta o su uno schermo piccolo, la barra si compatta: il nome dello show viene accorciato, «Salva come…» resta come icona, «N in esecuzione» si riduce al numero e, nelle più strette, «Blackout» e «Ferma tutto» restano solo con la loro icona. Passa il mouse sopra un'icona per vedere cos'è. Se tutto appare piccolo o grande, cambia la **Dimensione dell'interfaccia** in [Opzioni](opciones.md#generale).

Quando chiudi il programma, o apri o crei un altro show, se ci sono modifiche non salvate viene chiesto «Salvare le modifiche?» con «Annulla», «Non salvare» e «Salva». Alla chiusura, il programma ferma prima tutte le scene, i playback e il programmatore, e lascia che le uscite inviino il palco a riposo: molti apparecchi restano con l'ultima cosa che hanno ricevuto.

Se lo show che apri usa file che mancano (i suoi suoni, video o immagini; per esempio, uno show portato da un altro computer), compare un avviso con l'elenco. Lo show funziona senza di loro. Con **«Cerca in una cartella…»** scegli la cartella dove si trovano: vengono cercati per nome, anche nelle sottocartelle, e lo show passa a usarli (salvalo per mantenerlo così).

> **Consiglio:** se il programma si è chiuso in modo inatteso e avevi uno show non salvato, quando lo riapri ti propone «Recuperare lo show non salvato?». Premi «Recupera» e salvalo con un nome.

## Com'è organizzato questo manuale

Inizia da **Primi passi** se è la prima volta. Le altre pagine spiegano ogni pagina del programma e si possono leggere in qualsiasi ordine.

1. [Primi passi](primeros-pasos.md): una prima sessione completa, dal patch al salvataggio dello show.
2. [Live](directo.md): gruppi, esclusivo o mix, dissolvenza incrociata, master, rilasci, flash e mixer.
3. [Show](show.md): la schermata touch, le sue sezioni, tasti, fader, manopole, ruota dei colori, griglia XY, modifica e blocco.
4. [Playback](playbacks.md): pagine di playback, GO, Pausa/Indietro, Flash, livelli, registrare su un playback e controller APC.
5. [Programma](programar.md): selezione, gruppi, pianta 2D, dimmer, colore, posizione, funzioni, fader, palette, FX, ventaglio, cieco, evidenziare e localizzare, riga di comando, registrare e aggiornare.
6. [Scene](escenas.md): tutti i tipi di scena, super scene, cuelist con tracking, cartelle, importare ed esportare.
7. [Patch](patch.md): aggiungere fixture, indirizzi, universi, numeri, gruppi di fixture, pianta 2D, limiti, matrici LED e importare da un altro show.
8. [Fixture](fixtures.md): la libreria, il creatore di fixture, la Open Fixture Library, GDTF e le tue fixture.
9. [Uscite](salidas.md): universi, interfacce DMX USB, Art-Net e sACN, e il monitor dei livelli DMX.
10. [Mappature](mapeos.md): MIDI, OSC, ingresso DMX e tastiera, la modalità Apprendi e i modelli per Akai APC.
11. [Opzioni](opciones.md): lingua, tema, dimensione, avvio, blocco, salvataggio automatico, priorità, controllo remoto, libreria, aggiornamenti.
12. [Controllo remoto](remoto.md): lo SHOW sul telefono o sul tablet.
13. [Scorciatoie da tastiera](atajos.md): tutti i tasti e le combinazioni.

## Parole che vedrai spesso

- **Fixture**: ogni apparecchio luminoso patchato (un PAR, una testa mobile, una striscia LED…).
- **Universo**: 512 canali DMX. Ogni universo esce da un'interfaccia.
- **Programmatore**: dove dai valori alle fixture selezionate. Ciò che è nel programmatore prevale su tutto finché non lo svuoti.
- **Scena**: ciò che registri per riprodurlo dopo. Ce ne sono di vari tipi: statica, passi, FX, super scena, cuelist… (vedi [Scene](escenas.md)).
- **Gruppo**: una colonna di scene in Live. In un gruppo esclusivo è attiva una sola scena alla volta.
- **Gruppo di fixture**: un insieme di fixture da selezionare in un colpo solo (si creano in [Patch](patch.md#gruppi-di-fixture)).
- **Cuelist**: un elenco di cue che avanzano con GO. Ogni cue salva solo ciò che cambia.
- **Playback**: un fader con i suoi tasti (GO, Pausa/Indietro, Flash) che riproduce una cuelist o una scena.
- **Palette**: un valore salvato (un colore, una posizione…) che le scene seguono per riferimento: se cambi la palette, cambiano le scene che la usano.
- **Priorità**: da 0 a 10. Se due scene controllano lo stesso canale, vince quella con priorità più alta.
- **SHOW**: la schermata operativa che costruisci tu.

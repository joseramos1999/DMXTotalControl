# Mappature

![La pagina Mappature](../img/mapeos.png)

**Mappature** (F8) serve a controllare lo show dall'esterno: controller **MIDI**, **OSC** da un'altra applicazione, una console tramite **ingresso DMX** o la **tastiera**. Una mappatura collega un controllo del tuo dispositivo (un pad, un fader, un tasto…) a qualcosa del programma (una scena, un master, un GO…). Le mappature si salvano con lo show.

La pagina ha tre schede: **«Mappature»**, **«Dispositivi»** e **«Monitor»**, e il tasto **«Apprendi»**.

## Dispositivi

In **«Dispositivi»** compaiono i dispositivi di ingresso che il programma trova, con il loro tipo (MIDI, OSC o DMX), il loro nome e l'universo che usano. Prima vengono i controller (MIDI e OSC) e i dispositivi che usi già, sotto **«Controller e dispositivi in uso»**; poi, sotto **«Ingressi DMX: interfacce e rete»**, gli altri ingressi DMX, con una riga per ogni scheda di rete.

1. Collega il controller e, se non compare, premi **«Cerca di nuovo»**.
2. Attiva il suo **«Ingresso»**.
3. Se il dispositivo ha un'uscita, attiva **«Feedback (LED)»** perché il programma accenda i suoi LED o muova i suoi fader motorizzati.

Una mappatura di un dispositivo non collegato viene conservata: torna a funzionare quando lo colleghi.

## Modalità Apprendi

Il modo più rapido per creare mappature è la **modalità Apprendi**:

1. Attiva l'ingresso del tuo dispositivo in «Dispositivi».
2. Premi il **bersaglio** della barra superiore (o **«Apprendi»** in questa pagina). I controlli che si possono mappare si segnano in **viola**; un numero nell'angolo dice quante mappature hanno già.
3. Fai clic su un controllo del programma: una scena di Live, un master, un tasto del mixer, un playback…
4. La barra viola dice «Muovi un controllo del tuo dispositivo o premi un tasto per: …». Muovi il fader, premi il pad o premi un tasto.
5. Il programma avvisa «Mappato: …». Continua con il controllo successivo.
6. Premi **«Fatto»** (o Esc) per uscire.

Nella modalità Apprendi, **Maiusc+clic** continua a usare il controllo invece di mapparlo. **Esc** annulla il controllo in sospeso; un altro Esc esce dalla modalità. Il tasto «Mappature» della barra viola ti porta a questa pagina.

Un controllo del programma può avere più mappature (per esempio, un pad di un controller e un tasto della tastiera).

> **Consiglio:** quando apprendi un tasto di una Akai APC, la mappatura ha già i LED in verde durante la riproduzione.

### Cosa si può mappare

- **Barra superiore**: GO, Stop, BeatGO, grand master, Blackout e Ferma tutto.
- **Live**: ogni scena, ◀ ▶, Pausa e Rilascia gruppi dell'intestazione, e in ogni gruppo il suo rilascio, precedente, successiva e master.
- **Mixer**: in ogni striscia, il dimmer, la tonalità, la saturazione, lo strobo e la sua velocità, e i tasti Full, Blackout, Flash, Flash escl. e Full escl.
- **Programma**: le palette.
- **Playback**: i nomi delle pagine, ◀ ▶ di pagina, «Rilascia tutto», «Indietro», il GO grande, e in ogni playback la sua ×, il suo nome (selezionare), i suoi tre tasti e il suo fader.
- **SHOW**: i tasti e i fader (il fader intero, con tutti i suoi elementi), e ◀ ▶ delle sezioni di playback.

## L'elenco delle mappature

In **«Mappature»**, ogni riga mostra il controllo del dispositivo, ciò che controlla e il suo comportamento. Un punto viola indica un tasto e uno blu un fader. Fai clic su una mappatura per modificarla a destra, o premi **«Aggiungi»** per crearne una vuota.

## Modificare una mappatura

### Controllo

- **Il tipo**: **MIDI**, **OSC**, **«Ingresso DMX»** o **«Tastiera»**.
- **Dispositivo**: uno specifico o **«Qualsiasi dispositivo»**.
- **MIDI**: **CC** o **Nota**, il suo numero (0–127) e il **Canale** MIDI (1–16).
- **OSC**: il percorso, per esempio `/scene/1`.
- **Ingresso DMX**: il **Canale** (1–512).
- **Tastiera**: il tasto, per esempio `Q` o `F9`.
- **«Apprendi il controllo»**: premi e muovi il controllo del dispositivo per compilare tutto quanto sopra.

### Controlla

Ciò che fa la mappatura. I **tasti**:

| Destinazione | Cosa fa |
|---|---|
| Avvia una scena | La scena scelta, nel suo gruppo. |
| Scena N di un gruppo | La scena che si trova in quella posizione del gruppo (**Numero di scena**). |
| Rilascia un gruppo / Scena successiva di un gruppo / Scena precedente di un gruppo | In un gruppo o in «Tutti i gruppi». |
| Blackout, Ferma tutto, Metti tutto in pausa, Tap tempo, BeatGO, GO, Stop | Come i loro tasti (GO e Stop scelgono la modalità, come nella barra superiore: uno dei due è sempre acceso). |
| Palette | La palette sulle sue fixture o su un gruppo di fixture. |
| Mixer: full, blackout, flash, flash esclusivo, full esclusivo, strobo | Nella striscia scelta del mixer. |
| Playback: GO, indietro, pausa / indietro, flash, rilascia, seleziona | Su un playback. |
| Playback: vai a una cue | Salta alla **Cue** indicata (1 è la prima della lista). |
| GO del playback selezionato / Indietro del playback selezionato | Come il GO grande e «Indietro» di Playback. |
| Pagina di playback N / successiva / precedente | Cambia la pagina di playback. |
| Rilascia tutti i playback | Come «Rilascia tutto». |

I **fader**:

| Destinazione | Cosa muove |
|---|---|
| Grand master | Il GM. |
| Master di un gruppo | Il master di quel gruppo. |
| Dimmer dal vivo di una scena / Velocità dal vivo di una scena | I suoi cursori dal vivo. |
| BPM | Il tempo. |
| Mixer: dimmer, tonalità, saturazione, velocità strobo | Nella striscia scelta. |
| Fader dello SHOW | Un fader dello SHOW intero. |
| Playback: fader | Il fader di un playback. |

Per i playback si sceglie la **pagina** («Pagina attuale (segue)», che segue la pagina visibile, o una fissa) e il numero di **Playback**. Con **«O una cuelist o una scena, ovunque si trovi»** la mappatura agisce su quella cuelist o scena su qualsiasi playback si trovi. In «vai a una cue» si vede a quale cue porta ora («Ora: cue 3 di Canzone 1»).

### Comportamento

Nei **tasti**:

- **«Commuta: ogni pressione on / off»**: ogni pressione accende o spegne. Su una scena si chiama **«Premi: secondo GO / Stop»**: la avvia o la ferma secondo GO e Stop della barra superiore.
- **«Flash: finché è premuto»**.
- **«Solo accensione»** / **«Solo spegnimento»**.

I tasti di playback agiscono alla pressione, tranne il flash, e non hanno questa opzione.

Nei **fader**:

- **«Assoluto: segue il fader»**.
- **«Incrementale: un passo per pressione»** (per tasti che alzano o abbassano).
- **«Relativo: encoder infinito»** e **«Relativo: encoder, 1 su e 127 giù»** (encoder infiniti).
- **«Da»** … **«a»**: l'intervallo, da 0 a 100 %. **«Inverti»** lo capovolge.
- **«Passo %»** e **«In ciclo»** (incrementale e relativo).
- **«Aggancia il fader (senza motore)»** (assoluto): il fader del controller muove il livello solo dopo averlo raggiunto, senza salti.

### Feedback

Con MIDI o OSC, **«Feedback»** fa sì che il programma **accenda il tasto del dispositivo** o **muova il suo fader motorizzato**. Viene inviato solo ciò che cambia.

Per i LED dei tasti MIDI:

1. In **«LED di»**, scegli **«Qualsiasi controller (numeri)»** o il tuo controller: **Akai APC mini**, **Akai APC mini mk2** o **Akai APC40 mkII**.
2. Per **«Acceso»**, **«Spento»** e **«In pausa»**, scegli il colore tra i campioni (su una APC) o scrivi il valore (0–127) per qualsiasi altro controller.
3. Nei pad RGB della APC mini mk2, scegli anche come si accendono: **«Tenue»**, **«Pieno»**, **«Pulsante»** o **«Lampeggiante»** (sulla APC40 mkII, **«Fisso»**). Se non scegli nulla, «Acceso» si accende in **«Pieno»** e «Spento» in **«Tenue»**.

I LED del controller si spengono quando chiudi il programma, quando elimini una mappatura o la sposti su un altro controllo, quando apri un altro show o quando disattivi il dispositivo o il suo feedback.

### Duplicare ed eliminare

**«Duplica»** crea una copia della mappatura (utile per un altro dispositivo o canale) ed **«Elimina»** la cancella.

## Tastiera

Le mappature da tastiera funzionano in qualsiasi pagina e hanno la precedenza sulle scorciatoie del programma che usano lo stesso tasto. Non scattano mentre scrivi in un campo di testo (in Programma, mentre la riga di comando ha la tastiera) né con Ctrl o Alt premuti. I tasti dello SHOW hanno inoltre il loro tasto della tastiera, che funziona solo nella pagina Show (vedi [Show](show.md#opzioni-di-un-tasto)).

## Modelli per Akai APC

**«Mappa tutto»** assegna in un colpo solo un intero controller. Si trova in ogni dispositivo MIDI della scheda «Dispositivi»:

1. Premi **«Mappa tutto»** sul tuo controller.
2. Scegli il modello:
   - **«Akai APC mini → Gruppi»** / **«Akai APC mini mk2 → Gruppi»**
   - **«Akai APC mini → Playback»** / **«Akai APC mini mk2 → Playback»**
   - **«Akai APC40 mkII → Playback»**
3. Leggi cosa fa ogni controllo e premi **«Mappa»**. Le mappature che aveva quel dispositivo vengono sostituite; se il suo ingresso era spento, viene attivato.

**→ Gruppi** (APC mini e mk2): ogni colonna di pad avvia le scene di un gruppo (verde: in riproduzione; giallo: pronta; spento: non c'è scena). I fader sono i master dei gruppi e l'ultimo il grand master. I tasti sotto i pad rilasciano i gruppi; quelli a destra sono blackout, ferma tutto, pausa, tap, BeatGO, precedente, successiva e flash.

**→ Playback** (APC mini e mk2): ogni colonna è un playback 1-8 della pagina di playback attuale: il suo fader (aggancia il livello) e, dal basso, GO, Pausa/Indietro, Flash e le sue prime 5 cue (la cue 1 in alto). Il tasto sotto i pad lo rilascia. L'ultimo fader è il grand master; i tasti a destra sono pagina precedente e successiva, GO e Indietro del playback selezionato, blackout, tap, BeatGO e ferma tutto.

**→ Playback** (APC40 mkII): ogni traccia è un playback 1-8 della pagina attuale: il suo fader, GO sul tasto della traccia, Pausa/Indietro su S, Flash sull'armamento di registrazione, seleziona e rilascia, e le sue prime 5 cue sulle clip. I tasti di scena vanno alle pagine 1-5, quelli di banco cambiano la pagina e il gruppo, e Play e Record sono GO e Indietro del playback selezionato. Le manopole device sono i fader dei playback 9-16 e le manopole di traccia i master dei primi gruppi.

> **Consiglio:** dopo «Mappa tutto», ogni controllo compare come una normale mappatura nell'elenco: puoi cambiarne una qualsiasi, o cancellarla, senza toccare le altre.

## Monitor

**«Monitor»** mostra i **messaggi in entrata e in uscita**: l'ora, **IN** o **OUT**, il dispositivo (o «Tastiera»), il messaggio e il suo valore. Muovi un controllo di un dispositivo attivato per vedere cosa invia. **«Pulisci»** svuota l'elenco.

La spia verde del bersaglio, nella barra superiore, lampeggia a ogni messaggio in arrivo.

## Alt+clic

Fuori dalla modalità Apprendi, **Alt+clic** su un controllo che si può mappare lo porta nello SHOW: vedi [Show](show.md#portare-qualsiasi-controllo-nello-show).

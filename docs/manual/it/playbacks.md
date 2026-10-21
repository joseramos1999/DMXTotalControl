# Playback

![La pagina Playback](../img/playbacks.png)

**Playback** (F10) è una console di fader. Ogni **playback** riproduce una cuelist o qualsiasi scena, e ha un **GO**, un tasto **Pausa/Indietro**, un tasto **Flash** e un **fader** che è la sua intensità. I playback sono organizzati in **pagine** da 8 a 20 (10 di default).

È il modo più comodo per gestire uno show con cuelist: registri le cue dal programmatore direttamente su un playback e le avvii con GO o con la barra spaziatrice.

## La striscia di un playback

Dall'alto in basso:

- **Il numero** sul colore del playback. Si illumina durante la riproduzione. La **×** a destra lo rilascia.
- **Il nome**: fai clic su di esso per **selezionare** il playback (bordo arancione). Un playback senza niente mostra «Vuoto».
- **La cue in riproduzione e la successiva** («3 · Strofa», «▸ 4 · Ponte») e una barra con l'avanzamento della dissolvenza. Una cuelist ferma mostra «Rilasciata»; una scena mostra il suo tipo e se è «Acceso», «Spento» o «In pausa».
- **Tasto 1 (GO)**: la cue successiva; se il playback è spento, lo avvia.
- **Tasto 2 (Pausa / Indietro)**: mette in pausa una dissolvenza in corso; se non c'è dissolvenza, torna alla cue precedente.
- **Il fader** e la sua percentuale.
- **Tasto 3 (Flash)**: al massimo finché lo premi.

I tasti si illuminano secondo lo stato: **GO** in verde durante la riproduzione (lampeggia veloce durante una dissolvenza o un'attesa e lento in pausa), **Pausa/Indietro** in arancione in pausa e **Flash** in arancione finché è premuto.

### Il fader

Il fader è l'intensità di ciò che riproduce il playback:

- Quando lo **alzi da zero**, il playback parte.
- Quando lo **abbassi a zero**, una scena viene rilasciata; una cuelist resta sulla sua cue (si può cambiare nelle [proprietà](playbacks.md#proprietà-di-un-playback)).
- Un **GO con il fader abbassato** riproduce al massimo.

Il doppio clic sul fader lo porta al massimo o a zero.

## Mettere qualcosa su un playback

Ci sono vari modi:

- **Registrare dal programmatore**: vedi [Registrare su un playback](playbacks.md#registrare-su-un-playback).
- **«Aggiungi»** su un playback vuoto: **«Nuova cuelist dal programmatore»** o **«Metti qui una scena…»**.
- **Assegnare**: premi **«Assegna»** nell'intestazione. A destra compare l'elenco delle scene: trascinale sui playback (mentre l'elenco è aperto, una scena rilasciata su un playback occupato lo sostituisce) o fai doppio clic su una per metterla sul primo playback libero.
- **Clic destro › «Assegna una scena…»**: scegli la cuelist o la scena nell'elenco e premi **«Mettila sul playback N»** (o doppio clic).

Da Programma puoi anche registrare una nuova cuelist e metterla sul primo playback libero (vedi [Registrare](programar.md#registrare)).

## GO, Indietro e il playback selezionato

A destra dell'intestazione trovi i controlli del **playback selezionato**:

- **GO** (il grande tasto verde, o **Spazio**): la cue successiva del playback selezionato. Mostra il suo nome e quale cue viene dopo («Canzone 1 ▸ 3 · Ponte»).
- **«Indietro»** (**Maiusc+Spazio**): la cue precedente.
- **«Rilascia tutto»**: rilascia tutti i playback e tutte le cuelist in riproduzione.

Spazio e Maiusc+Spazio funzionano in questa pagina purché tu non stia scrivendo in un campo di testo. Ogni pressione dà un solo GO: tenere premuto il tasto non scorre le cue.

### Il pannello del playback selezionato

In basso si vede il playback selezionato («Playback N · nome»), il suo stato e i suoi tasti: indietro, **GO**, pausa / continua, rilascia e **«Modifica in Scene»**.

Se è una cuelist, compare l'elenco delle sue cue (CUE, NOME, AVVIO, RITARDO, FADE). La cue in riproduzione ha una barra verde e la successiva un bordo verde. In ogni riga:

- **«Avanti»**: quella cue sarà quella del prossimo GO.
- **«Vai»**: salta subito a quella cue.

Un clic su una riga la segna soltanto: non avvia niente per errore.

## Registrare su un playback

1. Prepara un look in [Programma](programar.md).
2. In Playback, premi **«Registra»**: lampeggia in rosso e ogni playback mostra cosa succederà quando fai clic.
3. Fai clic su un playback:
   - **Vuoto** («Nuova cuelist»): si crea una nuova cuelist con il programmatore come cue 1.
   - **Con una cuelist** («Registra la cue N»): si registra la sua cue successiva, solo con ciò che cambia rispetto a ciò che eredita.
   - **Con una scena** («Contiene una scena»): non ci si può registrare.

**Esc** annulla la registrazione armata, che si disarma anche quando esci dalla pagina.

### Aggiornare una cue

Cambia i valori nel programmatore, seleziona il playback e premi **«Aggiorna ▾»**:

- **«Aggiorna la cue N (vale anche per le successive)»**: la modifica passa alle cue successive che ereditano quei canali.
- **«Solo questa cue (le successive mantengono il loro aspetto)»**.

La cue che si aggiorna è quella caricata nel programmatore o, se non ce n'è nessuna, quella in riproduzione. Il menu del clic destro di ogni playback ha anche **«Registra la cue N»**, **«Aggiorna la cue attuale (vale anche per le successive)»** e **«Aggiorna solo la cue attuale»**.

> **Consiglio:** quando hai finito di registrare, premi «Azzera tutto» in Programma (o spunta «Svuota il programmatore dopo la registrazione»): finché il programmatore ha dei valori, prevale su ciò che avvii.

## Pagine

- Fai clic sul nome di una pagina per vederla. **◀ ▶** (o **PgUp** / **PgDn**) passano alla precedente o alla successiva.
- **«+ Pagina»** aggiunge una pagina.
- **«Fader per pagina»**: 8, 10, 12, 16 o 20 playback per pagina.
- Il doppio clic su una pagina la rinomina. Clic destro: **«Rinomina…»**, **«Sposta a sinistra»**, **«Sposta a destra»** e **«Elimina pagina…»** (con più di una pagina). Quando la elimini, i suoi playback vengono svuotati; ciò che riproducono continua e le cuelist e le scene restano in Scene.

## Proprietà di un playback

Clic destro › **«Proprietà…»**. Le modifiche si applicano subito.

- **Etichetta**: il nome che mostra (vuota: quello della scena).
- **Colore**: un campione, o la **A** per il colore del suo tipo di scena.
- **Tasto 1**, **Tasto 2** e **Tasto 3**: cosa fa ciascuno: «Niente», «GO», «Indietro», «Pausa», «Pausa / Indietro», «Rilascia», «Flash», «Flash + GO» (al massimo finché è premuto, e la cue successiva), «Acceso / spento» o «Seleziona» (perché Main GO, Registra e Aggiorna agiscano su di esso).
- **Il fader lo avvia**: se alzando il fader il playback parte.
- **Il fader a zero lo rilascia**: «Predefinito» (una scena sì, una cuelist no: mantiene la sua cue), «Sì» o «No».

Lo stesso menu ha **«Modifica in Scene»**, **«Rilascia»** e **«Svuota il playback»** (lo toglie dal playback; la cuelist o la scena resta in Scene).

## Rilasciare

- La **×** del numero di un playback in riproduzione, o **«Rilascia»** nel suo menu.
- **«Rilascia tutto»** nell'intestazione.

Rilasciare un playback rilascia anche quella scena se è in riproduzione in un gruppo di Live.

## Controller Akai APC

In [Mappature](mapeos.md#modelli-per-akai-apc) › «Dispositivi», **«Mappa tutto»** assegna in un colpo solo un intero controller ai playback:

- **Akai APC mini** e **APC mini mk2** → Playback: ogni colonna è un playback 1-8 della pagina attuale: il suo fader e, dal basso, GO, Pausa/Indietro, Flash e le sue prime 5 cue. Il tasto sotto i pad lo rilascia, l'ultimo fader è il grand master e i tasti a destra sono pagina precedente e successiva, GO e Indietro del playback selezionato, blackout, tap, BeatGO e ferma tutto.
- **Akai APC40 mkII** → Playback: ogni traccia è un playback 1-8, con le sue prime 5 cue sulle clip; le manopole device sono i fader dei playback 9-16.

I LED mostrano lo stato di ogni playback.

## Playback nello SHOW e nel controllo remoto

- Lo [SHOW](show.md#playback-nello-show) può avere una sezione di playback che segue la pagina attuale o ne mostra una fissa.
- Nel [controllo remoto](remoto.md), quando un playback contiene qualcosa, compare una pagina «Playback» con le sue strisce.

## Con la programmazione bloccata

Se blocchi la **«Programmazione»** con il lucchetto della barra superiore, i playback continuano a funzionare (GO, fader, flash), ma non si può registrare, aggiornare, assegnare né cambiare le pagine o le proprietà.

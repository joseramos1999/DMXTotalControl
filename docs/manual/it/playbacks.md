# Playback

![La pagina Playback](../img/playbacks.png)

**Playback** (F10) è una console di fader. Ogni **playback** riproduce una cuelist o qualsiasi scena, e ha un **GO**, un tasto **Pausa/Indietro**, un tasto **Flash** e un **fader** che è la sua intensità. I playback sono organizzati in **pagine** da 8 a 20 (10 di default).

È il modo più comodo per gestire uno show con cuelist: registri le cue dal programmatore direttamente su un playback e le avvii con GO o con la barra spaziatrice.

## La striscia di un playback

Dall'alto in basso:

- **Il numero** sul colore del playback. Si illumina durante la riproduzione. La **×** a destra lo rilascia. Una cuelist di un altro tipo mostra la sua etichetta: **SUB** (submaster), **CHASE**, **TC** (timecode) o **OVR** (override).
- **Il nome**: fai clic su di esso per **selezionare** il playback (bordo arancione). Un playback senza niente mostra «Vuoto».
- **La cue in riproduzione e la successiva** («3 · Strofa», «▸ 4 · Ponte») e una barra con l'avanzamento della dissolvenza. Se la successiva è il salto di un collegamento della cue, porta **↺** e il numero del salto: «▸ 2 ↺ 1/3» è il primo di tre (vedi [Cuelist](escenas.md#cuelist)). Un chase mostra prima la sua velocità: «♩ 120 ▸ 4». Una cuelist timecode mostra prima il suo orologio: «TC 00:01:23.4 · 3 · Strofa». Una cuelist ferma mostra «Rilasciata»; una scena mostra il suo tipo e se è «Acceso», «Spento» o «In pausa».
- **Tasto 1 (GO)**: la cue successiva; se il playback è spento, lo avvia.
- **Tasto 2 (Pausa / Indietro)**: mette in pausa una dissolvenza in corso; se non c'è dissolvenza, torna alla cue precedente.
- **Il fader** e la sua percentuale. Un fader A/B mostra «A/B» e a che punto è la sua dissolvenza.
- **Tasto 3 (Flash)**: al massimo finché lo premi (vedi [Il flash](playbacks.md#il-flash)).

I tasti si illuminano secondo lo stato: **GO** in verde durante la riproduzione (lampeggia veloce durante una dissolvenza o un'attesa e lento in pausa), **Pausa/Indietro** in arancione in pausa e **Flash** in arancione finché è premuto e fino alla fine della sua discesa.

### Il flash

Finché premi **Flash**, il playback va al massimo. Quando lo lasci, torna al livello del suo fader. Se era spento, il flash lo avvia e lo rilascia quando lo lasci.

Con un **attacco** e una **discesa** nelle sue [proprietà](playbacks.md#proprietà-di-un-playback), il cambio non è immediato:

- Quando premi, sale al massimo nel tempo dell'attacco, da dove si trova. Un playback spento sale da zero.
- Quando lasci, torna al livello del fader nel tempo della discesa. Un playback avviato dal flash scende a zero e viene rilasciato alla fine della discesa.
- Se premi di nuovo durante la discesa, risale da dove si trova, senza scendere prima.
- Se muovi il fader durante la discesa, la discesa finisce al nuovo livello.
- Un GO durante la discesa lo lascia acceso: la discesa finisce al livello del fader.

I canali che non sono di intensità (posizione, ruota colori) entrano subito, come sempre.

### Il fader

Il fader è l'intensità di ciò che riproduce il playback:

- Quando lo **alzi da zero**, il playback parte.
- Quando lo **abbassi a zero**, una scena viene rilasciata; una cuelist resta sulla sua cue (si può cambiare nelle [proprietà](playbacks.md#proprietà-di-un-playback)).
- Un **GO con il fader abbassato** riproduce al massimo.

In una cuelist **submaster** comanda il fader: alzarlo da zero la avvia al livello del fader, mai al massimo, e abbassarlo a zero la rilascia. Rilasciata, GO e Indietro scelgono solo la cue da cui partirà (vedi [Opzioni della cuelist](escenas.md#opzioni-della-cuelist)).

In una cuelist **override**, il fader porta in dissolvenza tutto ciò che controlla, anche colori, posizioni e gobo, tra ciò che c'è sotto e la sua cue. Al massimo, la sua cue sostituisce ciò che c'è sotto (vedi [Opzioni della cuelist](escenas.md#opzioni-della-cuelist)).

In una cuelist normale, il fader può essere una **dissolvenza incrociata (A/B)** («Fader» nelle [proprietà](playbacks.md#proprietà-di-un-playback)). Allora non è l'intensità: il playback riproduce al livello che aveva, e ogni corsa del fader da un estremo all'altro è un GO di cui fai la dissolvenza a mano.

- Appena lascia il suo estremo, parte la cue successiva e la sua dissolvenza segue il fader: il 40 % della corsa è il 40 % della dissolvenza. I ritardi, le discese e i tempi di attributi e fixture seguono in proporzione.
- All'altro estremo la cue è completa. La corsa successiva, nel senso opposto, lancia la cue dopo.
- Se torni all'estremo da cui sei partito, si annulla e torna la cue precedente. Se quella corsa ha avviato la cuelist, viene rilasciata.
- Una cuelist ferma parte dalla cue di un GO, che entra con il fader. Una cue senza tempo fa la dissolvenza in modo lineare lungo la corsa.
- Gli FX della cue partono con il loro tempo.
- **GO** (il tasto, Main GO o una mappatura) lancia la cue successiva con il suo tempo. A metà di una corsa, finisce quella dissolvenza con il suo tempo e non lancia un'altra cue.
- Segui e Attendi della cue successiva aspettano la fine della corsa. La velocità delle cue e il tempo dal vivo non cambiano la dissolvenza che fai a mano.

Il doppio clic sul fader lo porta al massimo o a zero. Su un fader A/B è una corsa intera: la cue si completa subito.

## Mettere qualcosa su un playback

Ci sono vari modi:

- **Registrare dal programmatore**: vedi [Registrare su un playback](playbacks.md#registrare-su-un-playback).
- **«Aggiungi»** su un playback vuoto: **«Nuova cuelist dal programmatore»** o **«Metti qui una scena…»**.
- **Assegnare**: premi **«Assegna»** nell'intestazione. A destra compare l'elenco delle scene: trascinale sui playback (mentre l'elenco è aperto, una scena rilasciata su un playback occupato lo sostituisce) o fai doppio clic su una per metterla sul primo playback libero.
- **Clic destro › «Assegna una scena…»**: scegli la cuelist o la scena nell'elenco e premi **«Mettila sul playback N»** (o doppio clic).

Da Programma puoi anche registrare una nuova cuelist e metterla sul primo playback libero (vedi [Registrare](programar.md#registrare)).

Anche le cue list di QLC+ possono arrivare ai playback: vedi [Scene › Cue list di QLC+](escenas.md#cue-list-di-qlc).

## GO, Indietro e il playback selezionato

A destra dell'intestazione trovi i controlli del **playback selezionato**:

- **GO** (il grande tasto verde, o **Spazio**): la cue successiva del playback selezionato. Mostra il suo nome e quale cue viene dopo («Canzone 1 ▸ 3 · Ponte»).
- **«Indietro»** (**Maiusc+Spazio**): la cue precedente.
- **«Rilascia tutto»**: rilascia tutti i playback e tutte le cuelist in riproduzione.
- **«Velocità delle cue»**: la velocità di tutte le cuelist (vedi [Velocità delle cue](playbacks.md#velocità-delle-cue)).

Spazio e Maiusc+Spazio funzionano in questa pagina purché tu non stia scrivendo in un campo di testo. Ogni pressione dà un solo GO: tenere premuto il tasto non scorre le cue.

### Il pannello del playback selezionato

In basso si vede il playback selezionato («Playback N · nome»), il suo stato e i suoi tasti: indietro, **GO**, pausa / continua, rilascia e **«Modifica in Scene»**.

Se è una cuelist, compare l'elenco delle sue cue (CUE, NOME, AVVIO, RITARDO, FADE). La cue in riproduzione ha una barra verde e la successiva un bordo verde. In ogni riga:

- **«Avanti»**: quella cue sarà quella del prossimo GO.
- **«Vai»**: salta subito a quella cue.

Un clic su una riga la segna soltanto: non avvia niente per errore.

## Velocità delle cue e tempo dal vivo

### Velocità delle cue

**«Velocità delle cue»**, nell'intestazione, cambia la velocità di tutte le cuelist insieme: le loro dissolvenze, i ritardi, Segui e Attendi, il loro ritmo battuto e i passi di un chase. Va da ×0,1 a ×10, con ×1 al centro.

- Agisce subito, anche sulle dissolvenze in corso: accelerano o rallentano da dove sono, senza salti.
- Il valore diventa arancione quando non è ×1. Un doppio clic lo riporta a ×1.
- Non cambia i FX né le altre scene.
- Non si salva: ogni show si apre a ×1.

È anche nei fader dello [SHOW](show.md#fader) e nelle [mappature](mapeos.md#controlla).

### Tempo dal vivo

Mentre una cue è in dissolvenza, il pannello del playback selezionato mostra **«Tempo dal vivo»**: il ritardo più la dissolvenza di quella cue (per esempio, «4,0 s»). Puoi cambiarlo mentre scorre:

- Scrivi un altro tempo, o premi **«×½»** o **«×2»**: la dissolvenza continua da dove si trova e finisce nel nuovo tempo.
- **«Adesso»** termina subito la dissolvenza.
- Se la cue successiva ha **Segui**, aspetta la nuova fine.

La modifica vale solo per questa volta: la cue successiva usa i propri tempi. «Tempo dal vivo» diventa arancione quando il tempo non è quello della cue. **«Salva nella cue»** salva nella cue i tempi cambiati (anche quelli dei suoi attributi e fixture). Non si può quando la cue va con un tempo di BACK o con la dissolvenza d'entrata di un gruppo o di un blocco, che non sono i suoi tempi. Si annulla come qualsiasi altra modifica della cuelist.

La stessa barra è nell'intestazione della cuelist in [Scene](escenas.md#la-tabella-delle-cue). Da un controller, usa le mappature «Tempo dal vivo di un playback» o «Tempo dal vivo del playback selezionato» (vedi [Mappature](mapeos.md#controlla)).

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
- **Attacco del flash** e **Discesa del flash**: quanto impiega il flash a salire al massimo e a tornare quando lo lasci, da 0 a 10 s (0 = subito). Valgono per il suo tasto Flash o Flash + GO, le mappature, lo SHOW e il controllo remoto (vedi [Il flash](playbacks.md#il-flash)).
- **Fader**: «Intensità» (predefinito) o «Dissolvenza incrociata (A/B)», solo su una cuelist normale (vedi [Il fader](playbacks.md#il-fader)). In A/B le due opzioni seguenti non compaiono: il fader la avvia sempre e non la rilascia mai.
- **Il fader lo avvia**: se alzando il fader il playback parte.
- **Il fader a zero lo rilascia**: «Predefinito» (sì per una scena e una cuelist submaster; no per le altre cuelist: mantengono la loro cue), «Sì» o «No».

Lo stesso menu ha **«Modifica in Scene»**, **«Rilascia»** e **«Svuota il playback»** (lo toglie dal playback; la cuelist o la scena resta in Scene).

## Rilasciare

- La **×** del numero di un playback in riproduzione, o **«Rilascia»** nel suo menu.
- **«Rilascia tutto»** nell'intestazione.

Rilasciare un playback rilascia anche quella scena se è in riproduzione in un gruppo di Live.

## Controller Akai APC

In [Mappature](mapeos.md#modelli-per-akai-apc) › «Dispositivi», **«Mappa tutto»** assegna in un colpo solo un intero controller ai playback:

- **Akai APC mini** e **APC mini mk2** → Playback: ogni colonna è un playback 1-8 della pagina attuale: il suo fader e, dal basso, GO, Pausa/Indietro, Flash e le sue prime 5 cue. Il tasto sotto i pad lo rilascia, l'ultimo fader è il grand master e i tasti a destra sono pagina precedente e successiva, GO e Indietro del playback selezionato, blackout, tap, BeatGO e ferma tutto.
- **Akai APC40 mkII** → Playback: ogni traccia è un playback 1-8, con le sue prime 5 cue sulle clip (la successiva pulsa in verde); le manopole device sono i fader dei playback 9-16.

I LED mostrano lo stato di ogni playback.

## Playback nello SHOW e nel controllo remoto

- Lo [SHOW](show.md#playback-nello-show) può avere una sezione di playback che segue la pagina attuale o ne mostra una fissa.
- Nel [controllo remoto](remoto.md), quando un playback contiene qualcosa, compare una pagina «Playback» con le sue strisce.

## Con la programmazione bloccata

Se blocchi la **«Programmazione»** con il lucchetto della barra superiore, i playback continuano a funzionare (GO, fader, flash), ma non si può registrare, aggiornare, assegnare né cambiare le pagine o le proprietà.

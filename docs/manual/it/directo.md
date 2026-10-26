# Live

![La pagina Live](../img/directo.png)

**Live** (F1) è la pagina per avviare le scene durante lo show. Le scene sono ordinate in **gruppi**, colonne che funzionano in parallelo: ogni gruppo ha le sue scene, la sua modalità (esclusivo o mix), il suo tempo di dissolvenza incrociata e il suo master.

Nell'intestazione della pagina trovi:

- **◀ ▶**: scena precedente o successiva in ogni gruppo attivo (anche con Ctrl+← e Ctrl+→).
- **«Pausa»** / **«Riprendi»**: congela tutto ciò che è in riproduzione e lo fa ripartire.
- **«Rilascia gruppi»**: ferma le scene di tutti i gruppi.
- **«Comprimi gruppi»** / **«Espandi gruppi»**: comprime tutte le colonne in una volta, per vederle tutte; quando sono già tutte compresse, le espande.
- **«Mixer»**: passa alla vista del [mixer](directo.md#mixer). Un punto arancione avvisa che il mixer sta modificando l'uscita.
- **«Nuovo gruppo»**: aggiunge una colonna.

## Gruppi

### Creare e organizzare i gruppi

1. Premi **«Nuovo gruppo»** (o il riquadro «Aggiungi gruppo» alla fine delle colonne).
2. Fai clic sul nome della colonna per cambiarlo (per esempio `Wash`, `Spot`, `Strobo`, `Effetti`) e premi Invio.

Il tasto con l'ingranaggio di ogni colonna apre il suo menu: **«Cambia colore»** (passa al colore successivo), **«Sposta a sinistra»**, **«Sposta a destra»** e **«Elimina gruppo»**. Puoi anche trascinare il gruppo dalla maniglia a sinistra del suo nome e rilasciarlo su un altro gruppo per prenderne il posto.

La freccia **‹** accanto al nome **comprime** il gruppo: diventa una striscia stretta con il suo nome e quante scene sono in riproduzione, per fare spazio agli altri. Le sue scene restano in riproduzione. La freccia **›** della striscia lo espande. Si salva con lo show. Per comprimerli o espanderli tutti in una volta, usa **«Comprimi gruppi»** / **«Espandi gruppi»** nell'intestazione.

### Aggiungere e ordinare le scene

1. Premi **«Aggiungi scene»** alla fine della colonna.
2. Nell'elenco, fai clic sulle scene che vuoi (si segnano con ✓). Puoi cercare per nome o per cartella e vederle per cartelle o per gruppi.
3. Premi **«Aggiungi N»**. Con un doppio clic su una scena, la aggiungi direttamente.

Per riordinare, passa il mouse su un riquadro e trascinalo dalla sua maniglia (a destra del riquadro): rilascialo su un'altra scena per metterlo davanti, o nello spazio vuoto della colonna per metterlo in fondo. Puoi rilasciarlo in **un altro gruppo** e si sposta con le sue opzioni. Nel menu del riquadro (clic destro) trovi anche **«Sposta su»**, **«Sposta giù»** e **«Rimuovi dal gruppo»**.

> **Consiglio:** quando registri da Programma puoi mettere la scena direttamente in un gruppo con «Aggiungi al gruppo». Vedi [Registrare](programar.md#registrare).

## Avviare le scene

Fai clic su un riquadro per avviare la scena (con **GO**, la modalità con cui parte il programma). Per rilasciarla, premi il quadrato nell'intestazione del gruppo («Rilascia questo gruppo») o passa a **Stop** e premila. Il riquadro in riproduzione si colora con il colore del gruppo e ha un punto che pulsa.

In ogni riquadro si vede:

- Il **nome** e, sotto, il **tipo** di scena. In una cuelist in riproduzione, la cue in cui si trova («Q 3 · Strofa»). Dopo, il [ritmo battuto](directo.md#ritmo-di-una-scena-a-passi), se ce l'ha («120 BPM»).
- **P7**, **P2**…: la priorità, solo quando non è quella normale (5). In arancione se è maggiore di 5.
- Un **fulmine**: è una scena flash.
- Una **barra divisa in due** in basso: una scena a passi, una cuelist o un FX in «BPM dello show» in riproduzione; si alterna a ogni passo (in un FX, a ogni battito), al suo ritmo.

### Esclusivo o mix

Il tasto di ogni colonna alterna tra:

- **«Esclusivo»**: è attiva una sola scena alla volta. Quando ne avvii un'altra, fa una dissolvenza incrociata verso la nuova.
- **«Mix»**: le scene si sommano; puoi averne diverse in riproduzione contemporaneamente.

### Tempo di dissolvenza incrociata

Il menu a tendina accanto a «Esclusivo»/«Mix» è il tempo di dissolvenza del gruppo:

- **«Per scena»**: ogni scena usa le sue dissolvenze in entrata e in uscita.
- **«Taglio»**: senza dissolvenza.
- **Da 0,5 s a 10 s**: quel tempo per tutte le scene del gruppo, al posto dei loro.

### Scena precedente e successiva

I tasti **◀ ▶** di ogni colonna avviano la scena precedente o successiva del gruppo, nell'ordine della colonna. Quelli nell'intestazione della pagina fanno lo stesso in tutti i gruppi attivi.

### Scene flash

Una scena con **flash** è attiva solo finché tieni premuto il suo riquadro. Si attiva nelle sue [proprietà](directo.md#proprietà-della-scena) con «Flash (attiva finché è premuto)».

### Cuelist in un gruppo

Una cuelist in un gruppo è un riquadro come gli altri: il clic la avvia (con **Stop**, la ferma). Il suo menu (clic destro) aggiunge **«GO (cue successiva)»**, **«Indietro (cue precedente)»** e **«Rilascia la cuelist»**. Per gestirla comodamente con GO, mettila su un [playback](playbacks.md).

### Ritmo di una scena a passi

Le scene a passi (sequenze e chaser), le cuelist e le scene FX con degli FX in «BPM dello show» seguono il ritmo con cui premi il loro riquadro:

- Un clic la avvia (con **GO**, la modalità con cui parte il programma; vedi sotto).
- Con la scena in riproduzione, premi il suo riquadro **più volte di seguito** a tempo di musica, con meno di 2 secondi tra una pressione e l'altra. Ogni pressione passa al passo successivo (in una cuelist, dà un GO) e la scena prosegue da sola a quel ritmo: un passo a ogni battito, al posto dei suoi tempi. Il ritmo è la media delle ultime pressioni. Una pressione che arriva un po' in ritardo, subito dopo il passo di quel battito (meno di mezzo battito), è quello stesso passo: non ne dà un altro, rimette solo a tempo il ritmo.
- Contano solo le pressioni con la scena in riproduzione: quella che la avvia inizia il conteggio. Così, cambiare scena in un gruppo esclusivo e tornare indietro non dà un ritmo a nessuna.
- Dopo una pausa di più di 2 secondi, un clic dà un passo senza cambiarne il ritmo, e la scena continua a suonare. Per fermarla, passa a **Stop** e premila.
- Si riproduce il ritmo che hai battuto, anche con una **velocità dal vivo** impostata; se poi muovi la velocità, il ritmo accelera o rallenta con essa. In pausa, le pressioni non fanno avanzare i passi.
- Il ritmo si salva con la scena ed è suo: lo segue anche quando è in riproduzione in una super scena o su un playback. Nelle sue [proprietà](directo.md#proprietà-della-scena) (e in [Scene](escenas.md#come-si-riproduce-una-scena)) puoi scriverlo o premere **«Rimuovi»** perché torni ai suoi tempi.
- In una **scena FX**, il ritmo è quello dei suoi FX in **«BPM dello show»**: la prima pressione di una serie fa ripartire il loro ciclo (come quando la avvii), e ognuna delle successive li porta al battito successivo, un ciclo più avanti per il loro moltiplicatore (un chase passa alla fixture successiva; a ×¼, avanzano di un quarto di ciclo). Più pressioni di seguito danno loro un ciclo a ogni battito (per il loro moltiplicatore), invece dei BPM dello show. Gli FX con una velocità propria non cambiano. Senza ritmo battuto, o dopo «Rimuovi», seguono i BPM dello show. Se disattivi «BPM dello show» in tutti i suoi FX, la scena perde il suo ritmo battuto.

Funziona allo stesso modo con i suoi tasti dello [SHOW](show.md), con le sue mappature di controller e tasti in modalità «Commuta» e con il [controllo remoto](remoto.md).

#### GO e Stop

I tasti **GO** e **Stop** della barra superiore (e della sezione «Ritmo» dello SHOW) decidono cosa fa una pressione su qualsiasi scena, per chiunque la prema. Uno dei due è sempre acceso: il programma parte sempre in **GO**, e premere l'altro cambia modalità:

- **GO** acceso: premere una scena la **avvia**; in una già in riproduzione, ogni pressione fa avanzare un passo (in una cuelist, un GO; in una scena FX, la prima fa ripartire il ciclo dei suoi FX in «BPM dello show» e le successive li portano al battito successivo) e più pressioni di seguito ne battono il ritmo. Non la ferma mai, anche se passano più di 2 secondi senza premere: così batti il tempo con calma.
- **Stop** acceso: premere una scena la **ferma**. Una che non è in riproduzione resta com'è.
- Per rilasciare le scene di un gruppo senza cambiare modalità, premi il quadrato nella sua intestazione («Rilascia questo gruppo»); **«Ferma tutto»** (Esc) ferma tutto.

Le scene flash continuano a funzionare finché le tieni premute, in qualsiasi modalità.

Durante la riproduzione, ogni riquadro di una scena a passi, di una cuelist o di una scena FX con degli FX in «BPM dello show» ha in basso una **barra del ritmo** divisa in due: si accende un lato e poi l'altro a ogni passo (in una cuelist, a ogni cue; in una scena FX, a ogni battito, un ciclo di quegli FX a ×1). Così vedi a che ritmo va ciascuna. Ce l'hanno anche i suoi tasti dello SHOW, le sue strisce di playback e i suoi tasti sul telefono.

## Master

- **Master del gruppo**: il cursore «Master» in fondo a ogni colonna scala l'intensità di tutte le sue scene.
- **Grand master**: il «GM» della barra superiore scala la luce di tutta l'uscita: i dimmer e, nelle fixture senza dimmer, i loro colori. Non tocca la miscelazione del colore (ciano, magenta, giallo, tinta e saturazione) né i colori di una fixture o di una testa con un dimmer proprio, che scendono con quel dimmer. In Opzioni decidi se agisce solo sull'intensità o su tutti i canali.
- **Dimmer dal vivo**: ogni scena ha inoltre il suo dimmer, nelle sue proprietà, sopra il master del gruppo.

I master e il grand master non influiscono sui canali che hanno disattivata l'opzione «Dimmer» nei [limiti della fixture](patch.md#limiti). Se è il dimmer di una fixture, non conta più come suo dimmer proprio: il grand master abbassa allora i suoi colori.

## Proprietà della scena

Fai clic destro su un riquadro e scegli **«Proprietà…»**.

| Campo | Cosa fa |
|---|---|
| **Nome** | Il nome della scena. |
| **Entrata** / **Uscita** | I suoi tempi di dissolvenza, in secondi. Se il gruppo ha un tempo di dissolvenza incrociata impostato, si usa quello. |
| **Priorità (0-10)** | Quale scena vince quando due toccano lo stesso canale (vedi [Priorità](directo.md#priorità)). |
| **All'avvio, rilascia** | Cosa ferma quando parte: [modalità di rilascio](directo.md#modalità-di-rilascio-e-protezione). |
| **Protetta dai rilasci di** | Quali rilasci ignora. |
| **Flash (attiva finché è premuto)** | La trasforma in scena flash. |
| **Dimmer dal vivo** | La sua intensità, sopra il master del gruppo. |
| **Ritmo battuto** | Passi, cuelist e scene FX con degli FX in «BPM dello show»: il [ritmo battuto](directo.md#ritmo-di-una-scena-a-passi) con il suo riquadro, in BPM. Scrivilo o premi «Rimuovi» per tornare ai suoi tempi (in una scena FX, ai BPM dello show). Agisce subito. |
| **Velocità dal vivo** | Da ×0,1 a ×10 (scene che si muovono: passi, FX, cuelist…). |
| **Fase dal vivo** / **Dimensione dal vivo** | La fase (0–359°) e la dimensione (0–200 %) degli FX. |
| **Velocità di dissolvenza** | Accelera o rallenta le sue dissolvenze, da ×0,1 a ×10. |

I cursori dal vivo agiscono subito, mentre li muovi, e cambiano come suona la scena in quel gruppo, non la scena registrata. Gli altri campi si applicano premendo **«Applica»**.

Lo stesso menu del riquadro ha **«Apri in Scene»**, **«Modifica nel programmatore»** (scene statiche) e il sottomenu **«Priorità: N»** per cambiarla senza aprire le proprietà.

## Priorità

Ogni scena ha una priorità da 0 a 10; le nuove prendono quella che indichi in [Opzioni](opciones.md#riproduzione) (5 di default).

- La scena con **priorità più alta comanda su tutti i canali che tocca**, zeri compresi: una scena rossa con priorità 8 sopra una blu con priorità 5 dà rosso, non magenta.
- Con la **stessa priorità**, nei canali di intensità (dimmer, RGB…) si mescolano e vince il valore più alto; negli altri canali vince l'ultima scena avviata.
- Se cambi la priorità di una scena in riproduzione, si applica subito.
- Quando rilasci con dissolvenza una scena con priorità più alta, fa una dissolvenza verso ciò che c'è sotto.
- **Il programmatore prevale su tutto**, intensità comprese. In Opzioni puoi abbassare la priorità delle modifiche dal vivo.

## Modalità di rilascio e protezione

**All'avvio, rilascia** decide quali scene si fermano quando parte questa:

- **«Quella del gruppo»**: in un gruppo esclusivo, le altre del suo gruppo; in uno mix, nessuna.
- **«Nessuno»**: non ferma niente.
- **«Il suo gruppo»**: le altre scene del suo gruppo.
- **«Tutte le scene»**: tutte le altre.
- **«Gli altri gruppi»**: le scene degli altri gruppi.

**Protetta dai rilasci di** fa sì che la scena ignori quei rilasci:

- **«Nessuno»**: viene rilasciata come sempre.
- **«Tutto (si ferma solo a mano)»**: la fermi solo tu.
- **«Il proprio gruppo»**: ignora quelli del suo gruppo.
- **«Gli altri gruppi»**: ignora quelli che vengono da altri gruppi.

> **Consiglio:** metti una scena d'ambiente con protezione «Tutto (si ferma solo a mano)» e avvia sopra scene con «Tutte le scene»: l'ambiente resta.

## Pausa, rilascio e arresto

- **«Pausa»** congela tutto ciò che è in riproduzione. **«Riprendi»** lo fa continuare.
- **«Rilascia gruppi»** ferma le scene di tutti i gruppi; il quadrato di ogni colonna, solo quelle di quel gruppo.
- **«Ferma tutto»** (Esc), nella barra superiore, ferma tutto ciò che è in riproduzione, anche fuori dai gruppi. Non svuota il programmatore.
- **«Blackout»** (Ctrl+B) manda l'uscita al buio senza fermare niente. Le teste non si muovono: il pan, il tilt, lo zoom, il fuoco e gli altri canali che non sono luce restano dove sono.

## Mixer

Il mixer agisce su ciò che esce, sopra ogni scena e le modifiche dal vivo. Premi **«Mixer»** per vederlo: c'è una striscia per **«Tutte le fixture»** e una per ogni [gruppo di fixture](patch.md#gruppi-di-fixture).

Ogni striscia ha:

- Un **fader di dimmer** con la sua percentuale.
- **«Full»**: il gruppo al massimo. **«Blackout»**: il gruppo al buio.
- **«Tonalità»** e **«Saturazione»**: tingono il colore. **«Ripristina colore»** toglie la tinta.
- **«Strobo»** e la sua velocità, da 0,5 a 20 Hz.
- **«Flash»**: al massimo finché è premuto. **«Flash escl.»**: quel gruppo al massimo e il resto al buio finché è premuto.
- **«Full escl.»**: quel gruppo al massimo e il resto al buio.

Ctrl+clic su un tasto flash lo lascia acceso fino al clic successivo. **«Ripristina il mixer»** riporta tutto allo stato normale. Il bordo di una striscia diventa arancione con un flash o un full, e rosso con il blackout.

> **Consiglio:** il mixer può anche essere una sezione dello [SHOW](show.md), e i suoi fader e tasti si possono [mappare](mapeos.md) su un controller MIDI.

## Controlli nello SHOW e nei controller

I riquadri delle scene, i master dei gruppi, i tasti ◀ ▶ e di rilascio di ogni gruppo, quelli dell'intestazione (◀ ▶, «Pausa» e «Rilascia gruppi») e i controlli del mixer:

- Con **Alt+clic** si copiano nella pagina dello SHOW aperta (vedi [Show](show.md#portare-qualsiasi-controllo-nello-show)).
- Nella modalità Apprendi si possono assegnare a un controller (vedi [Mappature](mapeos.md#modalità-apprendi)).

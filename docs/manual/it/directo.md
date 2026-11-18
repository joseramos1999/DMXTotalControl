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

- **«Esclusivo»**: è attiva una sola scena alla volta. Quando ne avvii un'altra, fa una dissolvenza incrociata verso la nuova. I [fader dello SHOW](show.md#fader) che avviano scene sono l'eccezione: aggiungono la loro, quindi puoi alzarne diversi dello stesso gruppo insieme.
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
- **Grand master**: il «GM» della barra superiore scala la luce di tutta l'uscita: i dimmer e, nelle fixture senza dimmer, i loro colori. Non tocca la miscelazione del colore (ciano, magenta, giallo, tinta e saturazione) né i colori di una fixture o di una testa con un dimmer proprio, che scendono con quel dimmer. In Opzioni decidi se agisce solo sull'intensità o su tutti i canali. Non abbassa le fixture **protette dal grand master** (nei [limiti della fixture](patch.md#limiti) o nel suo menu): la luce di sala, un media server, un hazer. Se ce n'è qualcuna, un'**ancora** accanto a «GM» lo ricorda (se la barra ha spazio) e, passando il mouse, dice quante sono («Il grand master non abbassa 3 fixture che ne sono protette»).
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
- Con la **stessa priorità**, nei canali di intensità (dimmer, RGB…) si mescolano e vince il valore più alto; negli altri canali vince l'**ultimo cambiamento**: l'ultima scena avviata o l'ultimo GO di una cuelist che ha cambiato quel canale. Ciò che una cuelist mantiene solo dalle cue precedenti non lo riprende.
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

Quando la avvia un [fader dello SHOW](show.md#fader), non rilascia niente, qualunque sia la sua modalità: si aggiunge alle altre. Con il suo pulsante, rilascia come sempre. E finché quel fader è alzato, niente la ferma, se non abbassarlo fino in fondo.

**Protetta dai rilasci di** fa sì che la scena ignori quei rilasci:

- **«Nessuno»**: viene rilasciata come sempre.
- **«Tutto (si ferma solo a mano)»**: la fermi solo tu.
- **«Il proprio gruppo»**: ignora quelli del suo gruppo.
- **«Gli altri gruppi»**: ignora quelli che vengono da altri gruppi.

> **Consiglio:** metti una scena d'ambiente con protezione «Tutto (si ferma solo a mano)» e avvia sopra scene con «Tutte le scene»: l'ambiente resta.

## Pausa, rilascio e arresto

- **«Pausa»** congela tutto ciò che è in riproduzione. **«Riprendi»** lo fa continuare.
- **«Rilascia gruppi»** ferma le scene di tutti i gruppi; il quadrato di ogni colonna, solo quelle di quel gruppo. Per 10 s, [«Recupera ciò che suonava»](directo.md#recupera-ciò-che-suonava) riavvia ciò che ha fermato «Rilascia gruppi» (il suo tasto, o un pad o un tasto dello SHOW in «Tutti i gruppi»); ciò che viene rilasciato in un solo gruppo (il suo quadrato, o un pad o un tasto dello SHOW di quel gruppo) o da un'azione di una cue non si può recuperare.
- **«Ferma tutto»** (Esc), nella barra superiore, ferma tutto ciò che è in riproduzione, anche fuori dai gruppi. Non svuota il programmatore. Né «Ferma tutto» né «Rilascia gruppi» fermano le scene che un [fader dello SHOW](show.md#fader) alzato trattiene: solo abbassarlo fino in fondo. Disarma anche gli [effetti speciali](directo.md#effetti-speciali-protetti), e per 10 s si può [recuperare ciò che suonava](directo.md#recupera-ciò-che-suonava). Con [«Ferma tutto con dissolvenza»](directo.md#ferma-tutto-con-dissolvenza), le funzioni scendono prima di fermarsi.
- **«Blackout»** (Ctrl+B) manda l'uscita al buio senza fermare niente. Le teste non si muovono: il pan, il tilt, lo zoom, il fuoco e gli altri canali che non sono luce restano dove sono. Disarma anche gli effetti speciali. Per abbassare la luce poco a poco e finire in Blackout, usa una [«Dissolvenza al nero»](directo.md#dissolvenza-al-nero).

## Sicurezza durante lo show

Perché un errore o un guasto del computer non rovini lo show: la [Modalità show](directo.md#modalità-show), gli [effetti speciali protetti](directo.md#effetti-speciali-protetti), i [canali parcheggiati](directo.md#canali-parcheggiati), l'[arresto dello show](directo.md#arresto-dello-show), [recupera ciò che suonava](directo.md#recupera-ciò-che-suonava), [ferma tutto con dissolvenza](directo.md#ferma-tutto-con-dissolvenza) e la [dissolvenza al nero](directo.md#dissolvenza-al-nero). Le loro impostazioni sono in [Opzioni › Sicurezza durante lo show](opciones.md#sicurezza-durante-lo-show).

> **Importante:** il computer non deve mai essere l'unica sicurezza di una macchina di CO2, di fiamme, di scintille fredde o di coriandoli. Imposta le macchine e i nodi DMX perché si spengano se perdono il segnale DMX (e non perché mantengano l'ultimo valore), metti un arresto di emergenza o un pulsante a uomo morto hardware dove la macchina ne ha bisogno e segui le regole del luogo e le istruzioni del produttore. Se il programma si blocca o il computer si spegne, nessun software può interrompere un'attivazione: solo l'impianto può farlo.

### Modalità show

Lo **scudo** della barra superiore, subito dopo il lucchetto, attiva o disattiva la **Modalità show** con un clic. Attiva, è arancione, e la seconda riga del titolo comincia con «Modalità show ·». Bloccare lo SHOW (anche all'avvio con «…e bloccato») la attiva da sola; sbloccarlo non la disattiva. Con lo SHOW bloccato, lo scudo si vede ma non si può premere. È un'impostazione del computer, non dello show.

Con la Modalità show attiva, queste azioni chiedono di tenere premuto un tasto per 1,5 s («Tieni premuto per chiudere», «Tieni premuto per aprire» o «Tieni premuto per continuare»), anche senza modifiche non salvate:

- chiudere il programma (la × della finestra, Alt+F4, la barra delle applicazioni) e installare un aggiornamento;
- **Nuovo** e **Apri** (anche Ctrl+N e Ctrl+O), gli show recenti, gli esempi e lo show di prova della guida;
- aprire uno show da Esplora file di Windows.

Rilasciare prima del tempo, **«Annulla»** o Esc annullano; Esc in quella finestra non ferma mai lo show. Vale anche tenere premuto Spazio o Invio (la ripetizione del tasto non conta). Poi, se ci sono modifiche non salvate, arriva «Salvare le modifiche?» come sempre. Quando Windows si spegne o chiudi la sessione con la Modalità show attiva, la domanda può trattenere lo spegnimento: chiudi prima il programma.

In Modalità show, se il portatile passa alla batteria, una barra rossa «A batteria: collega il caricatore» compare in tutte le pagine (anche bloccato e a schermo intero) finché non torna il caricatore; al 20 % o meno aggiunge «Batteria al N %». La batteria si controlla ogni 30 s e a ogni avviso di alimentazione di Windows. Senza la Modalità show, un avviso lo dice una volta (e un'altra quando è quasi scarica), e [Opzioni](opciones.md#sicurezza-durante-lo-show) lo mostra.

Se la combinazione per il risparmio di energia di Windows ha attiva la sospensione selettiva USB, all'avvio una barra blu avvisa che l'interfaccia DMX può fermarsi, con **«Come disattivarla»** (apre [Sospensione selettiva USB](opciones.md#sospensione-selettiva-usb)) e una × che la nasconde fino al prossimo avvio. Il programma legge soltanto queste impostazioni di Windows: non le cambia mai.

### Effetti speciali protetti

Un **effetto speciale protetto** è una fixture che non può attivarsi per errore. Le macchine di CO2, di fiamme, di scintille fredde e di coriandoli lo sono da sole, e qualsiasi fixture può esserlo a mano (vedi [Patch › Effetti speciali protetti](patch.md#effetti-speciali-protetti)). Hanno uno scudo nell'elenco e nella pianta del Patch.

- **Disarmati** (il programma parte sempre così), tutti i loro canali escono a 0, qualunque cosa suoni.
- **Armati**, escono solo da **scene** (anche flash), **cuelist**, **chaser**, **sequenze**, **super scene** e dai valori base delle **scene FX**. Così li attivi con un tasto dello SHOW in flash (si attiva finché lo tieni premuto), un riquadro di Live, un pad dell'APC, un playback o il [telefono](remoto.md#sicurezza-sul-telefono).
- **Mai**, nemmeno armati, li attivano: il programmatore, Identifica, Evidenzia, il mixer (Full, Flash, colore, strobo), gli FX e le loro macro, i Move FX, i Colour FX e i mapping della pianta, i Curve FX, gli script né un ingresso DMX che passa all'uscita. Il grand master non li scala.
- I [limiti](patch.md#limiti) contano solo mentre si attivano: a riposo esce sempre 0. Non invertire i canali di una macchina per effetti: un canale di attivazione invertito si attiva solo con un valore che, invertito, accende.
- I loro canali di pan e tilt (quelli di una testa a fiamma) escono solo armati e da quelle funzioni, senza tempo massimo; disarmati, a 0. Tutti gli altri, tranne quello di sicurezza, si attivano e a loro si applica il tempo massimo (vedi [Patch › Effetti speciali protetti](patch.md#effetti-speciali-protetti)).

#### Arma effetti

Il tasto **«Arma effetti»** (con una fiamma) è nella barra superiore, accanto a «Blackout», solo quando lo show ha effetti protetti, anche con lo SHOW bloccato. Un clic li arma e un altro li disarma. Armati, il tasto diventa rosso con il tempo che manca prima che si disarmino da soli («ARMATI · 9:59»), e il suo bordo pulsa mentre uno si attiva. In una finestra più stretta mostra «Arma» o solo la fiamma: sotto i 1450 px di larghezza, «Arma effetti», «Blackout» e «Ferma tutto» restano solo con la loro icona, e sotto i 1200 px circa il numero in esecuzione si nasconde e il grand master si accorcia.

Armati, una grande barra rossa **«EFFETTI ARMATI»** compare in tutte le pagine (anche bloccato e a schermo intero) con ciò che si attiva («In azione: CO2 1») o ciò che aspetta il rilascio, e un tasto **«Disarma»**.

La freccia **▾** accanto mostra se sono armati e quanto manca perché si disarmino da soli, **«In azione ora»**, **«Aspettano il rilascio»** (rilascia la sua scena, il suo tasto o il suo fader, a 0, e potrà attivarsi), **«Ultime attivazioni»** (nome · ora · secondi · come è finita: rilasciato, interrotto al suo tempo massimo, disarmato…), **«Apri il registro»** e **«Impostazioni di ogni effetto: Patch»** (questi due, non con lo SHOW bloccato).

Non si armano se nessuna fixture è protetta, con il Blackout attivo («Il Blackout è attivo: disattivalo per armare gli effetti speciali») né con l'arresto dello show attivo. Si armano anche con un [tasto dello SHOW](show.md#azioni-di-un-tasto), una [mappatura](mapeos.md#controlla) o il [telefono](remoto.md#sicurezza-sul-telefono).

#### Tempo massimo e attesa minima

- Un'attivazione dura finché la sua scena, il suo tasto o il suo fader la danno, al massimo il suo **tempo massimo** (per impostazione predefinita: CO2 3 s, fiamme 2 s, scintille 5 s, coriandoli 2 s). Arrivato lì si interrompe, anche se continui a premere, e non torna finché non rilasci.
- Tra un'attivazione e la successiva c'è un'**attesa minima** (1 s per impostazione predefinita) dal rilascio. Una pressione durante l'attesa non si attiva dopo: rilascia e premi di nuovo.
- Armare mentre una scena o un fader danno già un valore all'effetto non lo attiva: aspetta che tu rilasci. Disarmare mentre uno si attiva lo interrompe, anche se riarmi subito (non torna finché non rilasci); un comando che arma soltanto non fa nulla se sono già armati.
- Il **canale di sicurezza**, se la macchina ne ha uno, esce nei suoi valori solo mentre l'effetto è armato e si attiva; altrimenti 0. Da solo non attiva mai la macchina.
- Se il programma smette di inviare fotogrammi (è molto occupato), ogni universo interrompe comunque un'attivazione al suo tempo, al massimo con 40 ms di ritardo.

#### Quando si disarmano

Si disarmano da soli:

- con **Ferma tutto** (Esc, il suo tasto, un pad, un tasto dello SHOW o il telefono) e con il **Blackout** (Ctrl+B, il suo tasto, un tasto dello SHOW, un pad o il telefono; non il «Blackout» di una striscia del mixer);
- con l'**arresto dello show**, una **dissolvenza al nero** (quando comincia) e **recupera ciò che suonava**;
- quando apri o inizi **un altro show**, e quando **chiudi** il programma;
- dopo alcuni **minuti senza attivazioni** (10 per impostazione predefinita: «Disarma gli effetti non usati dopo», in [Opzioni](opciones.md#sicurezza-durante-lo-show)), mai mentre uno si attiva;
- quando l'ultimo effetto protetto viene eliminato o non è più protetto, e **a mano**.

Un avviso lo dice («Effetti speciali disarmati: Ferma tutto»). Non si salvano mai armati. Un fader dello SHOW che trattiene una scena con un CO2 continua a trattenerla dopo Esc, ma il CO2 resta a 0 finché non abbassi il fader fino in fondo.

#### Il registro delle attivazioni

Ogni sessione scrive un file in `%LOCALAPPDATA%\DMXTotalControl\DMX Total Control\effects-log` («Apri il registro» apre la cartella): ogni attivazione con la sua ora, il numero e il nome della fixture, quanto è durata e come è finita (rilasciato, interrotto al suo tempo massimo, disarmato, arresto dello show, parcheggiato, rimosso), e quando gli effetti sono stati armati e disarmati, e perché.

> **Non provato con apparecchi reali:** niente di tutto questo è stato provato con macchine di CO2, di fiamme, di scintille o di coriandoli, con i loro canali di sicurezza, con nodi Art-Net o sACN né con ciò che fanno quando perdono il segnale DMX. Provalo con la tua attrezzatura, senza pubblico e con la macchina scarica, prima di usarlo in uno show.

### Canali parcheggiati

Un canale **parcheggiato** esce sempre con un valore fisso, sopra le scene, il programmatore, il mixer, il grand master e Ferma tutto: l'hazer al 30 % per tutta la notte, una testa guasta spenta e ferma. Esc e Blackout non lo tolgono mai dal parcheggio; con il Blackout le sue intensità si spengono, tranne con «Anche in blackout». Si parcheggia nel [Patch](patch.md#parcheggiare-i-canali) e in [Programma](programar.md#parcheggiare). Quando si apre uno show con canali parcheggiati, si applicano subito, e un avviso dice quali («2 fixture parcheggiate: HAZER, Spot 3 (Patch)»).

### Arresto dello show

L'**arresto dello show** è una luce di sicurezza per fermare lo show e dare luce alle persone: sopra tutto, mette bianco aperto sulle luci, spegne strobo e laser, tiene ferme le teste e disarma gli effetti speciali.

Per **attivarlo o toglierlo**, premi **il tasto Pausa due volte** (in meno di 2 s; non in uno show che lo mappa, vedi [Mappature](mapeos.md#tastiera)), o **tieni premuto per 1 s** un tasto dello SHOW «Arresto dello show (tenere premuto 1 s)», un pad o un tasto della tastiera [mappati](mapeos.md#controlla), o la mano della barra del [telefono](remoto.md#sicurezza-sul-telefono). Una pressione più breve non fa niente, e una sola pressione del tasto Pausa avvisa soltanto («Arresto dello show: premi di nuovo Pausa per attivarlo»). Mentre tieni premuto, una barra arancione «Arresto dello show: continua a tenere premuto…» si riempie. Attivo, una grande barra rossa **«ARRESTO DELLO SHOW»** compare in tutte le pagine (anche bloccato e a schermo intero) con **«Tieni premuto per togliere»** (tienilo premuto un secondo). Se il programma perde la tastiera mentre tieni premuto un tasto mappato (un'altra finestra, un avviso di Windows), il tasto viene rilasciato e non cambia nulla.

Mentre è attivo:

- gli effetti speciali si disarmano («…disarmati: arresto dello show») e non si possono armare («L’arresto dello show è attivo: toglilo per armare gli effetti speciali»);
- le luci (teste mobili, scanner, PAR e wash, barre LED, blinder e dimmer) dei gruppi scelti vanno a **bianco aperto** al livello scelto: il dimmer al livello, il rosso, il verde, il blu e il bianco al massimo (senza dimmer, i colori al livello), CMY, ambra e UV a 0, lo shutter aperto, la ruota colori su bianco o aperta e i programmi interni su «nessuna funzione»;
- gli strobo si spengono (le loro intensità a 0, o tutti i loro canali se non ne hanno) e i laser vanno a 0 su tutti i canali (un laser DMX si spegne a 0: verificalo con il tuo);
- tutti gli shutter si aprono (senza strobo; un canale di strobo «da lento a veloce» va a 0) e lo strobo del mixer si ferma;
- le teste mobili restano dove sono (pan e tilt, anche quelli fini);
- il fumo, gli hazer, i ventilatori, le bolle, la neve, gli altri effetti, i gobo, i prismi e lo zoom restano come sono, e gli effetti protetti a 0.

Vince sul programmatore, sul grand master, sul Blackout (il resto resta al buio) e sui canali parcheggiati: una fixture parcheggiata spenta si illumina anch'essa, e un hazer parcheggiato resta parcheggiato. I [limiti](patch.md#limiti) contano ancora (un dimmer invertito, un massimo). Quali fixture illumina e a che livello si sceglie in [Opzioni › Arresto dello show](opciones.md#arresto-dello-show).

- **Ferma tutto** (Esc) durante l'arresto ferma le funzioni sotto, e l'arresto resta.
- Quando lo **togli**, lo show è come lo suonano ora le sue funzioni (hanno continuato sotto) e lo strobo del mixer torna. Gli effetti restano disarmati: armali a mano; un CO2 trattenuto va rilasciato prima.
- **Nuovo**, **Apri** e la chiusura del programma lo tolgono. Non si salva mai attivo: dopo una chiusura inattesa o un riavvio, attivalo di nuovo.
- Il registro degli effetti della sessione annota «Arresto dello show attivato» e «Arresto dello show tolto».

> **Non provato con apparecchi reali:** il bianco aperto e i valori di shutter delle fixture reali, che un laser resti davvero spento a 0, il tasto Pausa delle tastiere reali, i LED dell'APC e il telefono su un Wi-Fi reale.

### Recupera ciò che suonava

Dopo **Ferma tutto** (Esc, il suo tasto, un pad, un tasto dello SHOW o il telefono) o **«Rilascia gruppi»** (tutti i gruppi, non uno solo), una barra blu sotto la barra superiore dice cosa si è fermato («Fermato: 4 scene e 1 cuelist · Recupera ciò che suonava (Ctrl+Z)»; «Rilasciato: …» dopo «Rilascia gruppi»), con il tasto **«Recupera»**, una striscia che si svuota in 10 secondi e una × che la nasconde fino al prossimo Ferma tutto. Compare anche con lo SHOW bloccato. **Ctrl+Z** fa lo stesso in Live e nello SHOW (non mentre lo modifichi né mentre scrivi), e la barra dice «(Ctrl+Z)» solo dove funziona. «Recupera» ignora un tocco nel suo primo mezzo secondo (un secondo tocco di panico su Ferma tutto).

Tornano, con una dissolvenza di 0,5 s:

- la scena che suonava ogni gruppo;
- le scene avviate dai tasti dello SHOW, da Scene o dai playback;
- le cuelist, sulla cue in cui erano, al livello del loro playback; una cuelist in pausa torna in pausa.

Gli FX e i passi ricominciano il loro ciclo, le funzioni in beat usano la loro dissolvenza e le cuelist «Timecode» seguono il loro timecode. Non tornano: i flash che si tenevano premuti (in Live, nello SHOW, sui pad o nei playback), le scene che un fader dello SHOW trattiene (non si sono mai fermate), il programmatore, il mixer, il grand master, l'armamento degli effetti, i canali parcheggiati né l'arresto dello show.

Funziona una volta, e un avviso dice cosa è tornato («Recuperato: 4 scene e 1 cuelist»). Un altro Ferma tutto entro i 10 s si somma al primo (non il secondo Esc durante la dissolvenza di «Ferma tutto con dissolvenza»); «Rilascia gruppi» tiene solo i gruppi, e un Ferma tutto dopo si somma. Le funzioni eliminate nel frattempo si saltano, e uno show nuovo o aperto lo cancella.

Recuperare **non arma mai** gli effetti speciali: se erano armati (dopo «Rilascia gruppi», che non disarma, o quando li hai armati di nuovo dopo Ferma tutto), prima li disarma («Effetti speciali disarmati: Recupera ciò che suonava»). Ciò che torna con un CO2 aspetta che tu rilasci e resta a 0, anche dopo averli armati di nuovo.

### Ferma tutto con dissolvenza

Con **«Ferma tutto con dissolvenza»** in [Opzioni](opciones.md#sicurezza-durante-lo-show) (da 0 a 5 s; 0, subito, come sempre), Ferma tutto abbassa le funzioni in quel tempo. Le scene trattenute dai fader dello SHOW e i canali parcheggiati restano; gli effetti protetti si spengono subito (si disarmano). Durante la dissolvenza, il tasto «Ferma tutto» ha una striscia e dice «Dissolvenza in corso: Esc di nuovo ferma subito»: **Esc di nuovo** ferma tutto subito, anche ciò che hai avviato durante la dissolvenza. Dopo si può recuperare ciò che il primo Esc ha fermato; ciò che hai avviato durante la dissolvenza non torna.

### Dissolvenza al nero

Una **«Dissolvenza al nero»** (un [tasto dello SHOW](show.md#azioni-di-un-tasto) o una [mappatura](mapeos.md#controlla), con il suo tempo: 3 s per impostazione predefinita) abbassa le intensità in uscita in quel tempo e poi attiva il Blackout, per esempio alla fine di un set. Sotto, la luce torna al massimo: togliendo il Blackout, torna.

- Agisce sopra il grand master (anche sulle fixture che ne sono protette) e sui canali parcheggiati senza «Anche in blackout».
- Non tocca i parcheggiati con «Anche in blackout», il pan, il tilt né altri canali che non sono luce, né gli effetti protetti.
- Quando comincia, disarma gli effetti speciali.
- Un'altra pressione, o il Blackout, la terminano subito. Con il Blackout già attivo, non fa niente.
- Durante la dissolvenza, il tasto «Blackout» ha una striscia e dice «Dissolvenza al nero: alla fine arriva il Blackout (Blackout ora: subito)».

> **Non provato con apparecchi reali:** la Modalità show (la batteria, il computer sveglio, la sospensione USB), Recupera, Ferma tutto con dissolvenza e la Dissolvenza al nero non sono stati provati con portatili, interfacce DMX, LED dell'APC né macchine per effetti reali.

## Mixer

Il mixer agisce su ciò che esce, sopra ogni scena e le modifiche dal vivo. Premi **«Mixer»** per vederlo: c'è una striscia per **«Tutte le fixture»** e una per ogni [gruppo di fixture](patch.md#gruppi-di-fixture).

Ogni striscia ha:

- Un **fader di dimmer** con la sua percentuale.
- **«Full»**: il gruppo al massimo. **«Blackout»**: il gruppo al buio, tranne gli effetti speciali protetti (nessun tasto del mixer li spegne né li disarma).
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

Mentre una mappatura usa «La scena selezionata», o nella modalità Apprendi, il riquadro della scena selezionata (l'ultima che hai premuto) ha una barra sottile sul bordo inferiore (vedi [Mappature](mapeos.md#la-scena-selezionata)).

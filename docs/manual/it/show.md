# Show

![La pagina Show](../img/show.png)

La pagina **Show** (F2) è la tua schermata operativa: la costruisci tu, su misura, con tasti, fader, manopole, gruppi, playback, orologio, note… È pensata per gli schermi touch, per restare a schermo intero durante lo show e per usarla anche dal telefono con il [controllo remoto](remoto.md). Si salva con lo show.

Uno SHOW ha delle **pagine**. Ogni pagina è fatta di **sezioni** che riempiono lo schermo. Ogni sezione si può dividere in due metà, affiancate o una sopra l'altra, tutte le volte che vuoi, e ogni sezione mostra una cosa: una tabella di tasti, una tabella di fader, un gruppo, un orologio…

## Iniziare

Con uno show nuovo la pagina è vuota e offre due tasti:

- **«Nuova pagina»**: crea una pagina con una tabella di tasti ed entra nella modalità modifica.
- **«Parti dai miei gruppi»**: crea la pagina «Gruppi» con tutti i tuoi gruppi di Live in alto e, in basso, una tabella di fader «Master» (il grand master e i master dei primi gruppi), una sezione Ritmo e un orologio. Compare solo se hai già dei gruppi.

## La modalità modifica

Premi **«Modifica»** per costruire la schermata e **«Fatto»** per tornare a usarla. Nella modalità modifica:

- Fai clic su una sezione per **selezionarla** (in una tabella, su una delle sue celle o sulla sua etichetta in alto a destra). Le sue proprietà compaiono nel pannello **«Proprietà»** a destra.
- Passando il mouse su una sezione, e su quella selezionata, compare in alto a destra un'etichetta con il suo tipo e tre tasti: **dividi in due metà affiancate**, **dividi una sopra l'altra** e **rimuovi questa sezione** (in una sezione con titolo, sono nella sua barra del titolo). Gli stessi sono nelle proprietà: «Affiancate», «Una sopra l'altra» e «Rimuovi sezione».
- Le sezioni mantengono la loro altezza: non ricevono una barra mentre modifichi. Con il pannello delle proprietà, si restringono solo fino alla larghezza in cui le loro tabelle stanno senza scorrere; se così non ci stanno, la pagina scorre di lato con la barra in basso (al massimo è larga come senza il pannello).
- Trascina il **separatore** tra due metà per cambiarne la dimensione. Si aggancia da solo a quarti, terzi e metà.
- Una sezione **«Vuota»** mostra dei tasti con tutti i tipi: fai clic su uno per scegliere cosa mostra. Se la sezione è piccola e non ci stanno, resta un solo tasto, **«Cosa mostra ▾»**, con l'elenco.

### Tipi di sezione

| Tipo | Cosa mostra |
|---|---|
| **Tasti** | Una tabella di tasti (colonne × righe). |
| **Fader** | Una fila di fader. |
| **Un gruppo** | Un gruppo di Live con tasti grandi. |
| **Tutti i gruppi** | Tutti i gruppi di Live, uno accanto all'altro. |
| **Playback** | I playback della pagina Playback, con il loro GO, i loro tasti e il loro fader. |
| **Mixer dal vivo** | Il [mixer](directo.md#mixer) di Live. |
| **Orologio** | L'ora, con secondi e data facoltativi. |
| **Timer** | Un cronometro o un conto alla rovescia. |
| **Ritmo: GO e Stop** | **GO** e **Stop** grandi, come quelli della barra superiore (cambiano ciò che fanno i tasti delle scene), e **BeatGO**. I tasti di scene a passi, di cuelist o di FX in «BPM dello show» contengono la loro barra del ritmo. |
| **Note** | Un tuo testo, della dimensione che vuoi. |
| **In riproduzione ora** | Ciò che è in riproduzione, con un tasto per fermarlo. |
| **Manopole e ruote** | Una tabella di manopole o di rotelle infinite. |
| **Ruota dei colori** | Una ruota per dare colore a un gruppo di fixture. |
| **Griglia XY (pan e tilt)** | Un'area per muovere le teste mobili. |
| **Immagine** | Un'immagine (un logo, una pianta…). |
| **Livelli DMX** | I 512 canali di un universo, dal vivo. |
| **Vuota** | Niente (nella modalità modifica, l'elenco dei tipi). |

### Proprietà della sezione

A seconda del tipo, il pannello delle proprietà mostra:

- **Mostra**: il tipo di sezione.
- **Titolo**: il testo della sua barra del titolo. Senza titolo, la sezione non ha la barra, nemmeno in modalità modifica.
- **Sfondo**: il colore di sfondo della sezione, tra gli stessi colori dei tasti, oppure **∅** (quello del tema). Si vede come una tinta, in modifica, quando suoni e sul telefono, e il testo sopra resta leggibile.
- **Colore di tutti i tasti** (di tutti i fader, di tutte le manopole): dà quel colore a tutti quelli della tabella in una volta. Le celle vuote restano vuote.
- **Colonne** e **Righe**: la dimensione della tabella (fino a 32). Tasti, fader e manopole riempiono le loro celle. In una sezione troppo piccola per la sua tabella, le celle non si rimpiccioliscono oltre quanto si può leggere e premere: la tabella scorre (con la rotella o trascinando).
- **Riempi con le scene di un gruppo** (tasti): mette in un colpo solo un tasto per ogni scena di quel gruppo.
- **Gruppo** (un gruppo): quale mostra.
- **Fixture** (ruota dei colori e griglia XY): su quale gruppo di fixture agiscono, oppure «Tutte le fixture con colore» / «Tutte le teste mobili».
- **Immagine** (immagine): il file, e «Immagine intera» o «Riempi la sezione».
- **Secondi** e **Data** (orologio).
- **Conta** (timer): «In avanti (cronometro)» o «All'indietro (conto alla rovescia)», con i suoi **Minuti**.
- **Testo** e **Dimensione** (note).
- **Universo** (livelli DMX).
- **Pagina di playback**, **Playback** e **Primo playback** (playback): vedi [Playback nello SHOW](show.md#playback-nello-show).

## Tasti

1. Seleziona una sezione di tasti e fai clic su una cella vuota (quella con il **+**).
2. Premi **«Aggiungi un tasto qui»**.
3. Scegli la sua **Azione** e ciò che serve (la scena, il gruppo, la palette…).

Il modo più rapido per riempire una tabella è **trascinare le scene** dall'elenco che si trova sotto le proprietà («Trascina una scena su una cella:») fino alle celle: ogni scena diventa un tasto che la avvia.

### Azioni di un tasto

| Azione | Cosa fa |
|---|---|
| **Riproduci una scena** | Avvia la scena nel suo gruppo (o da sola, se non è in nessun gruppo). Premuto più volte di seguito, batte il [ritmo](directo.md#ritmo-di-una-scena-a-passi) di una scena a passi, di una cuelist o di una scena FX con degli FX in «BPM dello show». Con **Stop** nella barra superiore, la ferma. |
| **Scena successiva di un gruppo** / **Scena precedente di un gruppo** | In un gruppo, o in «Tutti i gruppi». |
| **Rilascia un gruppo** | Un gruppo, o tutti. |
| **Blackout** | Attiva o toglie il blackout. |
| **Ferma tutto** | Come il tasto della barra superiore. |
| **Tap tempo** | Batte il tempo dello show (quello degli FX e delle scene in modalità BPM). |
| **BeatGO (divisione successiva)** | Come il tasto BeatGO della barra superiore. |
| **Una palette** | Mette la palette sulle fixture con cui è stata registrata («Le sue fixture») o su un gruppo di fixture, come una modifica dal vivo del programmatore: va sopra le scene finché non premi di nuovo. |
| **GO della cuelist** / **Indietro della cuelist** / **Rilascia una cuelist** | Sulla cuelist che scegli in **Cuelist**. |
| **Vai a una cue…** | Salta alla **Cue** che scegli di quella cuelist. |

Un tasto portato da un'altra pagina con Alt+clic mostra «Un controllo su un'altra pagina» e sotto ciò che fa («Fa: …»).

### Opzioni di un tasto

- **Etichetta**: il suo testo. Vuota («Automatica»), mostra il nome della scena, della palette o dell'azione.
- **Comportamento** (scena, blackout e palette): **«Commuta: acceso / spento»** o **«Flash: finché è premuto»**. I tasti flash hanno un fulmine nell'angolo. Su una scena, al posto di «Commuta» c'è **«Premi: secondo GO / Stop»**: la avvia o la ferma secondo **GO** e **Stop** della barra superiore.
- **Tasto della tastiera**: un tasto della tastiera che lo preme mentre sei nella pagina Show. La lettera si vede nell'angolo del tasto.
- **Colore**, **Icona** e **Immagine** di sfondo (l'immagine appare attenuata mentre il tasto è spento).

Un tasto acceso si riempie con il suo colore.

### Spostare e rimuovere

Nella modalità modifica, trascina un tasto, un fader o una manopola in un'altra cella della stessa sezione per spostarlo. Per rimuoverlo, selezionalo e premi **Canc** (o «Rimuovi il tasto» / «Rimuovi il fader» nelle proprietà). **Esc** annulla la selezione.

## Fader

Un fader dello SHOW può controllare **più cose insieme**. Nelle sue proprietà, sotto **Controlla**, ogni elemento è uno di questi:

| Elemento | Cosa fa il fader |
|---|---|
| **Avvia una scena (la sua intensità)** | Salendo da 0, la scena parte nel suo gruppo; scendendo a 0, si ferma; nel mezzo è la sua intensità. Il fader aggiunge la sua scena: non ne ferma nessun'altra (né quelle del suo gruppo, anche se esclusivo, né quelle che fermerebbe la sua modalità «All'avvio, rilascia»), e nessun altro fader ferma la sua, quindi puoi alzarne diversi insieme. Finché è alzato, nient'altro la ferma: né i pulsanti del gruppo, né il suo, né «Rilascia gruppi», né «Ferma tutto»; solo abbassarlo fino in fondo (anche chiudere o cambiare show la ferma). Sui canali che condividono, a parità di priorità, l'intensità è la più alta e il resto (colore, posizione…) lo dà l'ultima che hai alzato; abbassandola, torna l'altra. Con priorità diverse vince la più alta ([Priorità](directo.md#priorità)). |
| **Dimmer di una scena (senza avviarla)** | Attenua la scena chiunque la avvii (il suo gruppo, un playback, un tasto, una super scena). Non la avvia né la ferma. Il suo livello si salva con lo show e dura finché c'è un fader o una manopola di dimmer su quella scena: se lo rimuovi, la scena torna al 100 %. |
| **Master di un gruppo** | Il master di quel gruppo di Live. |
| **Grand master** | Il GM della barra superiore. |
| **Dimmer di fixture** | Il dimmer del mixer di tutte le fixture o di un gruppo di fixture. |
| **Velocità di movimento delle teste mobili** | La velocità di ciò che muove le teste mobili (tutte o quelle di un gruppo): i loro FX di pan/tilt, quelli del programmatore e i passi che le muovono. In basso ×0,1, al centro quella normale (×1), in alto ×10; il fader mostra il moltiplicatore e un doppio clic lo riporta a ×1. Un FX di pan/tilt e di colore cambia solo la velocità del movimento; una cuelist, quella dei suoi FX. Il canale di velocità dei motori non viene toccato. |
| **Velocità di una scena** | La velocità dal vivo di quella scena (da ×0,1 a ×10, come quella del movimento). |
| **Avvia una scena (la sua velocità)** | Come «Avvia una scena (la sua intensità)», ma il fader è la sua velocità: in fondo la scena è ferma; alzandolo, parte nel suo gruppo e la corsa è la sua velocità, ×0,1 appena sopra il fondo, ×1 a metà e ×10 in cima. Il testo dice «Ferma» finché non suona, e il fader segue la scena se la si ferma o la si avvia da un'altra parte. La velocità vale solo per quella volta: una volta ferma, la scena torna alla velocità che aveva (quella del Live non cambia). Un doppio clic la porta a ×1 (e la avvia se era ferma). Come quello di intensità, aggiunge la sua scena al suo gruppo e la trattiene: diversi insieme suonano insieme (anche più scene di uno stesso gruppo esclusivo su un fader), e solo abbassarlo fino in fondo la ferma. |
| **BPM** | Il tempo dello show, da 20 a 300. |

La differenza tra i due di scena: **«Avvia una scena (la sua intensità)»** è la scena stessa, e la avvia e la ferma; **«Dimmer di una scena (senza avviarla)»** la attenua soltanto, anche quando viene avviata da un altro punto. Si possono usare insieme: con il dimmer al 50 % e il fader che la avvia all'80 %, la scena esce al 40 %.

Premi **«Aggiungi un altro elemento»** per aggiungerne un altro e l'icona del cestino per rimuoverlo. Se trascini una scena su una cella di fader, si crea un fader che la avvia.

Con **Intervallo**, **Da** … **a** … (da 0 a 100 %), la corsa del fader copre solo quella parte di ciò che muove, in tutti i suoi elementi. Per esempio, la velocità di una scena dal 20 all'80 %, o un dimmer da 0 a 60 % perché non vada oltre. Con «Da» maggiore di «a», o con **«Inverti»**, il fader va al contrario. Un fader che avvia una scena la ferma all'estremo dove il valore è più basso (in fondo; in cima se è invertito), anche se «Da» non è 0.

Il testo del fader mostra il valore reale (la %, i BPM o, se controlla solo velocità, il moltiplicatore ×), non dove si trova il fader. Se un altro controllo porta il valore fuori dall'intervallo, il fader resta all'estremo. Un fader che controlla solo velocità (del movimento o di scene, una o più) torna a ×1 con un doppio clic.

Sotto ogni fader c'è un tasto **«Flash»**: lo porta in cima alla sua corsa finché lo premi e, quando lo rilasci, ripristina esattamente ciò che c'era. Si toglie o si mette con l'interruttore «Tasto flash: al massimo finché è premuto» nelle sue proprietà.

> **Consiglio:** un fader con «Master di un gruppo» dei tuoi wash e «Dimmer di fixture» dei PAR sul pubblico ti dà un solo fader per abbassare tutto il frontale.

## Manopole e ruote

Le manopole controllano le stesse cose dei fader (la tabella sopra), con lo stesso **Intervallo**. Ogni manopola può essere:

- Una **manopola**: percorre tutto il suo intervallo in tre quarti di giro. Il doppio clic la porta all'inizio della sua corsa (il valore di «Da», o quello di «a» se è invertita); se controlla i BPM, a 90, o all'estremo dell'intervallo più vicino se 90 resta fuori; se controlla solo velocità, a ×1.
- Una **rotella incrementale** («Rotella incrementale: gira all'infinito, ogni scatto aggiunge o toglie un passo»): gira all'infinito, e ogni scatto aggiunge o toglie il **Passo** (da 1 a 25 %) al punto in cui si trova il valore. Il passo è una % del valore, non della corsa, e la rotella non esce dall'intervallo; girandola verso l'alto con la scena ferma, la avvia su «Da». L'arco intorno mostra il livello.

Per girarle, trascina intorno al centro con il mouse o con il dito, oppure su e giù vicino al centro. Ogni scatto della rotella del mouse le gira un po' (in una rotella incrementale, uno scatto).

## Ruota dei colori e griglia XY

- **Ruota dei colori**: tocca la ruota per dare tonalità e saturazione (bianco al centro) alle fixture della sezione, o premi uno dei campioni sotto.
- **Griglia XY (pan e tilt)**: trascina il punto per muovere le teste. **«Centro»** le porta al centro.

Entrambe agiscono come una modifica dal vivo del programmatore: vanno sopra le scene finché non premi **«Rilascia»**, e allora le scene tornano a dare il colore o la posizione.

## Gruppi e mixer

Le sezioni **Un gruppo** e **Tutti i gruppi** mostrano le colonne di [Live](directo.md) con tasti grandi, i loro precedente/successivo, il loro tasto di rilascio e il loro master, senza opzioni di modifica. **Mixer dal vivo** mostra le strisce del mixer adattate alla larghezza.

Anche qui i gruppi si **comprimono**, con la freccia accanto al loro nome: in **Tutti i gruppi**, il gruppo diventa una striscia stretta e gli altri si dividono la larghezza; una sezione **Un gruppo** che occupa una metà della pagina si comprime nella sua striscia (o nella sua intestazione, se le metà sono una sopra l'altra) e l'altra metà si allarga. Le sue scene restano in riproduzione e si salva con la pagina (è indipendente dalla compressione in Live). In uno SHOW bloccato i gruppi non si comprimono.

## Playback nello SHOW

La sezione **Playback** mostra strisce uguali a quelle della pagina [Playback](playbacks.md):

- **Pagina di playback**: **«Segui la pagina attuale»** (la barra del titolo mostra ◀ il nome della pagina ▶ per cambiarla) o una pagina fissa.
- **Playback**: quanti se ne vedono (fino a 20) e **Primo playback**: da quale.
- Tre interruttori per mostrare o nascondere il **secondo tasto (Pausa / Indietro)**, il **terzo tasto (Flash)** e **la cue in riproduzione e la successiva**.

Nella modalità modifica, rilascia una scena su una striscia per metterla su quel playback.

## Utilità

- **Orologio**: ora e data del computer.
- **Timer**: **«Avvia»** / **«Pausa»** e **«Azzera»**. Nel conto alla rovescia diventa arancione nell'ultimo minuto e rosso quando arriva a zero. Continua a contare anche se cambi pagina.
- **Ritmo**: **GO**, **Stop** e **BeatGO**.
- **Note**: il testo che scrivi nelle sue proprietà (la scaletta, avvisi per l'operatore…).
- **In riproduzione ora**: ciò che è in riproduzione, con il tasto per fermarla accanto a ogni scena.
- **Livelli DMX**: una griglia con i 512 canali dell'universo scelto.
- **Immagine**: intera o a riempire la sezione.

## Pagine

- Nella modalità modifica, **«+ Pagina»** aggiunge una pagina.
- Fai clic sul nome di una pagina per vederla. Le schede delle pagine si vedono se ce n'è più di una.
- Nella modalità modifica, un doppio clic su una scheda la rinomina; il clic destro apre **«Rinomina…»**, **«Sposta a sinistra»**, **«Sposta a destra»** e **«Elimina pagina…»**. Quando la elimini, viene indicato quante sezioni e controlli se ne vanno con lei: non si può annullare.

## Portare qualsiasi controllo nello SHOW

**Alt+clic** su quasi qualsiasi controllo del programma lo copia nella pagina dello SHOW che hai aperta: una scena di Live, il master di un gruppo, un tasto del mixer, il grand master, il blackout, una palette di Programma, i tasti e il fader di un playback… Il programma avvisa «Aggiunto allo SHOW, pagina «…»».

Il nuovo controllo va nella prima cella libera di una tabella di tasti o di fader di quella pagina. Se non ce n'è nessuna, una sezione vuota si trasforma in una (con il titolo «Touch»), oppure si divide l'ultima sezione.

## Schermo intero e blocco

- **«Schermo intero»** (F11) e **«Esci da schermo intero»**.
- **«Blocca»**: nasconde la barra laterale e lascia solo lo SHOW, senza modifica né navigazione. Per uscire, **tieni premuto «Sblocca»** (una barra si riempie sotto il tasto). Se c'è una password di blocco, la chiede.

### Blocco e modalità chiosco

- In Opzioni › «SHOW e blocco», **«Apri lo SHOW a schermo intero all'avvio»** apre direttamente lo SHOW. Con **«…e bloccato»**, si avvia anche bloccato: solo lo SHOW, come un chiosco.
- Se blocchi la **«Modifica dello SHOW»** con il lucchetto della barra superiore, il tasto «Modifica» scompare: le pagine si usano, ma non si modificano.

Vedi [Opzioni](opciones.md#show-e-blocco).

## Tasti, mappature e controllo remoto

- I **tasti della tastiera** assegnati ai tasti funzionano solo nella pagina Show. Per questo, in questa pagina il tasto G non fa BeatGO.
- Nella modalità Apprendi, i tasti e i fader dello SHOW si possono assegnare a un controller; un controllo MIDI può muovere un fader dello SHOW intero, con tutti i suoi elementi (vedi [Mappature](mapeos.md)).
- Le pagine dello SHOW si vedono e si usano sul telefono o sul tablet con il [controllo remoto](remoto.md).

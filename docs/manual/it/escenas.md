# Scene

![La pagina Scene](../img/escenas.png)

**Scene** (F4) è la libreria di tutto ciò che hai registrato: scene statiche, passi, FX, effetti colore, rack, super scene e cuelist. A sinistra c'è l'elenco; a destra, l'editor della scena scelta, diverso a seconda del tipo.

Quasi tutto si crea dal programmatore: i nuovi effetti prendono le fixture selezionate in [Programma](programar.md), e i passi e le cue i suoi valori.

## L'elenco delle scene

- **«Cerca…»**: filtra per nome (o per nome della cartella).
- **«Cartelle»** / **«Gruppi»**: vedere le scene per [cartelle](escenas.md#cartelle) o per i gruppi di Live in cui si trovano («Senza gruppo» per le altre).
- **Comprimi tutto** / **Espandi tutto**: il tasto a destra.
- **Filtro per tipo**: «Tutte», «Statica», «Passi», «Cuelist», «FX», «Colour FX», «Rack» e «Super scene».
- **Ctrl+clic** e **Maiusc+clic** selezionano più scene per spostarle in una cartella («In cartella…»).

Ogni scena ha una fascia con il colore del suo tipo. Fai clic su una per aprirla nell'editor.

## Creare una scena

Premi **«Nuova»**:

| Opzione | Cosa crea |
|---|---|
| **Statica (dal programmatore)** | Una scena statica con i valori del programmatore. |
| **Passi (programmatore = primo passo)** | Una scena a passi. |
| **Cuelist (programmatore = cue 1)** | Una cuelist. |
| **FX (nel programmatore)** | Ti porta in Programma › FX: gli FX si fanno lì e si registrano con «Registra…». |
| **Colour FX (fixture selezionate)** | Un effetto colore sulle fixture selezionate, nel loro ordine. |
| **Mapping sulla pianta 2D (fixture selezionate)** | Un effetto colore posizionato dove si trovano le fixture nella pianta 2D. |
| **Rack di effetti** | Un rack vuoto. |
| **Super scena (timeline con audio)** | Una super scena vuota. |
| **Importa da un file…** | Scene ed effetti esportati da un altro show (vedi [Importare ed esportare](escenas.md#importare-ed-esportare)). |
| **Cue list di QLC+…** | Cuelist con i chaser, le sequenze e le liste di cue di uno show di QLC+ (vedi [Cue list di QLC+](escenas.md#cue-list-di-qlc)). |

## L'intestazione dell'editor

- **Il nome**: fai clic per cambiarlo.
- **La cartella** (icona della cartella): spostare la scena in una cartella.
- **«Prova»** / **«Ferma»**: la riproduce ora, fuori dai gruppi. Le cuelist hanno il loro GO.
- **«Aggiungi al gruppo…»**: la aggiunge a un gruppo di Live.
- **Converti in passi** (rack, FX, Colour FX, Move FX, Curve FX e chaser): registra un ciclo in una nuova scena a passi, solo con i canali a cui la scena dà un valore: gli altri, anche se hanno un valore predefinito (il pan o lo zoom al centro di molte teste mobili), non entrano nei passi. Se un FX ha dei ritardi, registra il ciclo quando tutte le sue fixture sono già partite.
- **Converti in cuelist** (passi e chaser): vedi [Cuelist](escenas.md#cuelist).
- **«QLC+»** (cuelist): salva una copia dello show per QLC+; vedi [Cuelist e QLC+](escenas.md#cuelist-e-qlc).
- **Esporta**, **«Duplica»** ed **«Elimina»** (viene rimossa anche dai suoi gruppi e rack).

## Come si riproduce una scena

Sotto l'intestazione c'è il pannello di riproduzione. Mostra ciò che ha senso per ogni tipo:

- **«Entrata»** e **«Uscita»**.
- **«Tempo di ciclo»** (o **«Tempo per passo»** in un Colour FX) e **«Velocità»** (da un ciclo al minuto a dieci al secondo; doppio clic: un ciclo al secondo), con **×½** e **×2**.
- **«Entrata scaglionata»** (statiche e passi): ogni fixture inizia la sua dissolvenza questo tempo dopo la precedente, in ordine di selezione.
- **«Guidata da»**: **«Durata»** (i tempi normali), **«BPM»** (i tempi diventano beat e seguono il tempo dello show, anche le dissolvenze di ogni passo; il tempo di dissolvenza incrociata di un gruppo di Live e le dissolvenze dei blocchi di una super scena restano in secondi), **«BeatGO»** o **«Impulso»** (avanza con il tasto BeatGO o con i picchi dell'ingresso audio). Con BeatGO o Impulso, **«Divisioni»** (/2, ×2).
- **«Ritmo battuto»** (sequenze, chaser e scene FX con degli FX in «BPM dello show»): il ritmo battuto premendo il suo tasto più volte di seguito ([Live](directo.md#ritmo-di-una-scena-a-passi)), in BPM. Finché ce l'ha, i suoi passi vanno a quel ritmo e non secondo i loro tempi; in una scena FX, i suoi FX in «BPM dello show» fanno un ciclo a ogni battito invece di seguire i BPM dello show. **«Rimuovi»** torna ai suoi tempi (o ai BPM dello show). Con «BPM», un beat è un passo di quel ritmo: una dissolvenza di 0,5 b dura mezzo passo.
- **«Inizio»**: **«In riproduzione»** o **«In pausa, premi di nuovo per riprodurre»**.
- **«Si riproduce»**: **«Sempre»** o **«N volte»**, **«poi»** **«Rilasciala»** o **«Mantieni l'ultimo fotogramma»**, **«e riproduci»** «Niente», «La scena successiva del gruppo», «La scena precedente del gruppo» o qualsiasi altra scena.

## Tipi di scena

### Statica

Salva valori fissi. Mostra quanti canali e fixture ha e le palette che usa («Usa queste palette: quando cambiano, cambia anche la scena»).

Per modificarla:

1. Premi **«Modifica nel programmatore»**: viene caricata in Programma.
2. Cambia ciò che vuoi.
3. Torna in Scene e premi **«Salva il programmatore in questa scena»**.

Se salvi degli FX in una scena statica, diventa una scena FX.

### Passi

Una successione di look, ognuno con la sua **dissolvenza in entrata**, la sua **tenuta** e la sua **dissolvenza in uscita**.

- **«Aggiungi passo»**: il programmatore diventa un nuovo passo dopo quello selezionato.
- **«Salva nel passo»**: salva il programmatore nel passo selezionato.
- **«Carica passo»** (o doppio clic su un passo): lo porta nel programmatore per vederlo e modificarlo.
- **Duplica**, **▲ ▼** per riordinare ed **elimina** il passo.
- Il tasto **∞** della tenuta fa sì che il passo aspetti un GO («Fino a GO»).

**Ctrl+clic** e **Maiusc+clic** selezionano più passi: quando cambi un tempo, cambia in tutti. I passi usano le palette **per riferimento**, come le scene e le cue: se aggiorni una palette, cambiano i passi registrati con essa (quello in riproduzione, subito). L'icona della palette di un passo dice quali usa.

### FX

Una scena FX ha dei **valori base** e uno o più **livelli di FX**. Si crea registrando dal programmatore con degli FX (vedi [FX](programar.md#fx)).

- A sinistra, l'elenco degli FX: parametro, fixture e velocità (in «BPM dello show», il ritmo battuto della scena se ce l'ha). Il cestino rimuove un livello.
- A destra, i controlli del livello scelto (gli stessi di Programma) e la sua anteprima animata. Le modifiche si vedono dal vivo mentre la scena è in riproduzione.
- **«Collega»**: i livelli collegati condividono velocità e timing.
- Se alcune fixture fanno l'FX a modo loro, viene segnalato: se qui cambi la forma, la dimensione, la direzione o la fase, tutte faranno lo stesso.
- **«Per fixture»**: lo sfasamento e il ritardo di ogni fixture, anche dal vivo. Qui «Distribuisci…» e «Senza sfasamento né ritardo» valgono per tutte (vedi [Sfasamento e ritardo per fixture](programar.md#sfasamento-e-ritardo-per-fixture)).
- Per aggiungere livelli o cambiare i valori base: **«Modifica nel programmatore»** e poi **«Salva il programmatore in questa scena»**.

### Colour FX e mapping

Un **Colour FX** è un effetto pixel (più di 60: sfumature, fuoco, meteore, strobo, onde…) che percorre le fixture nell'ordine di selezione.

- **«Effetto»**: quale.
- **«Agisce su»**: «Colore (RGB)», «Bianco», «Ambra», «UV», «Dimmer (valore)» o «Shutter».
- **«Colori»**: fino a 5, a seconda dell'effetto. Fai clic su un colore per cambiarlo (campioni o tonalità), o «Nessun colore».
- **«Testo»**: negli effetti di testo, il testo che percorre le fixture.
- **«Immagine»** e **«Animazione»**: negli effetti di immagine, il file e «Fissa», «Scorri di lato», «Scorri su e giù» o «Animata (GIF)».
- **«Disposizione»**: **«In fila»** (in ordine di selezione) o **«Sulla pianta 2D»** (dove si trovano le fixture).
- **«Parametri»**: quelli propri di ogni effetto.
- **«Usa la selezione del programmatore»**: sostituisce le sue fixture con quelle selezionate.

In alto si vede il colore reale di ogni testa durante la riproduzione (premi «Prova» per vederlo).

Un **mapping sulla pianta 2D** è un Colour FX che nasce già con la disposizione della pianta: l'effetto, il testo o l'immagine si posizionano sul palco così come sono le fixture.

### Move FX e Curve FX

Sono effetti del modulo precedente: i **Move FX** muovono pan/tilt con una forma e i **Curve FX** danno un'onda (sinusoide, triangolo, rampe, quadra, casuale, strobo…) ai canali. Li troverai in show vecchi o in show di QLC+; non si creano più da «Nuova».

Si riproducono come prima e hanno il loro editor (forma, larghezza, altezza, rotazione, posizione, fase, distribuzione tra le fixture, simmetrico, relativo…). Un avviso offre **«Converti in FX»**: passano all'editor degli FX, anche nel programmatore, e i gruppi, i tasti dello SHOW e le mappature li conservano. La fase di un Move FX diventa la «Fase» dell'FX, e lo sfasamento di partenza di ogni fixture il suo «Sfasamento» ([Sfasamento e ritardo per fixture](programar.md#sfasamento-e-ritardo-per-fixture)).

### Rack di effetti

Più scene ed effetti che vengono riprodotti insieme come uno solo, ognuno sulle sue fixture.

- In **«Aggiungi»** (a destra), fai clic su una scena per metterla nel rack.
- In **«Nel rack (N)»**, la **×** la rimuove.

Un rack non può contenere una scena che riproduca il rack stesso.

### Chaser

I chaser degli show di QLC+ vengono riprodotti normalmente, ma non hanno un editor: convertili in passi (registra un ciclo) o in cuelist con i tasti dell'intestazione.

## Super scene

Una **super scena** è una timeline con tracce di scene e di audio. Si avvia come qualsiasi scena (in un gruppo, nello SHOW, con una mappatura). Il suo editor occupa tutta la pagina; il tasto dell'elenco, in alto a sinistra, torna all'elenco delle scene.

### La barra degli strumenti

- **«Riproduci»** / **«Ferma»** (**Spazio**): dal cursore. **⏮** torna all'inizio. Accanto si vede il tempo (e la battuta).
- Le frecce **Annulla** e **Ripeti** (**Ctrl+Z** / **Ctrl+Y**). Vedi [Annullare](escenas.md#annullare).
- **«Incolla»** (**Ctrl+V**): i blocchi copiati, sul cursore e a partire dalla traccia selezionata. Vedi [Copiare e incollare](escenas.md#copiare-e-incollare).
- **«Tempo»** / **«Battute»**: la griglia. In battute, i **BPM**, il metro (**4/4**, **3/4**, **2/4**) e **«Battuta 1 qui»** perché la prima battuta inizi al cursore e coincida con la musica.
- **«Aggancia»**: i blocchi e i punti si agganciano alla griglia.
- **«Fine qui»**: la super scena finisce al cursore (la **×** rimuove il marcatore: finisce con il suo ultimo blocco). **«Ciclo»**: ricomincia da capo.
- **«+ Traccia»** e **«+ Audio»** (un file audio sulla traccia selezionata, al cursore).
- **«Valori statici»**: **«Prendi i valori del programmatore»** (vengono riprodotti mentre la super scena è in esecuzione) o **«Cancella i valori statici»**.
- **Zoom** − e +.

### Mettere le scene sulla timeline

1. Premi **«+ Traccia»** se non ce n'è nessuna.
2. Trascina una scena dall'elenco a sinistra su una traccia. Oppure seleziona una traccia, posiziona il cursore e fai **doppio clic** sulla scena: viene messa al cursore.

I blocchi si possono sovrapporre.

Una cuelist **«Timecode»** con l'orologio proprio in un blocco segue la timeline: lo 00:00 del suo orologio è l'inizio del blocco. Mettila su un'altra traccia, accanto al blocco audio della canzone e dallo stesso punto: ogni cue suona al suo momento della canzone. Se riproduci da un altro punto, la cuelist va dritta alla cue di quel tempo. Allungata, il suo orologio va più veloce o più lento perché la cuelist duri quanto il blocco.

### Lavorare con i blocchi

- **Trascina** un blocco per spostarlo, anche su un'altra traccia. Trascina i suoi **bordi** per cambiarne la durata.
- Con un solo blocco selezionato, le **maniglie** degli angoli superiori sono le sue dissolvenze in entrata e in uscita.
- Quando allunghi un blocco, la scena **si ripete in ciclo** (si vedono dei segni a ogni giro) o si **allunga** perché un ciclo duri tutto il blocco. Si cambia nel menu del blocco o nella barra in basso («In ciclo» / «Allungata»).
- **Clic destro**: «Ripeti la scena nel blocco» / «Allunga la scena per adattarla al blocco», «Copia», «Taglia», «Duplica», «Blocca» / «Sblocca», «Apri la scena» ed «Elimina». Su un blocco della selezione, il menu agisce su tutta la selezione; «Apri la scena» apre quella del blocco cliccato.
- **Doppio clic** su un blocco apre la sua scena. **Canc** (o **Backspace**) elimina i blocchi selezionati; quelli delle tracce bloccate restano. **Ctrl+D** li duplica subito dopo: ogni copia sulla sua traccia.
- La barra in basso mostra il blocco selezionato: **Inizio**, **Durata**, **Entrata**, **Uscita**. Con più blocchi, dice quanti sono. Con uno o più blocchi, ha **Copia**, **Taglia**, **Duplica** ed **Elimina**.

### Selezionare più blocchi

- **Clic** su un blocco: solo quello. La sua traccia diventa la traccia selezionata.
- **Ctrl+clic** aggiunge un blocco alla selezione o lo toglie.
- **Maiusc+clic** seleziona dall'ultimo blocco scelto fino a questo: i blocchi delle tracce tra i due, dall'inizio più presto alla fine più tarda.
- **Trascinare in una zona vuota** delle tracce disegna un rettangolo che seleziona i blocchi che tocca. Con **Ctrl** o **Maiusc**, si aggiungono alla selezione.
- **Ctrl+A** seleziona tutti i blocchi delle tracce visibili. Quelli di una traccia che nascondi escono dalla selezione.
- Un clic in una zona vuota toglie la selezione (e sposta il cursore). Con Ctrl o Maiusc, la selezione resta.

Quando **trascini** un blocco selezionato, si sposta tutta la selezione:

- Tutti si spostano dello stesso tempo: il blocco trascinato si aggancia alla griglia e gli altri mantengono le distanze. Il primo si ferma a 0.
- Salgono o scendono dello stesso numero di tracce. Le tracce dove vanno si illuminano.
- I blocchi bloccati e quelli delle tracce bloccate restano dove sono.
- Se uno di essi cadesse fuori dalle tracce o su una traccia bloccata, si spostano solo nel tempo.
- I bordi e le dissolvenze cambiano solo il blocco sotto il mouse.

Premere un blocco della selezione e rilasciarlo senza spostarlo lascia selezionato solo quello. Ogni azione sulla selezione è un solo passo di [Annullare](escenas.md#annullare) («Sposta 3 blocchi»).

### Copiare e incollare

- **Ctrl+C** copia i blocchi selezionati. **Ctrl+X** li taglia: li copia e li toglie. Quelli delle tracce bloccate vengono copiati, ma restano.
- **Ctrl+V** o **«Incolla»** li incolla sul cursore, a partire dalla traccia selezionata (senza traccia selezionata, dalla prima).
- **Clic destro** su una zona vuota di una traccia: **«Incolla qui»** incolla in quel punto e su quella traccia. **«Seleziona tutto»** fa lo stesso di **Ctrl+A**.
- Il primo blocco cade sul cursore e gli altri mantengono le loro distanze nel tempo. Quello della traccia più in alto va sulla traccia selezionata e gli altri mantengono le loro distanze in tracce.
- Se mancano tracce, vengono aggiunte alla fine. Una traccia nascosta che riceve blocchi viene mostrata.
- I blocchi incollati mantengono durata, dissolvenze, ciclo o allungamento e colore. Arrivano sbloccati, come con Duplica.
- Ciò che incolli resta selezionato e il cursore passa alla sua fine: premi di nuovo **Ctrl+V** e la copia successiva va subito dopo.
- Puoi incollare in un'altra super scena dello stesso show: aprila e incolla. Le scene non vengono copiate: il blocco incollato riproduce la stessa scena.
- Un blocco audio incollato nella stessa super scena riproduce lo stesso audio. In un'altra super scena ha il suo audio, letto dallo stesso file: eliminarlo lì non si porta mai via l'originale. Se hai tagliato il blocco, il suo audio viene riletto dal file.
- Non si incolla niente su una traccia bloccata, né una scena che riproduce questa super scena (per esempio, un rack che la contiene). Un avviso lo dice.
- Ciò che copi resta finché lo show è aperto. Aprire un altro show o iniziarne uno nuovo lo svuota. Una scena che elimini dalla lista ne esce.
- Incollare e tagliare sono un solo passo di [Annullare](escenas.md#annullare). Ciò che incolli con la super scena in riproduzione si sente dal prossimo «Riproduci».

### Audio

I blocchi audio (wav, flac, ogg, aiff e mp3 se il sistema lo supporta) mostrano la loro **forma d'onda**, che viene calcolata in background. Per aggiungerne uno serve una traccia non bloccata.

### Tracce e automazione

Nell'intestazione di ogni traccia:

- **M**: muto (la traccia non suona).
- **Lucchetto**: blocca (i suoi blocchi e punti non si muovono).
- **D**: la linea di automazione del **dimmer** (in una traccia audio, il volume).
- **φ**: la linea di automazione della **fase** dei suoi effetti.
- **⋯**: «Rinomina» (anche con doppio clic sul nome), «Sposta su», «Sposta giù», «Cancella l'automazione di questa corsia» ed «Elimina la traccia».
- La freccia a destra nasconde o mostra la traccia.

Su una linea di automazione: il **clic** aggiunge un punto, **trascinare** lo sposta, il **doppio clic** lo cancella e il **clic destro** alterna tra retta e curva morbida. Senza punti, il dimmer è al massimo e la fase a 0.

### Muoversi sulla timeline

- Clic o trascinamento sul **righello** sposta il cursore; **doppio clic** sul righello riproduce da lì. Anche un clic su una traccia vuota sposta il cursore.
- **Ctrl+rotella**: zoom. **Maiusc+rotella**: spostarsi nel tempo. La rotella da sola sale e scende tra le tracce.

### Annullare

- **Ctrl+Z** annulla l'ultima modifica. **Ctrl+Y** (o **Ctrl+Maiusc+Z**) la ripete. Le frecce della barra degli strumenti fanno lo stesso; passandoci sopra con il mouse, dicono quale modifica annullano o ripetono.
- Ogni super scena ha la sua cronologia, fino a 100 passi. Dura finché lo show è aperto: salvare non la cancella, aprire un altro show o iniziarne uno nuovo sì.
- Contano le modifiche a tracce, blocchi, automazione, griglia, BPM, fine, ciclo e valori statici. Più modifiche dello stesso campo della barra in basso (o dei BPM) in meno di un secondo sono un solo passo.
- Non contano nascondere o mostrare le tracce, lo zoom, il cursore, «Aggancia» e la selezione.
- Mentre scrivi in un campo, Ctrl+Z annulla quello che hai scritto nel campo.
- Funziona con la super scena in riproduzione. Un blocco tolto smette di suonare; un blocco che torna suona dalla prossima «Riproduci».
- Un blocco audio torna con il suo audio, riletto dal suo file.
- I blocchi di una scena eliminata dalla lista non tornano.

## Cuelist

Una **cuelist** è un elenco di cue che avanzano con GO. Funziona con il **tracking**: ogni cue salva **solo ciò che cambia** e il resto prosegue dalle cue precedenti. Un GO cambia solo i canali che cambiano: senza cali e senza riprendere ciò che non tocca.

Con altre cuelist e scene della stessa priorità, ogni canale che non è di intensità (posizione, gobo, ruota colori…) viene dall'ultimo che l'ha cambiato: il GO che lo cambia lo prende, e ciò che la cuelist mantiene solo dalle cue precedenti non lo riprende. Quando viene rilasciato, fa una dissolvenza verso chi lo aveva prima. Vedi [Priorità](directo.md#priorità).

Le cuelist si registrano dal programmatore, si gestiscono in [Playback](playbacks.md) e si modificano qui.

### Opzioni della cuelist

In cima all'editor:

- **«Tipo»**: come si riproduce la cuelist. Una riga accanto ai tasti dice cosa fa il tipo scelto. In Playbacks e in questo editor, una cuelist di un altro tipo mostra la sua etichetta: **SUB**, **CHASE**, **TC** o **OVR**.
  - **«Normale»** (predefinito): ogni GO avvia la cue successiva.
  - **«Chase»**: le cue si susseguono da sole, in ciclo, a una velocità (vedi sotto).
  - **«Timecode»**: ogni cue parte al suo tempo su un orologio (vedi sotto).
  - **«Submaster»**: comanda il fader del suo playback (vedi sotto).
  - **«Override»**: il suo fader porta in dissolvenza tutto ciò che controlla tra ciò che c'è sotto e la sua cue (vedi sotto).
- **«Tracking»**: acceso, ogni cue salva solo ciò che cambia. Spento, ogni cue va per conto suo.
- **«Dopo l'ultima cue»**: **«Ciclo»** (torna alla prima), **«Rilascia»** o **«Resta»**.
- **«Tempo di rilascio»**: la dissolvenza quando si rilascia la cuelist.
- **«Al rilascio, torna alla cue 1»**: dopo averla rilasciata, il GO riparte dalla prima cue.
- **«Rilasciata dal GO di un'altra cuelist»**: una canzone che finisce quando inizia la successiva.
- **«Dissolvenza delle nuove cue»**: la dissolvenza della prima cue registrata; le successive prendono i tempi della precedente.
- **«Indietro»**: i tempi di BACK: **«Tempi della cue»** o **«Tempo fisso»**.
- **«Priorità»**.
- **«GO a tempo»**: «No», «BeatGO» o «Impulso» danno un GO a ogni divisione.
- **«Ritmo battuto»**: il ritmo battuto premendo il suo tasto più volte di seguito, in BPM: un GO a ogni battito. Finché ce l'ha, i suoi Segui e Attendi non contano. **«Rimuovi»** torna ai suoi tempi.
- **«Movimento al buio»**: quando una cue è finita, le fixture spente che la cue successiva accende prendono già la sua posizione, il colore e il fascio. Al GO salgono solo d'intensità. **«Dopo»** è l'attesa dopo la fine della cue (0 s di default). **«Tempo»** è quanto durano quei movimenti (2 s di default).
  - Una fixture che un altro playback o la cue stessa illumina non si muove.
  - In una fixture senza dimmer, il suo RGB è la sua luce: si preparano solo i suoi altri attributi.
  - I canali di un FX non si preparano: l'FX parte con il suo GO.
  - Se scegli un'altra cue per il prossimo GO («Avanti» in [Playbacks](playbacks.md)), si prepara quella.
  - Non vale in una cuelist «Chase» né «Override».

In una finestra stretta, parte delle opzioni si comprime: **«Più opzioni»** le mostra.

Una cuelist **«Submaster»** è comandata dal fader del suo [playback](playbacks.md#il-fader):

- Quando alzi il fader da zero, parte sulla sua cue successiva, al livello del fader e mai al massimo.
- Quando lo abbassi a zero, viene rilasciata con il suo tempo di rilascio. Si può cambiare nelle [proprietà](playbacks.md#proprietà-di-un-playback) del playback.
- Mentre suona, GO e Indietro cambiano cue al livello del fader.
- Rilasciata, GO e Indietro scelgono solo la cue da cui partirà (anche Pausa/Indietro e «Vai»). Il playback la mostra come la successiva («▸ 3»). Dopo l'ultima cue, GO torna alla prima; con «Resta», resta sull'ultima.
- Se la rilasci con il fader alzato, abbassa il fader a zero e rialzalo per avviarla.
- Il flash la porta al massimo finché lo premi.
- In un gruppo di Live e su un tasto dello SHOW suona come una cuelist normale. Anche quando non è su nessun playback o quando il suo fader non la avvia («Il fader lo avvia» spento).

Una cuelist **«Chase»** percorre le sue cue da sola, in ciclo:

- **«Velocità»**: un passo a ogni battito, in BPM (da 10 a 600; 120 come predefinito). Con **«BPM dello show»**, segue il tempo dello show. **×¼**, **×½**, **×1**, **×2** e **×4** la moltiplicano.
- **«Dissolvenza»**: la parte del passo che dura il cambio, in %. Con 0 (il predefinito) è un taglio. Con **«Tempi della cue»**, ogni cue usa il suo ritardo e la sua dissolvenza.
- **«Direzione»**: **«Avanti»**, **«Al contrario»**, **«Andata e ritorno»** (torna indietro alla prima e all'ultima cue) o **«Casuale»** (non ripete mai un passo due volte di seguito).
- GO passa subito al passo successivo, e i successivi contano da lì. Indietro va di un passo nell'altra direzione. Pausa la ferma dove si trova.
- Un **«Ritmo battuto»** vince sulla sua velocità; **«Rimuovi»** torna a essa. Con BeatGO o Impulso, avanza solo a ogni divisione.
- La [velocità delle cue](playbacks.md#velocità-delle-cue) e la velocità dal vivo la accelerano o la rallentano.
- Il tracking vale sempre: un passo può cambiare solo il colore e tenere il resto.
- Ricomincia sempre: non ha la colonna **AVVIO** né **«Dopo l'ultima cue»**, e i collegamenti delle cue non contano. Con una dissolvenza in %, i tempi delle cue appaiono attenuati: non si usano.
- In una super scena, un blocco dura i suoi passi per la durata di un passo. In «Andata e ritorno», l'andata e il ritorno.

In una cuelist **«Override»**, il fader del suo [playback](playbacks.md#il-fader) la mescola con ciò che c'è sotto:

- Il fader porta in dissolvenza tutto ciò che controlla: intensità, colori, posizioni e gobo. A metà, le teste mobili sono a metà strada. A zero, tutto è come se non ci fosse.
- Al massimo, la sua cue sostituisce ciò che c'è sotto, anche nelle intensità: un blu sopra un rosso dà blu, non magenta.
- Le cue, i GO e le dissolvenze funzionano come in una cuelist normale. La dissolvenza di una cue e il fader si moltiplicano.
- Quando la rilasci, tutto torna con una dissolvenza a ciò che c'è sotto, nel suo tempo di rilascio. Con il fader a zero resta caricata, come ogni cuelist.
- Quando scegli «Override», una cuelist con la priorità predefinita (5) passa a 10. Il programmatore resta sopra di lei.
- Una cuelist o una scena con la stessa priorità avviata dopo di lei si mescola sopra come sempre: nelle intensità vince il valore più alto.
- Un canale con **Dissolvenze** spento nei [limiti della fixture](patch.md#limiti) salta al suo valore appena il fader lascia lo zero.
- I suoi FX suonano come in una cuelist normale: il fader non fa loro questa dissolvenza.
- Se la cuelist sta già suonando, il passaggio a «Override» (o da «Override») vale dal suo prossimo avvio.
- Usi tipici: un blinder o la luce di sala, una posizione fissa per un assolo, una luce di emergenza su un fader.

Una cuelist **«Timecode»** avvia ogni cue al suo tempo su un orologio:

- Ogni cue ha il suo **TEMPO** (ore:minuti:secondi.centesimi). La colonna **TEMPO** sostituisce **AVVIO**. Scrivi il tempo («00:01:23.40», «1:23.4» o «83.4») o usa la rotella. Se lo cancelli, la cue non ha tempo e l'orologio non la avvia.
- **«Orologio»**: **«Orologio proprio»** o **«Timecode MIDI (MTC)»** (vedi sotto). Con l'orologio proprio, GO avvia la cuelist e fa partire l'orologio da 00:00, o dal tempo della cue su cui si trova (quella scelta con «Vai alla cue» o, con «Al rilascio, torna alla cue 1» spento, la successiva dopo averla rilasciata).
- Suona l'ultima cue della lista il cui tempo è arrivato. Prima del tempo della prima cue, la cuelist non dà nulla.
- Quando passa alla cue successiva, la cue fa la sua dissolvenza. Un salto (indietro o di più cue) va dritto all'aspetto di quel tempo, senza dissolvenza.
- GO passa alla cue successiva con la sua dissolvenza, e l'orologio salta al suo tempo. Indietro va alla cue precedente e al suo tempo, senza dissolvenza (con un tempo fisso in **«Indietro»**, in quel tempo). «Vai alla cue» mette l'orologio al suo tempo. Con una cue senza tempo, l'orologio continua come andava.
- Pausa ferma l'orologio. La [velocità delle cue](playbacks.md#velocità-delle-cue) e la velocità dal vivo lo accelerano o lo rallentano.
- I suoi Segui, Attendi e collegamenti non contano. «Dopo l'ultima cue» vale per il GO sull'ultima cue.
- L'intestazione e il suo playback mostrano l'orologio: **«TC 00:01:23.4»**.
- **«Metti all'orologio»** (l'orologio della barra degli strumenti) mette la cue selezionata al tempo dell'orologio mentre la cuelist suona.
- Una cue registrata mentre l'orologio corre prende il suo tempo. Con la cuelist rilasciata o in pausa, prende il tempo della cue precedente più 5 s.
- Un triangolo avvisa di una cue il cui tempo viene prima di quello della cue precedente. Delle due, suona solo l'ultima.
- Quando scegli «Timecode», le cue senza tempo ne ricevono uno: 00:00 la prima e 5 s più della precedente le altre.
- In una [super scena](escenas.md#super-scene), l'orologio è la timeline.

Con **«Timecode MIDI (MTC)»**, la cuelist segue il timecode che una DAW, un lettore video o un generatore di timecode inviano a un ingresso MIDI:

- Attiva l'ingresso del dispositivo MIDI in [Mappature › Dispositivi](mapeos.md#dispositivi). Non serve nessuna mappatura.
- **«Offset»**: il timecode in cui si trova lo 00:00 della cuelist, in ore:minuti:secondi:fotogrammi (per esempio «01:00:00:00»). Prima di quel tempo, la cuelist non dà nulla.
- Accanto si vede il timecode che arriva, **«MTC 01:00:12:05 · 25 fps»**, o **«Nessun timecode MIDI»**.
- Frequenze dei fotogrammi: 24, 25, 29,97 drop-frame e 30.
- Quando la avvii, la cuelist aspetta il timecode e va dritta alla cue di quel tempo. Se è la prima cue, la cue fa la sua dissolvenza.
- Se il timecode si ferma o si interrompe, la cuelist resta sulla sua cue. Se torna da un altro tempo (un riavvolgimento o un salto), la cuelist va dritta alla cue di quel tempo, senza dissolvenza.
- GO, Indietro e «Vai alla cue» non fanno nulla: comanda il timecode. GO sulla cuelist rilasciata la avvia.
- La velocità delle cue non muove l'orologio; cambia solo le dissolvenze.
- Anche in una super scena, l'orologio è il timecode.

### La tabella delle cue

| Colonna | Cos'è |
|---|---|
| **CUE** | Il numero. Accetta decimali: la 2.5 va tra la 2 e la 3. Doppio clic per cambiarlo. Una matita indica che la cue è caricata nel programmatore. |
| **NOME** | Il suo nome. |
| **AVVIO** | **GO** (aspetta un GO), **Segui** (quando finisce la cue precedente, più un tempo) o **Attendi** (un tempo dopo il GO della cue precedente). |
| **TEMPO** | Solo in una cuelist «Timecode», al posto di AVVIO: il tempo della cue sull'orologio. Un triangolo avvisa se viene prima di quello della cue precedente. |
| **RITARDO** | Quanto aspetta la cue prima di iniziare la sua dissolvenza. |
| **FADE** | Il tempo di dissolvenza. |
| **DISCESA** | La dissolvenza dei dimmer che scendono. «=» è uguale alla dissolvenza. |
| **MODIFICHE** | Quanti canali cambia, e i segni **FX**, **[R]** (valori rilasciati), l'icona della palette se usa palette, un orologio se ha tempi propri o una curva, un anello di catena se ha un collegamento e un fulmine se ha azioni (passaci sopra con il mouse per vederli). |
| **S** | **Salta** (solo questa cue): ciò che cambia non passa alle successive. |
| **B** | **Blocco**: Registra e Aggiorna salvano anche i valori uguali a quelli che eredita. |
| **MIB** | Solo con «Movimento al buio»: la cue viene preparata mentre le sue fixture sono spente. Deselezionata, i suoi movimenti si vedono al suo GO. |
| **NOTA** | Una nota per te. |

Per cambiare un tempo, fai clic su di esso e scrivi i secondi, o usa la **rotella** del mouse sul tempo premuto: ±0,1 s (con Maiusc, 1 s). Con **Ctrl+clic** o **Maiusc+clic** selezioni più cue e cambi i loro tempi, il loro avvio o i loro segni insieme.

La cue in riproduzione ha una barra verde con l'avanzamento della dissolvenza; la successiva, un bordo verde. In alto si vede lo stato («Q 3 · Strofa ▸ 4», con «↺ 1/3» se la successiva è il salto di un collegamento, o «Rilasciata · GO avvia la 1») e i tasti **Indietro**, **GO** e **rilascia**, gli stessi del suo playback.

Mentre una cue è in dissolvenza, l'intestazione mostra anche **«Tempo dal vivo»**: il tempo di quella cue, che puoi cambiare mentre scorre (scrivendone un altro, con **«×½»**, **«×2»** o **«Adesso»**), e il tasto per salvarlo nella cue. Funziona come in [Playback](playbacks.md#tempo-dal-vivo).

### Registrare e modificare le cue

- **«Registra la cue N»**: il programmatore diventa una nuova cue dopo quella selezionata, solo con ciò che cambia rispetto a ciò che eredita.
- **«Aggiorna ▾»**: unisce il programmatore alla cue selezionata: **«Aggiorna la cue N (vale anche per le successive)»** o **«Solo questa cue (le successive mantengono il loro aspetto)»**.
- **«Carica ▾»** (o doppio clic su una cue): la porta nel programmatore: **«Ciò che mostra il palco nella cue N»** o **«Solo ciò che cambia la cue N»**. Poi aggiornala dalla barra «Modifica della cue…» di Programma.
- **«Togli ▾»**: con i canali del programmatore, **«Togli dalla cue N i canali del programmatore (tornano a ereditare)»** o **«[Rilascia] i canali del programmatore nella cue N»** (da quella cue tornano a ciò che c'è sotto: altre scene o il loro valore di riposo).
- **Copia** alla fine: **«Copia ciò che cambia la cue N»** o **«Copia tutto l'aspetto della cue N (cue di blocco)»**.
- **«Numero…»**: un altro numero per la cue selezionata. Spostare una cue cambia ciò che ereditano quelle che vengono dopo.
- **Elimina**: con **«Mantieni l'aspetto delle cue successive»** spuntata, ciò che cambia la cue passa alla successiva e quelle dopo appaiono uguali.

In un elenco stretto (o in una lingua con parole lunghe), «Aggiorna», «Carica» e «Togli» restano con la loro icona e la loro freccia **▾**; passando il mouse se ne vede il nome.

Il clic destro su una cue ha **«Vai alla cue N»**, i due modi di caricare, **«Aggiorna con il programmatore»**, **«Tempi…»**, **«Collegamento e azioni…»**, **«Numero…»** ed **«Elimina…»**.

Le cue possono usare **palette per riferimento** e avere **livelli di FX**, che si ereditano da una cue all'altra.

### Il foglio di tracking

**«Dettagli della cue»** ha tre schede: **«Tracking»**, **«Tempi»** e **«Collegamento e azioni»**. Se la finestra è larga, il pannello si vede da solo accanto alle cue.

**«Tracking»** mostra ciò che il palco ha nella cue selezionata, fixture per fixture («In scena: cue N»).

- Le etichette colorate dicono cos'è ogni valore: **«Modifiche»** (lo cambia questa cue), **«Ereditati»** (viene da una cue precedente, che viene indicata: «Q2»), **«Blocco»** e **«Rilasciati»** (barrati, con [R]).
- In alto si vedono le palette che usa la cue.
- Fai clic sui canali per sceglierli e premi **«Rilascia»** (li mette in [RELEASE] da questa cue) o **«Togli»** (tornano a ereditare dalle cue precedenti).

### Tempi della cue

In una cue, ogni attributo e ogni fixture possono avere un ritardo e una dissolvenza propri. Seleziona una cue (o più) e apri **«Dettagli della cue»** › **«Tempi»**, o clic destro › **«Tempi…»**. Con più cue selezionate, le modifiche valgono per tutte.

- **«Tempi per attributo»**: **Intensità**, **Colore**, **Posizione** e **Fascio**, ognuno con RITARDO e FADE. Vuoto o «=» è il tempo della cue. Lo strobo va con Intensità; gobo, prisma, zoom, fuoco e iris vanno con Fascio.
- **«Tempi per fixture»**: **«+ Fixture selezionate»** aggiunge le fixture selezionate in Programma, con i tempi della cue. Poi cambia i loro tempi. **×** ne toglie una e **«Rimuovi»** le toglie tutte.
- Il tempo di una fixture vince su quello del suo attributo, e quello di un attributo vince su quello della cue.
- I dimmer che scendono usano la dissolvenza della loro fixture, se ce l'ha. Altrimenti la DISCESA della cue; con «=», la dissolvenza di Intensità.
- Una cue dura quanto il suo tempo più lungo: Segui, la barra verde e la durata in una super scena ne tengono conto. Conta anche il tempo di un attributo che la cue non cambia.
- Un solo tempo per tutto, come prima: la dissolvenza in entrata di un gruppo o di un blocco di super scena sulla prima cue, e un tempo fisso di **«Indietro»**. Con **«Tempi della cue»**, BACK usa tutti i tempi della cue a cui torna.
- I livelli di FX entrano con la dissolvenza della cue.
- Una cue nuova prende i tempi di attributo della cue precedente, ma non quelli delle sue fixture. Copiare una cue copia tutti i suoi tempi.

Esempio: la cue 2 cambia colore e posizione con dissolvenza 0, e Posizione ha ritardo 1 s e dissolvenza 3 s. Al GO il colore cambia subito; un secondo dopo, le teste mobili si muovono per 3 s.

**Curve.** **«Curva delle dissolvenze»** dice come avanzano le dissolvenze della cue:

- **Lineare** (predefinita): la stessa velocità per tutto il tempo.
- **Morbida**: lenta all'inizio e alla fine.
- **Partenza lenta**: si muove appena all'inizio e accelera alla fine.
- **Partenza veloce**: avanza quasi di colpo, poi si assesta.

La curva vale per tutto ciò che la cue sfuma: dimmer che salgono e che scendono, colori, movimenti e i canali che rilascia. Ogni attributo ha anche la sua **CURVA**; «=» è quella della cue. Il tempo di rilascio della cuelist, un tempo fisso di **«Indietro»**, la dissolvenza in entrata di un gruppo e i livelli di FX sono sempre lineari. L'orologio della colonna MODIFICHE appare anche se la curva della cue non è lineare.

**Ventaglio di tempi.** In **«Tempi per fixture»**, **«Ventaglio»** distribuisce un ritardo o una dissolvenza su più fixture:

- Scegli **Ritardo** o **Fade**, e i tempi **da** e **a**.
- Scegli la forma: **Uno dopo l'altro**, **Dal centro** o **Dalle estremità**.
- Scegli le fixture: **«Selezionate in Programma»**, nell'ordine in cui le hai selezionate, o **«Di questa cue, per numero»**.
- **«Applica»** dà a ogni fixture il suo tempo nelle cue selezionate. L'altro suo tempo non cambia.

Con 5 fixture e un ritardo da 0 a 2 s, **Uno dopo l'altro** dà 0; 0,5; 1; 1,5 e 2 s. **Dal centro** dà 2; 1; 0; 1 e 2 s. **Dalle estremità** dà 0; 1; 2; 1 e 0 s. Se «da» è maggiore di «a», l'ordine si inverte. Il ventaglio scrive tempi fissi, non una regola: poi puoi cambiare ogni riga.

### Collegamenti e azioni

Seleziona una cue e apri **«Dettagli della cue»** › **«Collegamento e azioni»**, o clic destro › **«Collegamento e azioni…»**. Con più cue selezionate, modifichi l'ultima scelta.

**Collegamento.** **«Dopo questa cue»** dice quale cue suona dopo: **«La cue successiva»** (il caso normale) o un'altra cue della lista. **«Numero di volte»** dice quante volte salta il collegamento prima di proseguire; 0 vuol dire sempre. Un collegamento nuovo parte da 1.

- Con cue 5 → cue 2, 3 volte, i GO suonano 1 2 3 4 5, 2 3 4 5, 2 3 4 5, 2 3 4 5 e 6: le cue da 2 a 5 suonano 4 volte. Sotto, una frase lo spiega: «Le cue da 2 a 5 suonano 4 volte, poi la 6.»
- Con 0 (sempre), le cue si ripetono finché non vai a un'altra cue.
- Un collegamento in avanti salta delle cue; un collegamento alla cue stessa la ripete.
- Vale l'avvio della cue a cui salta: se la cue 2 ha Segui, il ciclo va avanti da solo.
- Quella cue è la successiva ovunque: il bordo verde della tabella, l'intestazione («▸ 2 ↺ 1/3»: il primo salto di tre; solo «↺» se è sempre), i playback, lo SHOW, il telefono e i LED del controller.
- BACK e «Vai alla cue» non contano i salti. Rilasciare la cuelist, o avviarla di nuovo, fa ripartire il conteggio.
- Fatti tutti i salti, la lista prosegue dopo la cue. La volta successiva che ci arriva, il conteggio riparte.
- Sull'ultima cue, il collegamento viene prima di **«Dopo l'ultima cue»**.
- In una cuelist «Chase» o «Timecode» i collegamenti non contano: la scheda lo dice. Il collegamento resta per quando la lista torna «Normale».
- Se elimini la cue a cui salta, il collegamento sparisce e un avviso lo dice.

**Azioni all'avvio.** **«+ Aggiungi un'azione ▾»** aggiunge ciò che fa la cue quando parte:

- **«GO di una cuelist»**, **«Vai a una cue…»** di un'altra cuelist e **«Rilascia una cuelist»**.
- **«Avvia una scena»** e **«Ferma una scena»**, nel suo gruppo se ne ha uno (come i tasti dello SHOW). Non cambiano la scena selezionata.
- **«Rilascia un gruppo»** (o tutti) e **«Rilascia tutti i playback»**.
- **«Imposta il BPM»**, da 20 a 300.

Ogni riga si modifica lì; la **×** la toglie. Le azioni si eseguono in ordine quando la cue parte con GO, Segui, Attendi o «Vai alla cue», anche la prima cue quando si avvia la cuelist. Non si eseguono con BACK, quando una modifica riapplica la cue né quando una super scena avvia la cuelist a metà.

- La cuelist stessa non compare: a questo serve il collegamento.
- Un'azione su una scena o cuelist eliminata mostra «(eliminata)» e non fa nulla.
- Se due cuelist si avviano a vicenda, il ciclo si ferma: una cuelist avviata da un'azione non avvia, con le sue azioni, una cuelist precedente in quella catena. Inoltre, la stessa cue non ripete le sue azioni prima di 200 ms.
- Copiare una cue non ne copia il collegamento né le azioni.

### Annullare

Le frecce **Annulla** e **Ripeti**, dopo il cestino, annullano e ripetono le modifiche della cuelist, una alla volta. Anche **Ctrl+Z** e **Ctrl+Y** (o **Ctrl+Maiusc+Z**). Passando il mouse dicono cosa annullano: «Annulla: Registra la cue 3».

- Si può annullare tutto ciò che cambia le cue o le opzioni della cuelist: registrare (anche da Programma, da un playback o dalla riga di comando), aggiornare, togliere, rilasciare, copiare, eliminare, il numero, i tempi, l'avvio, il nome, la nota, i segni S e B, il collegamento, le azioni e le opzioni in alto, tranne «GO a tempo» e «Ritmo battuto».
- Ogni cuelist conserva i suoi ultimi 50 passi.
- Più scatti della rotella (o valori scritti) sullo stesso tempo, a meno di 2 secondi l'uno dall'altro, sono un solo passo. Se torni al valore di partenza, il passo scompare.
- Dopo un annullamento resta selezionata la prima cue che cambia. Se la cuelist sta suonando, il palco cambia subito.
- In un campo con del testo scritto, Ctrl+Z annulla prima quel testo.
- Creare la cuelist e convertire i passi in cuelist non si possono annullare. Annulla mantiene i valori attuali delle palette e non riporta i canali dei fixture eliminati.
- Una priorità o un tempo di rilascio cambiati altrove (Live, Playback) restano come sono.
- La cronologia non si salva con lo show. Nuovo e Apri la svuotano, e così anche eliminare la cuelist.

### Convertire i passi o un chaser in cuelist

In una scena a passi, il tasto **«Converti in cuelist»** dell'intestazione converte ogni passo in una cue con ciò che cambia rispetto al precedente; i passi con tempo si concatenano con **Attendi**. I suoi gruppi, tasti dello SHOW e mappature passano a riprodurre la cuelist. I passi scompaiono. Le cue continuano a usare le palette dei passi.

In un chaser, ogni cue porta la scena del suo passo (i Move FX diventano FX) e rilascia ciò che il passo precedente aveva e questo no; i passi che non sono scene né effetti restano in una cue vuota con una nota. Le sue scene restano.

Un ping-pong va e torna; un ordine casuale suona in ordine. I tempi in beat passano a secondi con il BPM di quel momento. I passi di una copia per QLC+ tornano con il numero, il nome e la nota di ogni cue.

### Cuelist e QLC+

QLC+ non legge le cuelist: se apri questo show in QLC+, non ci sono. Per portarle in QLC+, premi **«QLC+»** nell'intestazione di una cuelist. **«Salva una copia per QLC+»** scrive un altro `.qxw` in cui ogni cuelist selezionata è una lista di passi di QLC+:

- **Sequence** (predefinito): un solo elemento in QLC+.
- **Chaser**: una scena per cue, in una cartella con il nome della cuelist.

Con **«Una lista di cue per ciascuna nella Console virtuale»** selezionata, la copia si apre sulla Console virtuale di QLC+, con un frame «Cuelist» e una lista di cue per ogni cuelist: «Avanti» è il GO, «Precedente» il BACK e «Stop» la rilascia. Il tuo show non cambia.

La casella **«Con i loro tasti della tastiera e del controller»** dà a ogni lista di cue i tasti della tastiera e del controller della sua cuelist: GO, Indietro, Pausa/Indietro e Rilascia diventano «Avanti», «Precedente», «Riproduci» e «Stop». Contano le mappature della cuelist e quelle del playback che la tiene nella pagina attuale. Ogni tasto di QLC+ prende un tasto della tastiera e un ingresso. I controlli OSC e quelli di un dispositivo non attivato non passano; la finestra lo avvisa.

Ogni passo contiene tutto ciò che mostra il palco in quella cue, e la sua nota dice il numero e il nome («Q2.5 · Strofa»). La dissolvenza della cue diventa la dissolvenza di entrata del passo, e la discesa quella di uscita. GO lascia il passo in attesa di «Avanti»; Attendi e Segui diventano un tempo. In una cuelist «Chase», i passi vanno da soli, in ciclo e nella sua direzione: ognuno dura un passo della sua velocità, con la sua dissolvenza. In una cuelist «Timecode», ogni passo dura fino al tempo della cue successiva. I passi hanno l'ID della cuelist: le super scene e i rack che la riproducono riproducono i passi.

Sotto ogni cuelist, la finestra avvisa di ciò che non passa:

- «Chase» con «BPM dello show» o con un ritmo battuto: i passi mantengono un BPM fisso, per il suo moltiplicatore: quello dello show in quel momento con «BPM dello show», altrimenti quello della cuelist. Non seguono il ritmo battuto.
- «Timecode»: i passi scorrono dal primo GO; non seguono il timecode MIDI né la timeline di una super scena. Il primo passo parte con il GO, non al tempo della sua cue. Su una cue senza tempo, si fermano fino a un GO.
- «Submaster»: i passi partono con un GO, non con il fader.
- «Override»: i passi si mescolano con ciò che c'è sotto come gli altri.
- Gli FX: solo i loro valori base.
- Il ritardo, anche quello di un attributo o di una fixture: il passo inizia subito.
- Una discesa diversa dalla dissolvenza: solo approssimata.
- I tempi per attributo e per fixture: solo la dissolvenza della cue.
- Le curve delle dissolvenze: solo lineari.
- Un Attendi più corto della dissolvenza, o in «Chase» e «Timecode» un passo più corto della sua dissolvenza: il passo successivo aspetta che finisca la dissolvenza.
- I collegamenti: i passi suonano in ordine. Se l'ultima cue è collegata sempre alla prima, i passi ricominciano.
- Le azioni delle cue: non vengono eseguite.
- In Sequence, i valori rilasciati vanno al loro valore di riposo. In Chaser, sono lasciati a QLC+.
- Con «Resta», GO sull'ultima cue ferma i passi.
- GO a tempo e ritmo battuto: i passi non vanno a tempo.
- «Movimento al buio»: le fixture si muovono mentre salgono.
- «Rilasciata dal GO di un'altra cuelist»: quelle liste vanno in un frame «Cuelist (una alla volta)», dove avviarne una ferma le altre.

Una cuelist che ha già la sua lista di cue nella Console virtuale (uno show di QLC+ di cui hai convertito i passi in cuelist) non ne riceve un'altra: quella di prima la riavvia. Prova la copia in QLC+ prima di usarla dal vivo: QLC+ fa le dissolvenze a modo suo.

### Cue list di QLC+

**«Nuova» › «Cue list di QLC+…»** converte in cuelist i chaser, le sequenze e le liste di cue della Console virtuale di uno show di QLC+: dello show aperto o di un altro («Da un altro show…»). Seleziona quelle che vuoi convertire. Prima vengono le liste di cue della Console virtuale, selezionate; poi i chaser e le sequenze che nessuna lista usa. Quelle che hanno già una cuelist con il loro nome non sono selezionate.

Ogni passo è una cue con ciò che cambia; i passi a tempo si concatenano con Attendi e quelli che aspettano sono GO. Una lista di cue della Console virtuale dà il suo nome alla cuelist.

- **«Sostituire gli originali»** (solo nello show aperto): la cuelist prende il loro posto e il loro ID. I loro gruppi, tasti dello SHOW e mappature passano alla cuelist, ma QLC+ non li vede più. Senza questa casella, gli originali restano e lo show suona uguale in QLC+.
- **«Metterle sui playback liberi di questa pagina»**. Se non ci stanno tutte, l'avviso dice quante restano fuori.
- **«Portare i loro tasti della tastiera e del controller come mappature»**: «Avanti», «Precedente», «Riproduci» e «Stop» di ogni lista di cue diventano GO, Indietro, Pausa/Indietro e Rilascia della sua cuelist. Non passano i controlli OSC, i tasti con Ctrl, Alt o Maiusc, gli ingressi di un universo senza controller in questo show né i controlli che hanno già una mappatura. L'avviso dice quante mappature vengono create e quali controlli restano fuori. Un tasto di una Akai APC arriva con i suoi LED, come quando lo apprendi.

Da un altro show entrano solo le cuelist, sulle fixture di questo show che corrispondono alle sue (prima lo stesso indirizzo e lo stesso modello). L'avviso dice quante fixture mancano.

Non passano: l'ordine casuale (suonano in ordine), i beat (passano a secondi con il BPM attuale), il fader laterale delle liste di cue e i passi che non sono scene né effetti. Le liste di cue nel vecchio formato di QLC+ non si possono convertire: aprile e salvale prima in QLC+.

Aprendo uno show di QLC+ con liste di cue e senza cuelist, l'avviso «Aperto» ricorda dove convertirle.

## Cartelle

Le cartelle ordinano l'elenco delle scene (anche nello SHOW, nelle super scene, nei rack e quando aggiungi scene a un gruppo).

- **Creare**: il tasto della cartella con **+** accanto alla ricerca, o **«Nuova cartella…»** nel menu di una scena.
- **Inserire scene**: trascinale sulla cartella, o clic destro › **«Sposta nella cartella»**. Funziona con più scene selezionate.
- **Togliere**: clic destro › **«Togli dalla sua cartella»**.
- **Rinominare o eliminare**: il tasto **⋯** della cartella (o clic destro): **«Rinomina cartella»** ed **«Elimina cartella (le sue scene restano)»**.
- Fai clic sull'intestazione di una cartella per comprimerla o espanderla.

Le cartelle si salvano con lo show.

## Importare ed esportare

- **Esportare**: il tasto di esportazione dell'intestazione salva la scena in un file `.dtcfx` («Effetti di DMX Total Control») per caricarla in altri show. Funziona anche con i rack.
- **Importare**: **«Nuova» › «Importa da un file…»**. Le fixture vengono abbinate a quelle di questo show; se qualcuna del file non c'è, la sua parte viene omessa e viene segnalato.
- **Le scene di un altro show intero**: in [Patch › Importa](patch.md#importare-da-un-altro-show), «Scene di un altro show…».
- **Per QLC+**: «QLC+» nell'intestazione di una cuelist ([Cuelist e QLC+](escenas.md#cuelist-e-qlc)).
- **Da QLC+**: «Nuova» › «Cue list di QLC+…» ([Cue list di QLC+](escenas.md#cue-list-di-qlc)).

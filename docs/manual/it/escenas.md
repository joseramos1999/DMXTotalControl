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

## L'intestazione dell'editor

- **Il nome**: fai clic per cambiarlo.
- **La cartella** (icona della cartella): spostare la scena in una cartella.
- **«Prova»** / **«Ferma»**: la riproduce ora, fuori dai gruppi. Le cuelist hanno il loro GO.
- **«Aggiungi al gruppo…»**: la aggiunge a un gruppo di Live.
- **Converti in passi** (rack, FX, Colour FX, Move FX, Curve FX e chaser): registra un ciclo in una nuova scena a passi, solo con i canali a cui la scena dà un valore: gli altri, anche se hanno un valore predefinito (il pan o lo zoom al centro di molte teste mobili), non entrano nei passi.
- **Converti in cuelist** (passi): vedi [Cuelist](escenas.md#cuelist).
- **Esporta**, **«Duplica»** ed **«Elimina»** (viene rimossa anche dai suoi gruppi e rack).

## Come si riproduce una scena

Sotto l'intestazione c'è il pannello di riproduzione. Mostra ciò che ha senso per ogni tipo:

- **«Entrata»** e **«Uscita»**.
- **«Tempo di ciclo»** (o **«Tempo per passo»** in un Colour FX) e **«Velocità»** (da un ciclo al minuto a dieci al secondo; doppio clic: un ciclo al secondo), con **×½** e **×2**.
- **«Entrata scaglionata»** (statiche e passi): ogni fixture inizia la sua dissolvenza questo tempo dopo la precedente, in ordine di selezione.
- **«Guidata da»**: **«Durata»** (i tempi normali), **«BPM»** (i tempi diventano beat e seguono il tempo dello show, anche le dissolvenze di ogni passo; il tempo di dissolvenza incrociata di un gruppo di Live e le dissolvenze dei blocchi di una super scena restano in secondi), **«BeatGO»** o **«Impulso»** (avanza con il tasto BeatGO o con i picchi dell'ingresso audio). Con BeatGO o Impulso, **«Divisioni»** (/2, ×2).
- **«Ritmo battuto»** (sequenze e chaser): il ritmo battuto premendo il suo tasto più volte di seguito ([Live](directo.md#ritmo-di-una-scena-a-passi)), in BPM. Finché ce l'ha, i suoi passi vanno a quel ritmo e non secondo i loro tempi. **«Rimuovi»** torna ai suoi tempi. Con «BPM», un beat è un passo di quel ritmo: una dissolvenza di 0,5 b dura mezzo passo.
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

**Ctrl+clic** e **Maiusc+clic** selezionano più passi: quando cambi un tempo, cambia in tutti. I passi applicano le palette per valore, non per riferimento.

### FX

Una scena FX ha dei **valori base** e uno o più **livelli di FX**. Si crea registrando dal programmatore con degli FX (vedi [FX](programar.md#fx)).

- A sinistra, l'elenco degli FX: parametro, fixture e velocità. Il cestino rimuove un livello.
- A destra, i controlli del livello scelto (gli stessi di Programma) e la sua anteprima animata. Le modifiche si vedono dal vivo mentre la scena è in riproduzione.
- **«Collega»**: i livelli collegati condividono velocità e timing.
- Se alcune fixture fanno l'FX a modo loro, viene segnalato: se qui cambi la forma, la dimensione, la direzione o la fase, tutte faranno lo stesso.
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

Si riproducono come prima e hanno il loro editor (forma, larghezza, altezza, rotazione, posizione, fase, distribuzione tra le fixture, simmetrico, relativo…). Un avviso offre **«Converti in FX»**: passano all'editor degli FX, anche nel programmatore, e i gruppi, i tasti dello SHOW e le mappature li conservano.

### Rack di effetti

Più scene ed effetti che vengono riprodotti insieme come uno solo, ognuno sulle sue fixture.

- In **«Aggiungi»** (a destra), fai clic su una scena per metterla nel rack.
- In **«Nel rack (N)»**, la **×** la rimuove.

Un rack non può contenere una scena che riproduca il rack stesso.

### Chaser

I chaser degli show di QLC+ vengono riprodotti normalmente, ma non hanno un editor. Puoi convertirli in passi con il tasto di conversione dell'intestazione.

## Super scene

Una **super scena** è una timeline con tracce di scene e di audio. Si avvia come qualsiasi scena (in un gruppo, nello SHOW, con una mappatura). Il suo editor occupa tutta la pagina; il tasto dell'elenco, in alto a sinistra, torna all'elenco delle scene.

### La barra degli strumenti

- **«Riproduci»** / **«Ferma»** (**Spazio**): dal cursore. **⏮** torna all'inizio. Accanto si vede il tempo (e la battuta).
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

### Lavorare con i blocchi

- **Trascina** un blocco per spostarlo, anche su un'altra traccia. Trascina i suoi **bordi** per cambiarne la durata.
- Con il blocco selezionato, le **maniglie** degli angoli superiori sono le sue dissolvenze in entrata e in uscita.
- Quando allunghi un blocco, la scena **si ripete in ciclo** (si vedono dei segni a ogni giro) o si **allunga** perché un ciclo duri tutto il blocco. Si cambia nel menu del blocco o nella barra in basso («In ciclo» / «Allungata»).
- **Clic destro**: «Ripeti la scena nel blocco» / «Allunga la scena per adattarla al blocco», «Duplica», «Blocca» / «Sblocca», «Apri la scena» ed «Elimina».
- **Doppio clic** su un blocco apre la sua scena. **Canc** elimina il blocco selezionato.
- La barra in basso mostra il blocco selezionato: **Inizio**, **Durata**, **Entrata**, **Uscita**.

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

## Cuelist

Una **cuelist** è un elenco di cue che avanzano con GO. Funziona con il **tracking**: ogni cue salva **solo ciò che cambia** e il resto prosegue dalle cue precedenti. Un GO cambia solo i canali che cambiano: senza cali e senza riprendere ciò che non tocca.

Le cuelist si registrano dal programmatore, si gestiscono in [Playback](playbacks.md) e si modificano qui.

### Opzioni della cuelist

In cima all'editor:

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

In una finestra stretta, parte delle opzioni si comprime: **«Più opzioni»** le mostra.

### La tabella delle cue

| Colonna | Cos'è |
|---|---|
| **CUE** | Il numero. Accetta decimali: la 2.5 va tra la 2 e la 3. Doppio clic per cambiarlo. Una matita indica che la cue è caricata nel programmatore. |
| **NOME** | Il suo nome. |
| **AVVIO** | **GO** (aspetta un GO), **Segui** (quando finisce la cue precedente, più un tempo) o **Attendi** (un tempo dopo il GO della cue precedente). |
| **RITARDO** | Quanto aspetta la cue prima di iniziare la sua dissolvenza. |
| **FADE** | Il tempo di dissolvenza. |
| **DISCESA** | La dissolvenza dei dimmer che scendono. «=» è uguale alla dissolvenza. |
| **MODIFICHE** | Quanti canali cambia, e i segni **FX**, **[R]** (valori rilasciati) e l'icona della palette se usa palette. |
| **S** | **Salta** (solo questa cue): ciò che cambia non passa alle successive. |
| **B** | **Blocco**: Registra e Aggiorna salvano anche i valori uguali a quelli che eredita. |
| **NOTA** | Una nota per te. |

Per cambiare un tempo, fai clic su di esso e scrivi i secondi, o usa la **rotella** del mouse sul tempo premuto: ±0,1 s (con Maiusc, 1 s). Con **Ctrl+clic** o **Maiusc+clic** selezioni più cue e cambi i loro tempi, il loro avvio o i loro segni insieme.

La cue in riproduzione ha una barra verde con l'avanzamento della dissolvenza; la successiva, un bordo verde. In alto si vede lo stato («Q 3 · Strofa ▸ 4» o «Rilasciata · GO avvia la 1») e i tasti **Indietro**, **GO** e **rilascia**, gli stessi del suo playback.

### Registrare e modificare le cue

- **«Registra la cue N»**: il programmatore diventa una nuova cue dopo quella selezionata, solo con ciò che cambia rispetto a ciò che eredita.
- **«Aggiorna ▾»**: unisce il programmatore alla cue selezionata: **«Aggiorna la cue N (vale anche per le successive)»** o **«Solo questa cue (le successive mantengono il loro aspetto)»**.
- **«Carica ▾»** (o doppio clic su una cue): la porta nel programmatore: **«Ciò che mostra il palco nella cue N»** o **«Solo ciò che cambia la cue N»**. Poi aggiornala dalla barra «Modifica della cue…» di Programma.
- **«Togli ▾»**: con i canali del programmatore, **«Togli dalla cue N i canali del programmatore (tornano a ereditare)»** o **«[Rilascia] i canali del programmatore nella cue N»** (da quella cue tornano a ciò che c'è sotto: altre scene o il loro valore di riposo).
- **Copia** alla fine: **«Copia ciò che cambia la cue N»** o **«Copia tutto l'aspetto della cue N (cue di blocco)»**.
- **«Numero…»**: un altro numero per la cue selezionata. Spostare una cue cambia ciò che ereditano quelle che vengono dopo.
- **Elimina**: con **«Mantieni l'aspetto delle cue successive»** spuntata, ciò che cambia la cue passa alla successiva e quelle dopo appaiono uguali.

In un elenco stretto (o in una lingua con parole lunghe), «Aggiorna», «Carica» e «Togli» restano con la loro icona e la loro freccia **▾**; passando il mouse se ne vede il nome.

Il clic destro su una cue ha **«Vai alla cue N»**, i due modi di caricare, **«Aggiorna con il programmatore»**, **«Numero…»** ed **«Elimina…»**.

Le cue possono usare **palette per riferimento** e avere **livelli di FX**, che si ereditano da una cue all'altra.

### Il foglio di tracking

**«Foglio di tracking»** mostra ciò che il palco ha nella cue selezionata, fixture per fixture («In scena: cue N»). Se la finestra è larga, si vede da solo accanto alle cue.

- Le etichette colorate dicono cos'è ogni valore: **«Modifiche»** (lo cambia questa cue), **«Ereditati»** (viene da una cue precedente, che viene indicata: «Q2»), **«Blocco»** e **«Rilasciati»** (barrati, con [R]).
- In alto si vedono le palette che usa la cue.
- Fai clic sui canali per sceglierli e premi **«Rilascia»** (li mette in [RELEASE] da questa cue) o **«Togli»** (tornano a ereditare dalle cue precedenti).

### Convertire i passi in cuelist

In una scena a passi, il tasto **«Converti in cuelist»** dell'intestazione converte ogni passo in una cue con ciò che cambia rispetto al precedente; i passi con tempo si concatenano con **Attendi**. I suoi gruppi, tasti dello SHOW e mappature passano a riprodurre la cuelist. I passi scompaiono.

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

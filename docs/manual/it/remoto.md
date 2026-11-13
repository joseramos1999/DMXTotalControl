# Controllo remoto

Il **controllo remoto** mostra le pagine dello [SHOW](show.md) nel browser di un telefono o di un tablet collegati alla stessa rete wifi del computer. Non serve installare niente: funziona in entrambe le direzioni, quindi ciò che tocchi sul telefono cambia sul computer e ciò che cambia sul computer si vede sul telefono.

## Attivarlo

1. Sul computer, apri **Opzioni** (F9) › **Controllo remoto**.
2. Attiva **«Controllo remoto attivo»**.
3. Se vuoi, scrivi un **PIN**. Senza PIN, chiunque si trovi sulla rete può usarlo.
4. Sotto compaiono gli **indirizzi** del computer (per esempio `http://192.168.1.20:8800/`), uno per ogni rete a cui è collegato, con il nome della rete davanti. Il wifi viene per primo. Il tasto per copiare mette l'indirizzo negli appunti. Se il computer ha un **nome sulla rete**, il primo è quello del nome, per esempio `http://dmxtotalcontrol.local:8800/` (più sotto).

## Collegare il telefono o il tablet

1. Collega il telefono alla **stessa rete wifi** del computer.
2. Apri il browser e scrivi uno degli indirizzi, oppure inquadra il codice QR (più sotto).
3. Se c'è un PIN, scrivilo e premi **OK**. Viene chiesto una sola volta su ogni dispositivo.

In Opzioni si vede quanti dispositivi sono collegati.

> **Consiglio:** aggiungi la pagina alla schermata Home del telefono: si apre come un'applicazione, con l'icona del programma.

> **Consiglio:** se il telefono non si collega, controlla che entrambi siano sulla stessa rete e che il firewall di Windows permetta a DMX Total Control di usare la rete. Se un altro programma usa la porta, cambiala in Opzioni (l'avviso compare in rosso).

> **Consiglio:** la prima volta che attivi il controllo remoto, Windows chiede se DMX Total Control può usare la rete. Consentilo sulle reti **private**. Se Windows considera il wifi una rete **pubblica**, il telefono non raggiunge il computer: impostala come privata nelle impostazioni di rete di Windows.

> **Consiglio:** se il wifi del locale non lascia che i dispositivi si vedano tra loro, attiva sul computer l'**hotspot mobile** di Windows e collega il telefono a quello. Il remoto è sempre in `http://192.168.137.1:8800/`.

## Inquadrare il codice QR

Con il controllo remoto attivo, sotto gli indirizzi compare un **codice QR**. Con questo il remoto si apre senza scrivere niente:

1. Collega il telefono o il tablet alla **stessa rete wifi** del computer.
2. Inquadra il codice con la fotocamera.
3. Tocca il link che compare.

- Il codice contiene l'indirizzo con il bordo colorato: all'inizio, il primo indirizzo numerico, che funziona su tutti i telefoni. Per usarne un altro, fai clic sulla sua riga o sul suo tasto del codice QR.
- Se c'è un PIN, il codice contiene anche quello: il telefono entra senza chiederlo. Il PIN non resta nella barra degli indirizzi del browser.
- Per questo, con un PIN, chiunque fotografi il codice può entrare.
- **«Più grande»** mostra il codice in grande, per inquadrarlo da un paio di metri.
- Nel tema **Notte** il codice è più tenue, per non abbagliare.

## Trovarlo per nome

Il router può dare al computer un altro indirizzo da un giorno all'altro. Il nome non cambia: con il controllo remoto attivo, il telefono apre `http://dmxtotalcontrol.local:8800/` anche se l'indirizzo è un altro. Così l'icona nella schermata Home continua a funzionare.

Se apri il remoto con il codice QR o con un indirizzo numerico, il telefono controlla in background se raggiunge il computer per nome. Se ci riesce, passa da solo a `http://dmxtotalcontrol.local:8800/`, sulla stessa pagina e senza chiedere il PIN. Aggiungi l'icona alla schermata Home dopo questo cambio: la barra degli indirizzi mostra già il nome.

- In Opzioni, l'indirizzo **«Per nome»** è il primo dell'elenco.
- Il nome si cambia in **Nome sulla rete**. Accetta lettere, numeri e trattini; le maiuscole diventano minuscole e gli spazi trattini. Se è vuoto, non c'è nessun nome.
- Funziona su iPhone e iPad, e sui telefoni con Android 12 o successivo. Su quelli più vecchi usa un indirizzo numerico o il codice QR.
- Non funziona se il telefono ha una VPN attiva o usa i dati mobili.
- Se il telefono non raggiunge il computer per nome, resta sull'indirizzo numerico senza avvisare.
- Se poi il nome smette di rispondere, il telefono torna all'indirizzo numerico, purché quell'indirizzo risponda.
- Se un altro computer della rete usa già il nome (per esempio un altro DMX Total Control), questo prende il successivo, `dmxtotalcontrol-2`, e Opzioni lo indica sotto il nome.
- Se la porta di rete 5353 non è disponibile, o se altri computer usano già il nome e da `-2` a `-9`, compare un avviso in rosso. Gli indirizzi numerici continuano a funzionare.

## Cosa si vede

In alto c'è la barra del controllo remoto:

- Il **nome dello show** e le **schede** delle pagine dello SHOW. Il telefono ricorda l'ultima pagina che hai aperto.
- Un **punto** che indica se c'è connessione con il computer.
- **BO**: blackout.
- Il tasto **schermo intero**.

Sotto, la pagina con le sue sezioni, come sul computer:

- **Tasti**: si premono con il dito, anche quelli flash (finché il dito è sopra). Si illuminano quando sono attivi.
- **Fader**: si trascinano; quelli che hanno il tasto **Flash** ce l'hanno sotto.
- **Manopole e ruote**: si muovono trascinando il dito su e giù. I fader e le manopole seguono lo stesso [intervallo](show.md#fader) del computer e mostrano il valore reale.
- **Gruppi**: le loro scene come tasti e il loro master.
- **Playback**: le strisce con il loro nome, la cue in riproduzione e la successiva, **GO**, **Indietro**, il loro fader e **Flash**. Un fader A/B mostra «A/B» e a che punto è la sua dissolvenza, con il bordo tratteggiato. Le sezioni che seguono la pagina attuale hanno ◀ ▶ per cambiarla.
- **Ruota dei colori** e **griglia XY**, con i loro tasti **Rilascia** e **Centro**.
- **Orologio**, **Ritmo** (**GO**, **Stop** e **BeatGO**; i tasti di scene a passi, di cuelist o di FX in «BPM dello show» contengono la loro barra del ritmo), **note**, **in riproduzione ora** e **immagine**.
- **Timer**: è lo stesso del computer, con **Avvia** / **Pausa** e **Azzera**. Se lo avvii sul telefono, conta anche sul computer. Se lo metti in pausa sul computer, si ferma su tutti i telefoni. Nel conto alla rovescia diventa arancione nell'ultimo minuto e rosso quando arriva a zero.
- **Livelli DMX**: i 512 canali dell'universo scelto, come sul computer: più alto è il valore, più la casella è luminosa. Il telefono li riceve circa 8 volte al secondo, solo mentre quella pagina è sullo schermo.
- **Mixer dal vivo**: le stesse strisce del [mixer](directo.md#mixer) del computer, una accanto all'altra: «Tutte le fixture» e una per ogni gruppo di fixture. Se non ci stanno, si scorrono di lato con il dito. Ogni striscia ha il dimmer, **Full**, **Blackout**, **Tonalità**, **Saturazione**, **Ripristina colore**, **Strobo** con la sua velocità, **Flash**, **Flash escl.** e **Full escl.**
  - **Flash** e **Flash escl.** funzionano finché il dito è sopra. Si rilasciano se il telefono perde la connessione, si blocca o cambi app.
  - Quando si rilasciano si spengono, anche se erano bloccati accesi sul computer (Ctrl+clic).
  - Il bordo della striscia diventa arancione con un flash o un full, e rosso con il blackout.
  - Ciò che cambi sul telefono cambia sul computer, e viceversa.

Se un playback ha qualcosa assegnato, il controllo remoto aggiunge una pagina **«Playback»** con le strisce della pagina di playback attuale.

Il controllo remoto usa lo stesso [tema](opciones.md#generale) del programma ed è nella lingua del telefono se è spagnolo, portoghese, francese, italiano o tedesco (altrimenti, in inglese).

## Se la connessione cade

Se il telefono perde la connessione, compare «Nessuna connessione con DMX Total Control. Nuovo tentativo…» e si ricollega da solo. Ciò che il dito teneva premuto (un flash, un tasto) viene rilasciato sul computer, perché niente resti acceso.

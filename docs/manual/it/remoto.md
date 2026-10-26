# Controllo remoto

Il **controllo remoto** mostra le pagine dello [SHOW](show.md) nel browser di un telefono o di un tablet collegati alla stessa rete wifi del computer. Non serve installare niente: funziona in entrambe le direzioni, quindi ciò che tocchi sul telefono cambia sul computer e ciò che cambia sul computer si vede sul telefono.

## Attivarlo

1. Sul computer, apri **Opzioni** (F9) › **Controllo remoto**.
2. Attiva **«Controllo remoto attivo»**.
3. Se vuoi, scrivi un **PIN**. Senza PIN, chiunque si trovi sulla rete può usarlo.
4. Sotto compaiono gli **indirizzi** del computer (per esempio `http://192.168.1.20:8800/`), uno per ogni rete a cui è collegato. Il tasto per copiare mette l'indirizzo negli appunti.

## Collegare il telefono o il tablet

1. Collega il telefono alla **stessa rete wifi** del computer.
2. Apri il browser e scrivi uno degli indirizzi.
3. Se c'è un PIN, scrivilo e premi **OK**. Viene chiesto una sola volta su ogni dispositivo.

In Opzioni si vede quanti dispositivi sono collegati.

> **Consiglio:** aggiungi la pagina alla schermata Home del telefono: si apre come un'applicazione, con l'icona del programma.

> **Consiglio:** se il telefono non si collega, controlla che entrambi siano sulla stessa rete e che il firewall di Windows permetta a DMX Total Control di usare la rete. Se un altro programma usa la porta, cambiala in Opzioni (l'avviso compare in rosso).

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
- **Playback**: le strisce con il loro nome, la cue in riproduzione e la successiva, **GO**, **Indietro**, il loro fader e **Flash**. Le sezioni che seguono la pagina attuale hanno ◀ ▶ per cambiarla.
- **Ruota dei colori** e **griglia XY**, con i loro tasti **Rilascia** e **Centro**.
- **Orologio**, **Ritmo** (**GO**, **Stop** e **BeatGO**; i tasti di scene a passi, di cuelist o di FX in «BPM dello show» contengono la loro barra del ritmo), **note**, **in riproduzione ora** e **immagine**.

I **timer**, il **mixer dal vivo** e i **livelli DMX** compaiono come «Solo sul computer».

Se un playback ha qualcosa assegnato, il controllo remoto aggiunge una pagina **«Playback»** con le strisce della pagina di playback attuale.

Il controllo remoto usa lo stesso [tema](opciones.md#generale) del programma ed è nella lingua del telefono se è spagnolo, portoghese, francese, italiano o tedesco (altrimenti, in inglese).

## Se la connessione cade

Se il telefono perde la connessione, compare «Nessuna connessione con DMX Total Control. Nuovo tentativo…» e si ricollega da solo. Ciò che il dito teneva premuto (un flash, un tasto) viene rilasciato sul computer, perché niente resti acceso.

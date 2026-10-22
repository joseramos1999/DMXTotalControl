# Opzioni

![La pagina Opzioni](../img/opciones.png)

**Opzioni** (F9) raccoglie le impostazioni del programma. A sinistra ci sono le impostazioni generali, l'avvio, il blocco, il salvataggio automatico, la riproduzione e l'audio; a destra, il controllo remoto, la libreria di fixture, gli aggiornamenti, l'aiuto e «Informazioni».

Le impostazioni di questa pagina sono del computer, non dello show: valgono per tutti gli show che apri.

## Generale

- **Lingua**: «Lingua del sistema», Español, English, Português, Français, Italiano o Deutsch. Si applica al prossimo avvio del programma.
- **Tema**: **Scuro**, **Chiaro**, **Alto contrasto** o **Notte (attenuato)**. Cambia subito. Chiaro, per sale illuminate; Alto contrasto, per leggere da lontano; Notte, molto scuro e in rosso, per una regia al buio senza abbagliarti. Il [controllo remoto](remoto.md) segue lo stesso tema.
- **Dimensione dell'interfaccia**: dall'80 % al 150 %, più piccola per i portatili piccoli e più grande per gli schermi touch. Si applica al prossimo avvio del programma.
- **Apri l'ultimo show all'avvio**.

> **Consiglio:** il tema Chiaro va meglio nelle prove di giorno o all'aperto; Alto contrasto, quando c'è molta luce intorno allo schermo.

## SHOW e blocco

- **Apri lo SHOW a schermo intero all'avvio**: per installazioni e chioschi. Il programma si avvia nella pagina Show a schermo intero.
- **…e bloccato**: inoltre si avvia bloccato: solo lo SHOW, senza navigazione. Per uscire, tieni premuto il tasto del lucchetto (con la password, se c'è). Vedi [Show](show.md#schermo-intero-e-blocco).
- **Blocca schermate**: **«Blocca…»** / **«Sblocca…»**. È lo stesso del lucchetto della barra superiore.

### Bloccare le schermate

Il blocco evita che qualcosa cambi per errore durante lo show. Live e lo SHOW continuano a funzionare, e i playback restano in riproduzione.

1. Premi il **lucchetto** della barra superiore (o «Blocca…» qui).
2. Scegli cosa bloccare:
   - **Setup**: Patch, Fixture, Uscite, Mappature e Opzioni. Non si può nemmeno aprire o creare un altro show.
   - **Programmazione**: Programma e Scene. In Playback non si può registrare, aggiornare né assegnare.
   - **Modifica dello SHOW**: le pagine dello SHOW si usano, non si modificano.
3. Se vuoi, scrivi una **password** (è facoltativa).
4. Premi **«Blocca»**.

Le pagine bloccate hanno un lucchetto nella barra laterale. Quando ne apri una, viene chiesta la password (se c'è) e si sblocca tutto. Per sbloccare, premi il lucchetto della barra superiore. **«Rimuovi la password»** si trova nella stessa finestra, al momento del blocco.

## Salvataggio automatico

- **Salva una copia di recupero**, **Ogni (minuti)**: da 1 a 60.

Il salvataggio automatico non tocca il file dello show: ogni pochi minuti, se ci sono modifiche non salvate, salva a parte una copia di recupero, e il punto **●** resta. Solo **Salva** scrive il file; la versione che c'era prima del primo salvataggio di ogni sessione si conserva come `.bak`. La copia si cancella quando salvi, quando chiudi (salvando o no) e quando apri un altro show. Se il programma si chiude in modo inatteso, quando lo riapri ti propone la copia («Recuperare lo show non salvato?») con il nome dello show e l'ora della copia: recuperandola si apre quello show con le sue modifiche, ancora non salvate; salvalo per conservarle. Nel frattempo l'ultimo show è già aperto com'era salvato (con «Apri l'ultimo show all'avvio»): «Scarta» lo lascia così. Nella barra superiore si vede l'ora dell'ultima copia («copia di recupero…»); se non riesce (un disco pieno), compare un avviso in rosso.

## Riproduzione

- **Priorità predefinita delle nuove scene**: da 0 a 10. Se due scene controllano lo stesso canale, vince quella con priorità più alta (vedi [Priorità](directo.md#priorità)).
- **Priorità delle modifiche dal vivo**: ciò che metti in Programma vince sulle scene con questa priorità o inferiore. Con 10, su tutte le scene.
- **Quando parte una scena, le modifiche dal vivo**:
  - **«Restituiscono alla scena i suoi canali»**: i canali che la scena tocca escono dal programmatore.
  - **«Restano sempre»**.
  - **«Vengono tutte azzerate»**: il programmatore si svuota.
- **Il grand master agisce su**: **«Canali di intensità»** (la luce: i dimmer e i colori delle fixture senza dimmer) o **«Tutti i canali»**.

## Audio

- **Ingresso audio**: il microfono o l'ingresso di linea che ascoltano **Impulso** e le matrici audio. **«Predefinito del sistema»** usa quello del sistema operativo. Impulso cambia ingresso subito; una matrice audio già in riproduzione, al suo prossimo avvio. Un ingresso scelto che ora non è collegato appare come «(non collegato)», e nel frattempo si usa quello predefinito.

## Show recenti

L'elenco degli ultimi show aperti (anche nella freccia **▾** della barra superiore). **«Svuota elenco»** lo cancella.

## Controllo remoto

Per usare le pagine dello SHOW da un telefono o un tablet sulla stessa rete wifi, senza installare niente:

- **Controllo remoto attivo**.
- **Porta**: cambiala se un altro programma la usa (di default, 8800). Se è occupata, viene segnalato in rosso.
- **PIN**: viene chiesto una volta su ogni dispositivo. Se è vuoto, chiunque sulla rete può usarlo.
- Gli **indirizzi** da aprire nel browser del telefono, con un tasto per copiarli.
- Quanti dispositivi sono collegati.

Vedi [Controllo remoto](remoto.md).

## Libreria di fixture

Indica quante definizioni di fixture sono disponibili (la libreria integrata, le generiche e le tue) e permette di ampliarla:

- **Open Fixture Library**: **«Scarica»** (serve internet). Una volta scaricata, si vede quante fixture ha e quando è stata scaricata, e il tasto diventa **«Aggiorna»**: porta le fixture nuove e quelle migliorate, e conserva le tue fixture e quelle che la libreria non ha più. Se ci sono novità, viene segnalato e il tasto si evidenzia (vedi [Fixture](fixtures.md#open-fixture-library)).
- **Importa file di fixture**: **«Importa…»** definizioni `.qxf` (il formato di QLC+), file `.gdtf` o uno `.zip` che li contenga.
- **Fixture GDTF**: **«GDTF-Share»** apre il suo sito; **«Importa GDTF…»** importa file, cartelle o lo `.zip` del suo download multiplo. Indica quante fixture GDTF ci sono nella libreria.
- **Importa le mie fixture di QLC+**: **«Importa»** copia le fixture che hai creato in QLC+.
- **Cartella delle fixture utente**: **«Apri cartella»**.

Tutto questo è spiegato in [Fixture](fixtures.md).

## Aggiornamenti

- **Cerca aggiornamenti all'avvio**: una volta al giorno, su GitHub: il programma e, se hai scaricato la Open Fixture Library, se ha fixture nuove (lo dice un avviso). Non si scarica né si installa niente senza chiederti. Senza internet, non dà fastidio.
- **Includi versioni di prova**: le novità prima, meno collaudate.

Sotto si vede lo stato («Hai l'ultima versione», «È disponibile la versione…») e i tasti:

- **«Cerca aggiornamenti»**: controlla ora (anche la Open Fixture Library).
- **«Novità»**: le novità di quella versione.
- **«Scarica e installa»**: scarica l'installer e verifica la sua impronta SHA-256 prima di salvarlo. Se una versione non pubblica la sua impronta, si può scaricare solo dalla sua pagina («Apri su GitHub»).
- **«Installa ora»**: il programma si chiude (prima chiede se salvare lo show), si installa la nuova versione e si riapre.
- **«Alla chiusura»** / **«Non alla chiusura»**: installarla quando chiudi il programma.
- **«Apri su GitHub»**: la pagina della versione.

Quando c'è una nuova versione, compare anche un avviso sopra le pagine con gli stessi tasti. **«Più tardi»** toglie l'avviso per quella versione; resta disponibile qui.

## Aiuto

- **Manuale**: **«Apri il manuale»** apre questo manuale sul suo indice. F12 o il tasto **«?»** della barra superiore lo aprono alla pagina in cui ti trovi. È nella lingua del programma (spagnolo, inglese, portoghese, francese, italiano o tedesco); le schermate sono quelle della versione spagnola.
- **Guida ai primi passi**: **«Avvia la guida»** riapre la guida (vedi [Primi passi](primeros-pasos.md#la-guida)).
- **Show di esempio**: si trovano nella freccia **▾** accanto ad «Apri» della barra superiore, in «Esempi» (vedi [l'indice](index.md#la-guida-e-gli-show-di-esempio)).

## Informazioni

La versione del programma, la sua licenza (software libero, Apache 2.0) e i crediti.

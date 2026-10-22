# Patch

![La pagina Patch](../img/patch.png)

**Patch** (F5) contiene le fixture del tuo impianto: cosa sono, in quale universo e indirizzo si trovano, il loro numero, i gruppi di fixture e dove si trovano sul palco.

Nell'intestazione scegli la vista e trovi gli strumenti per aggiungere:

- **«Elenco»**: tutte le fixture per indirizzo.
- **«Canali»**: i 512 canali di ogni universo, con le fixture sopra.
- **«Pianta 2D»**: il palco visto dall'alto.
- **«Importa»**, **«Matrice LED»** e **«Aggiungi fixture»**.

## Aggiungere fixture

1. Premi **«Aggiungi fixture»**.
2. Scegli il **Produttore** (scrivi in «Cerca produttore…» per filtrare) e il **Modello**.
3. Nel pannello **Patch**:
   - **Nome**: quello che vedrai in tutto il programma (di default, quello del modello).
   - **Modalità**: la modalità DMX, con il suo numero di canali.
   - **Quantità**: quante uguali.
   - **Universo** e **Indirizzo**: l'indirizzo si imposta da solo sul primo libero dell'universo.
   - **Spaziatura**: canali liberi tra una fixture e la successiva.
   - Con più di una, **«Creane un gruppo»** e il nome del **Gruppo**.
4. In basso si vede quali indirizzi useranno («Usa gli indirizzi da 1 a 20»). Se non entrano in ciò che resta dell'universo, passano al successivo.
5. Premi **«Patcha fixture»** o **«Patcha N fixture»**.

La finestra resta aperta per continuare ad aggiungere, con l'indirizzo già sul successivo libero. Se qualcosa non si può patchare (gli indirizzi sono occupati, per esempio), il motivo compare in rosso in basso. Premi **«Chiudi»** quando hai finito.

Per dimmer singoli, scegli **Generic › Generic Dimmer**: al posto della modalità, indichi quanti **Canali**.

> **Consiglio:** se non trovi la tua fixture, scarica la [Open Fixture Library](fixtures.md#open-fixture-library), importa il suo GDTF o creala nella pagina [Fixture](fixtures.md).

## L'elenco

Colonne: casella di selezione, **#** (numero), **NOME**, **INDIRIZZO** (universo . indirizzo), **CAN**, **FIXTURE** (produttore e modello), **MODALITÀ**, **INVERTI** (pan e tilt) e il tasto per identificare.

- **«Raggruppa per fixture»** le raggruppa per modello, con una casella per selezionare tutte quelle di ogni modello. Un clic sull'intestazione di un modello lo comprime (nasconde le sue fixture) o lo espande.
- **Doppio clic** su una fixture la modifica.

## Selezionare

Gli strumenti della barra lavorano sulla selezione:

- Il **clic** seleziona; **Ctrl+clic** aggiunge o toglie; **Maiusc+clic** seleziona tutte le fixture dall'ultima che hai cliccato fino a quella (in quel senso, che è l'ordine della selezione), e **Ctrl+Maiusc+clic** le aggiunge. **«Tutte»** e **«Nessuno»**. La casella di una riga aggiunge o toglie la sua fixture (come Ctrl+clic); con Maiusc, aggiunge tutte quelle dall'ultima che hai cliccato.
- **«Identifica»**: le fixture selezionate lampeggiano in bianco per trovarle sul palco. **«Smetti di identificare»** lo ferma (si ferma anche quando esci dalla pagina). Il tasto di ogni riga identifica solo quella fixture. L'elenco resta dov'era.
- **Inverti pan** e **inverti tilt** (le icone con le frecce): per le fixture che li hanno.
- **«Limiti»**, **«Modifica»**, **«Reindirizza»**, **«Rinumera»**, **«Crea gruppo»** ed **«Elimina»**: sono spiegati più avanti.

## Indirizzi e universi

- **Modificare una fixture**: selezionane una e premi **«Modifica»** (o doppio clic). Puoi cambiare **Nome**, **Modalità**, **Universo**, **Indirizzo** e **Numero**, e premere **«Applica»**.
- **Reindirizzarne diverse**: selezionale e premi **«Reindirizza»**. Scegli **Universo**, **Indirizzo iniziale** e **Spaziatura**: ricevono indirizzi consecutivi, nell'ordine dell'elenco.
- **Trascinare nella vista Canali**: scegli l'universo in alto e trascina una fixture su un altro indirizzo. L'ombra appare verde se entra e rossa se i canali non sono liberi. In alto si vede quanti dei 512 canali sono in uso. Il doppio clic su una fixture la modifica.

Gli universi si creano e si collegano alle interfacce in [Uscite](salidas.md).

## Numeri di fixture

Ogni fixture ha un **numero**, quello che usa la [riga di comando](programar.md#riga-di-comando) di Programma (`1 THRU 8 @ FULL`). Una nuova fixture riceve il numero successivo all'ultimo.

- Cambiane uno in **«Modifica»** › **Numero**.
- Per numerarne diverse di seguito, selezionale, premi **«Rinumera»** e scrivi il **Primo numero**: vengono numerate in ordine di indirizzo.

## Gruppi di fixture

I gruppi di fixture servono per selezionare con un clic in Patch e in Programma (`GROUP 2` nella riga di comando), e ognuno ha la sua striscia nel [mixer](directo.md#mixer). Si usano anche nello SHOW (ruota dei colori, griglia XY, palette, dimmer e velocità di movimento) e nelle mappature.

1. Seleziona le fixture.
2. Premi **«Crea gruppo»** e scrivi il suo nome (per esempio, `Wash frontale` o `Spot`).

Il pannello **«Gruppi di fixture»** a sinistra li mostra con il loro numero di fixture. Un clic su un gruppo seleziona le sue fixture; **Ctrl+clic** o **Maiusc+clic** le aggiunge. Clic destro:

- **«Rinomina»**.
- **«Sposta su»** e **«Sposta giù»**: lo spostano nell'elenco.
- **«Sostituisci con la selezione»**: il gruppo passa ad avere le fixture selezionate.
- **«Salva qui la selezione…»**: vedi [Selezioni](patch.md#selezioni).
- Schemi di selezione, nell'ordine del gruppo: **«Seleziona le dispari»**, **«Seleziona le pari»**, **«Seleziona in ordine inverso»**, **«Seleziona la prima metà»**, **«Seleziona la seconda metà»**, **«Seleziona dagli estremi al centro»** e **«Seleziona dal centro agli estremi»**.
- **«Elimina gruppo»**.

**L'ordine dei gruppi** lo scegli tu: trascina un gruppo dalla sua maniglia (a sinistra del nome) in un altro punto dell'elenco, oppure usa «Sposta su» e «Sposta giù». **«A–Z»**, nell'intestazione del pannello, li ordina alfabeticamente (i numeri come numeri: «Truss 2» prima di «Truss 10»). I gruppi nuovi vanno in fondo. L'ordine si salva con lo show ed è lo stesso in Programma e nei numeri di `GROUP` della riga di comando.

### Selezioni

Una **selezione** è una parte ordinata di un gruppo: per esempio, gli spot di un gruppo di sei presi dagli estremi verso il centro. L'ordine conta, perché i ventagli e gli FX lo seguono.

1. Seleziona le fixture nell'ordine che vuoi (o con uno schema del menu del gruppo).
2. Premi il **+** del gruppo (o «Salva qui la selezione…») e scrivi un nome.

Le selezioni compaiono sotto il loro gruppo. Un clic seleziona le sue fixture nel loro ordine, qui e in Programma. Clic destro: **«Rinomina»** ed **«Elimina selezione»**. La freccia a sinistra del gruppo nasconde o mostra le sue selezioni.

## Pianta 2D

La pianta mostra il palco visto dall'alto, con il **PUBBLICO** in basso. Serve per selezionare in Programma, per vedere ciò che esce e per gli effetti colore posizionati sulla pianta.

Nell'intestazione della pianta:

- **«Larghezza (m)»** e **«Profondità (m)»**: la dimensione del palco.
- **«Disponi automaticamente»**: distribuisce tutte le fixture.

Gli strumenti (lavorano sulla selezione e nel suo ordine):

- **«Dal vivo»**: mostra il colore, il dimmer, lo strobo e il gobo di ogni fixture, e ogni pixel delle barre e delle matrici.
- **«Lazo»**: seleziona disegnando attorno alle fixture invece che con un rettangolo.
- **«Seleziona»**: clic o rettangolo per selezionare.
- **«Sposta»**: trascina le fixture (si sposta tutta la selezione) o la vista.
- **«Griglia»**: «Libero», 10 cm, 25 cm, 50 cm o 1 m: dove finiscono le fixture quando le sposti.
- **«Aspetto»**: come appaiono le fixture selezionate: «Rotondo», «Quadrato» o «Rettangolo».
- **«Disponi»**: le dispone in «Linea», «Linea, da davanti a dietro», «Cerchio», «Griglia, per righe», «Griglia, a zigzag», «Griglia, per colonne» o «Griglia, a zigzag per colonne», con la **Spaziatura (cm)** e le **Colonne** accanto.
- **«Allinea»**: «Sinistra», «Centro», «Destra», «Fondo», «Mezzo», «Davanti», «Distribuisci in larghezza» e «Distribuisci in profondità».
- **15°** (clic destro: nell'altro senso) e **90°**: ruotano la selezione.
- **«Altezza e orientamento»**: dove sono appese le teste mobili selezionate, per [Follow](patch.md#altezza-e-orientamento-follow).

Le teste mobili hanno una piccola freccia sul bordo: la direzione verso cui guardano. I nomi delle fixture molto vicine si nascondono per non sovrapporsi: aumenta lo zoom per leggerli.

La **rotella** fa lo zoom, il **tasto destro** o quello centrale trascinano la vista e un doppio clic con essi la centra. Anche i tasti nell'angolo fanno lo zoom.

> **Consiglio:** posiziona le fixture nell'ordine in cui vuoi che scorrano gli effetti: selezionale da sinistra a destra e usa «Disponi › Linea».

### Altezza e orientamento (Follow)

Perché [Follow](programar.md#follow-puntare-a-un-punto) punti bene tutte le teste mobili su un punto, ognuna deve sapere dove si trova. Posiziona le teste sulla pianta dove sono davvero, selezionale e premi **«Altezza e orientamento»**. Ciò che cambi si applica subito a tutte quelle selezionate e si salva con lo show:

| Campo | Cos'è |
|---|---|
| **Altezza** | A quanti metri dal pavimento del palco si trova l'asse della testa (se non impostata: 4 m appesa, 0 m in piedi). |
| **Montaggio** | **«Appesa»** (a una barra o a un truss, a testa in giù) o **«In piedi»** (sul pavimento). |
| **Orientamento** | La direzione del fascio con il pan al centro e il tilt inclinato in avanti. 0° è verso il pubblico e ruota in senso orario; è la stessa rotazione dei tasti 15° e 90°, e la freccia sulla pianta la mostra. |
| **Escursione del pan** / **del tilt** | I gradi che percorre ogni asse. «Quella della definizione» usa quelli della definizione della fixture; se non li riporta si usano 540° e 270°, e viene segnalato in arancione. |
| **Pan invertito** / **Tilt invertito** | Se con Follow una testa ruota o si inclina dal lato opposto. Riguarda solo Follow; le inversioni del Patch sono in [Limiti](patch.md#limiti). |

«(diverse)» indica che le teste selezionate non hanno lo stesso valore; quando lo cambi, tutte prendono quello nuovo.

## Limiti

Seleziona le fixture e premi **«Limiti»**:

- **Dimmer**, **Pan** e **Tilt**: l'intervallo consentito, da 0 a 100 %. Per esempio, un dimmer che non superi l'80 % o un tilt che non punti sul pubblico.
- **«Inverti pan»**, **«Inverti tilt»** e **«Scambia pan e tilt»** (per teste appese al contrario o ruotate).
- **«Ripristina»**: nessun limite, nessun asse invertito o scambiato.

Con una sola fixture, l'elenco **Canali** permette di decidere per ogni canale:

- **Dissolvenze**: se segue i tempi di dissolvenza. Se è spento, salta direttamente al suo valore (utile per ruote colori e gobo).
- **Dimmer**: se lo scalano il grand master, i master di gruppo e i dimmer di scena. Il grand master, inoltre, scala solo la luce (vedi [Master](directo.md#master)).

I limiti agiscono sull'uscita: scene, effetti e modifiche dal vivo conservano i loro valori, ma ciò che esce li rispetta. Con più fixture si vedono i valori della prima che ha ciascun controllo, e ciò che cambi si applica a tutte quelle che lo hanno.

## Matrici e strisce LED

**«Matrice LED»** patcha in un colpo solo una matrice o una striscia di pixel:

1. **Nome**, **Larghezza (pixel)** e **Altezza (pixel)** (altezza 1 per una striscia).
2. **Colori**: l'ordine di ogni pixel (RGB, GRB, BGR, BRG, GBR, RBG o RGBW).
3. **Cablaggio**: «Righe», «Righe, a zigzag», «Colonne» o «Colonne, a zigzag».
4. **Pixel 1**: l'angolo da cui inizia («In alto a sinistra», ecc.).
5. **Universo** e **Indirizzo**.
6. Controlla il riepilogo e l'anteprima dell'**ordine dei pixel**, e premi **«Patcha»**.

Si crea una fixture per ogni riga di pixel, un gruppo per gli effetti colore e la sua posizione sulla pianta. Se una riga ha bisogno di più di un universo, accorciala o cablala nell'altro senso.

## Importare da un altro show

**«Importa»** porta in questo show una parte di un altro (di DMX Total Control o di QLC+):

- **«Patch di un altro show…»**: le sue fixture. Vanno agli stessi indirizzi se sono liberi; altrimenti, ai successivi.
- **«Scene di un altro show…»**: le sue scene, assegnate alle fixture di questo show che coincidono (per indirizzo e modello, per nome o per modello). Entrano in gruppi con lo stesso nome.
- **«Patch e scene di un altro show…»**: entrambe le cose.

## Eliminare fixture

Selezionale e premi **«Elimina»**. Viene chiesto prima: i loro valori vengono rimossi da tutte le scene.

# Primi passi

![La guida ai primi passi nella pagina Patch](../img/primeros-pasos.png)

## La guida

La prima volta che apri il programma compare la **guida ai primi passi**: una scheda in basso a destra con cinque passi (le tue fixture, un look, una scena, Live e i playback). **«Avanti»** e **«Indietro»** cambiano passo e ti portano alla pagina di ciascuno; le pagine si possono usare mentre la scheda è aperta. La chiudi con la **✕** e la riapri in Opzioni › Aiuto › **«Avvia la guida»**.

Nel primo passo, **«Apri lo show di prova»** apre l'esempio «Primeros pasos» (se hai modifiche non salvate, prima chiede se salvarle). Ha già quattro PAR LED RGBW nel gruppo di fixture `PARs`, due teste mobili in `Móviles` e il gruppo «Mis escenas» vuoto in Live, pronto per le tue scene.

## La sessione completa

In questa prima sessione farai il percorso completo: collegare l'uscita, patchare alcune fixture, preparare un look nel programmatore, registrarlo come scena in un gruppo, avviarlo in Live, registrare una cuelist su un playback e salvare lo show. Ti servirà una ventina di minuti.

Useremo fixture della libreria generica: quattro PAR LED RGBW e due teste mobili. Se hai altri apparecchi, cerca il loro produttore e modello al posto di quelli generici.

> **Consiglio:** puoi seguire questa guida senza un'interfaccia DMX collegata. Ciò che esce si vede in Uscite › «Livelli DMX» e nella pianta 2D di Programma.

> **Consiglio:** con lo show di prova aperto hai già le fixture e il gruppo: salta al passo 4 e usa il gruppo di fixture `Móviles` dove qui c'è scritto `Teste`.

## 1. Collega l'uscita

1. Collega la tua interfaccia DMX USB al computer (o assicurati che il computer sia sulla stessa rete del tuo nodo Art-Net o sACN).
2. Apri **Uscite** (F7).
3. Nella riga dell'universo 1, apri il menu a tendina che indica «Non collegato» e scegli la tua interfaccia.
4. Se non compare, premi **«Cerca di nuovo»**.

L'universo collegato viene segnato in verde. Troverai più dettagli in [Uscite](salidas.md).

## 2. Patcha alcune fixture

1. Apri **Patch** (F5) e premi **«Aggiungi fixture»**.
2. In **Produttore**, scegli **Generic** (puoi scrivere in «Cerca produttore…»).
3. In **Modello**, scegli **LED PAR RGBW**.
4. Nel pannello **Patch** a destra:
   - **Modalità**: «5ch DRGBW».
   - **Quantità**: 4.
   - **Universo**: 1. L'**Indirizzo** si imposta da solo sul primo libero.
   - Lascia spuntato **«Creane un gruppo»** e scrivi in **Gruppo** il nome `PARs`.
5. Premi **«Patcha 4 fixture»**. In basso vedrai quali indirizzi usano.
6. Senza chiudere la finestra, scegli ora **Moving Head Spot**, modalità «11ch Standard», quantità 2, gruppo `Teste`, e premi **«Patcha 2 fixture»**.
7. Premi **«Chiudi»**.

> **Consiglio:** imposta su ogni apparecchio lo stesso indirizzo DMX che mostra l'elenco del Patch. Per trovare un apparecchio sul palco, selezionalo e premi **«Identifica»**: lampeggia in bianco.

## 3. Crea un gruppo in Live

Le scene si avviano in Live, organizzate in gruppi. Creane uno prima di registrare:

1. Apri **Live** (F1) e premi **«Nuovo gruppo»**.
2. Fai clic sul nome della colonna («Gruppo 1»), cancellalo, scrivi `Mis escenas` e premi Invio.
3. Nel menu a tendina del tempo di dissolvenza incrociata della colonna (indica «Per scena»), scegli **2 s**.

Il gruppo è **«Esclusivo»**: è attiva una sola scena alla volta e, quando ne premi un'altra, fa una dissolvenza incrociata.

## 4. Prepara un look nel programmatore

1. Apri **Programma** (F3).
2. Nel pannello delle fixture, fai clic sull'etichetta del gruppo **PARs**: si selezionano tutte e quattro.
3. Nel pannello **Rapido**, premi **100%** accanto al fader **Dimmer**.
4. Fai clic sul campione di colore **rosso**.

I PAR si accendono in rosso appena lo tocchi: ciò che metti nel programmatore esce subito, sopra tutto.

> **Consiglio:** lo stesso con la riga di comando: scrivi `GROUP 1 @ FULL` e premi Invio. `GROUP 1` è il primo gruppo di fixture. Vedi [la riga di comando](programar.md#riga-di-comando).

## 5. Registra la scena nel gruppo

1. Premi **«Registra…»** (in alto a destra).
2. In **Registra in**, lascia **«Nuova scena»**.
3. In **Nome**, scrivi `Rosso`.
4. In **Aggiungi al gruppo**, scegli **Mis escenas**.
5. Premi **«Registra»**.

Registra un secondo look: con i PAR selezionati e il dimmer al 100 %, fai clic sul campione **blu**, premi **«Registra…»**, nome `Blu`, gruppo **Mis escenas** e **«Registra»**.

Prima di andare in Live, premi **«Azzera tutto»** per svuotare il programmatore. Finché il programmatore ha dei valori, questi prevalgono sulle scene e non vedresti ciò che avvii.

> **Consiglio:** nella finestra di registrazione, spunta **«Svuota il programmatore dopo la registrazione»** e il programmatore si svuoterà da solo ogni volta che registri. L'opzione viene ricordata.

## 6. Avvia le scene in Live

1. Apri **Live** (F1).
2. Premi **Rosso**: il riquadro si illumina con il colore del gruppo e i PAR entrano in rosso.
3. Premi **Blu**: il gruppo fa una dissolvenza incrociata di 2 secondi dal rosso al blu.
4. Muovi il **Master** della colonna per abbassare l'intensità di tutto il gruppo.
5. Per rilasciarla, premi il quadrato nell'intestazione della colonna («Rilascia questo gruppo»), o passa a **Stop** nella barra superiore, premi **Blu** e torna a **GO**.

Altro in [Live](directo.md).

## 7. Registra una cuelist su un playback

Le cuelist avanzano cue dopo cue con GO. Il modo più rapido per crearne una è registrare direttamente su un playback.

1. In **Programma**, fai clic sull'etichetta **Teste**.
2. Premi **«Beam on»** (dimmer al massimo e shutter aperto) e **«Centra»** (pan e tilt al centro).
3. Apri **Playback** (F10) e premi **«Registra»**: il tasto lampeggia in rosso.
4. Fai clic sul playback **1**, che è vuoto: si crea una nuova cuelist con la cue 1.
5. Torna in **Programma**, apri **Funzioni**, scheda **Posizione**, e trascina la croce della griglia XY in un altro punto.
6. In **Playback**, premi **«Registra»** e fai di nuovo clic sul playback 1: si registra la cue 2, solo con ciò che è cambiato (la posizione).
7. Torna in **Programma** e premi **«Azzera tutto»**.
8. In **Playback**, fai clic sul nome del playback 1 per selezionarlo e premi **Spazio** (o il grande tasto **GO**): parte la cue 1. Premi di nuovo: passa alla cue 2.

Il fader del playback è la sua intensità: se lo alzi da zero, la cuelist parte. Altro in [Playback](playbacks.md).

## 8. Salva lo show

1. Premi **Ctrl+S** (o il dischetto della barra superiore).
2. Essendo la prima volta, scegli cartella e nome: si salva come file `.qxw`.

Da qui in poi, Ctrl+S salva nello stesso file e conserva come `.bak` la versione che c'era prima del primo salvataggio di ogni sessione. Con il salvataggio automatico attivo (vedi [Opzioni](opciones.md#salvataggio-automatico)), il programma salva ogni pochi minuti una copia di recupero a parte, senza toccare il tuo file: se si chiude in modo inatteso, ti propone di recuperare le modifiche.

## E adesso…

- Costruisci la tua schermata operativa nello [Show](show.md).
- Impara a usare [palette](programar.md#palette) e [FX](programar.md#fx) nel programmatore.
- Modifica le cue, i loro tempi e il loro tracking in [Scene](escenas.md#cuelist).
- Collega un controller MIDI in [Mappature](mapeos.md).

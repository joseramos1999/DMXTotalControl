# Patch

![La pagina Patch](../img/patch.png)

**Patch** (F5) contiene le fixture del tuo impianto: cosa sono, in quale universo e indirizzo si trovano, il loro numero, i gruppi di fixture e dove si trovano sul palco.

Nell'intestazione scegli la vista e trovi gli strumenti per aggiungere:

- **«Elenco»**: tutte le fixture per indirizzo.
- **«Canali»**: i 512 canali di ogni universo, con le fixture sopra.
- **«Pianta 2D»**: il palco visto dall'alto.
- **«Importa»**, **«Matrice LED»** e **«Aggiungi fixture»**.
- Con fixture parcheggiate, **«N parcheggiate · Togli tutto dal parcheggio»**: quante sono e il tasto per toglierle tutte dal parcheggio (vedi [Parcheggiare i canali](patch.md#parcheggiare-i-canali)).

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
4. In basso si vede quali indirizzi useranno («Usa gli indirizzi da 1 a 20»). Se non entrano in ciò che resta dell'universo, passano al successivo. Se la modalità ha altri *DMX break* (una fixture GDTF con due ingressi DMX), un avviso ti chiede di patchare anche la sua modalità «(DMX break 2)», al suo indirizzo.
5. Premi **«Patcha fixture»** o **«Patcha N fixture»**.

La finestra resta aperta per continuare ad aggiungere, con l'indirizzo già sul successivo libero. Se qualcosa non si può patchare (gli indirizzi sono occupati, per esempio), il motivo compare in rosso in basso. Premi **«Chiudi»** quando hai finito.

Per dimmer singoli, scegli **Generic › Generic Dimmer**: al posto della modalità, indichi quanti **Canali**.

**«Cerca su GDTF-Share…»**, in basso, apre [GDTF-Share](fixtures.md#gdtf-share): scarica la fixture e premi **«Patchala»** per averla scelta qui.

> **Consiglio:** se non trovi la tua fixture, cercala su GDTF-Share, scarica la [Open Fixture Library](fixtures.md#open-fixture-library), importa il suo GDTF o creala nella pagina [Fixture](fixtures.md).

## L'elenco

Colonne: casella di selezione, **#** (numero), **NOME**, **INDIRIZZO** (universo . indirizzo), **CAN**, **FIXTURE** (produttore e modello), **MODALITÀ**, **INVERTI** (pan e tilt) e il tasto per identificare. Dopo il nome, uno **scudo** indica un [effetto speciale protetto](patch.md#effetti-speciali-protetti) e un **lucchetto** una fixture con canali [parcheggiati](patch.md#parcheggiare-i-canali).

- **«Raggruppa per fixture»** le raggruppa per modello, con una casella per selezionare tutte quelle di ogni modello. Un clic sull'intestazione di un modello lo comprime (nasconde le sue fixture) o lo espande.
- **Doppio clic** su una fixture la modifica.

## Selezionare

Gli strumenti della barra lavorano sulla selezione:

- Il **clic** seleziona; **Ctrl+clic** aggiunge o toglie; **Maiusc+clic** seleziona tutte le fixture dall'ultima che hai cliccato fino a quella (in quel senso, che è l'ordine della selezione), e **Ctrl+Maiusc+clic** le aggiunge. **«Tutte»** e **«Nessuno»**. La casella di una riga aggiunge o toglie la sua fixture (come Ctrl+clic); con Maiusc, aggiunge tutte quelle dall'ultima che hai cliccato.
- **«Identifica»**: le fixture selezionate lampeggiano in bianco per trovarle sul palco. **«Smetti di identificare»** lo ferma (si ferma anche quando esci dalla pagina). Il tasto di ogni riga identifica solo quella fixture. L'elenco resta dov'era. Gli effetti speciali protetti non si identificano mai (far lampeggiare un CO2 lo attiverebbe): un avviso dice quanti sono rimasti fuori.
- **Inverti pan** e **inverti tilt** (le icone con le frecce): per le fixture che li hanno.
- **«Limiti»**, il tasto della **fiamma** (**«Effetto speciale»**), **«Modifica»**, **«Reindirizza»**, **«Rinumera»**, **«Crea gruppo»** ed **«Elimina»**: sono spiegati più avanti.

Un **clic destro** su una riga dell'elenco, o su una fixture della pianta 2D senza muovere il mouse, apre il menu della fixture; se la fixture non era selezionata, prima la seleziona. In alto dice su cosa agisce: la fixture (il suo numero, il nome, l'indirizzo e il tipo) o quante sono selezionate e quali. Sotto, con le stesse icone della barra: **«Identifica»** (mentre ci sono fixture che lampeggiano, **«Smetti di identificare»**, con quante sono); in **Impostazioni**, **«Limiti…»**, **«Effetto speciale…»** (con lo scudo dell'elenco se ci sono effetti protetti), **«Modifica…»** (con una sola fixture; con più di una dice «Una alla volta») e l'interruttore **«Proteggi dal grand master»**; in **Parcheggio**, che a destra dice quante della selezione sono parcheggiate, **«Parcheggia com'è ora»**, **«Parcheggia spento»** e, con qualche fixture parcheggiata nella selezione, l'interruttore **«Anche in blackout»** e **«Togli dal parcheggio»**; e in **Tutto lo show**, con fixture parcheggiate, **«Togli tutto dal parcheggio»**, che toglie dal parcheggio tutte quelle dello show, non solo le selezionate. Un interruttore a metà, con quante lo hanno (3/4), vuol dire che solo alcune delle selezionate lo hanno: premendolo, si attiva in tutte. Con il mouse fermo sulla maggior parte delle voci, una nota accanto al menu dice cosa fanno.

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

Il pannello **«Gruppi di fixture»** a sinistra li mostra con il loro numero di fixture. Un clic su un gruppo seleziona le sue fixture (nella vista «Pianta 2D», lo isola: vedi [Isolare un gruppo nella pianta](patch.md#isolare-un-gruppo-nella-pianta)); **Ctrl+clic** o **Maiusc+clic** le aggiunge. Clic destro:

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

- **«Larghezza (m)»** e **«Profondità (m)»**: la dimensione del palco, fino a 200 m. Lo allargano o lo restringono a destra e davanti; una fixture rimasta fuori da un palco più piccolo resta dov’è.
- **«Disponi automaticamente»**: distribuisce tutte le fixture.

Fuori dal palco, al centro di ogni lato, ci sono un **«−»** e un **«+»**: tolgono o aggiungono 1 m su quel lato (a sinistra, a destra, sul fondo o davanti, dal lato del pubblico); tienili premuti per più metri. Le fixture restano dove sono sul palco: a sinistra e sul fondo si spostano con quel lato, come il punto a cui punta [Follow](programar.md#follow-puntare-a-un-punto), e la nuova striscia compare su quel lato. Finché il mouse è sopra, la vista della pianta non cambia e i pulsanti non si spostano, così puoi fare clic più volte di seguito; quando lo allontani, la pianta si adatta di nuovo (se avevi fatto zoom o spostato la vista, resta com'è). Il **«−»** di un lato funziona solo se nessuna fixture è a 1 m o meno da esso: spostala prima (passa il mouse sul pulsante per sapere perché).

Gli strumenti (lavorano sulla selezione e nel suo ordine):

- **«Dal vivo»**: mostra il colore, il dimmer, lo strobo e il gobo di ogni fixture, e ogni pixel delle barre e delle matrici.
- **«Lazo»**: seleziona disegnando attorno alle fixture invece che con un rettangolo.
- **«Seleziona»**: clic o rettangolo per selezionare.
- **«Sposta»**: trascina le fixture (si sposta tutta la selezione) o la vista.
- **«Griglia»**: «Libero», 10 cm, 25 cm, 50 cm o 1 m: dove finiscono le fixture quando le sposti.
- **«Aspetto»**: come appaiono le fixture selezionate: «Rotondo», «Quadrato» o «Rettangolo», e la loro **«Icona»** (vedi [Icone, colori e legenda](patch.md#icone-colori-e-legenda)).
- **«Disponi»**: le dispone in «Linea», «Linea, da davanti a dietro», «Cerchio», «Griglia, per righe», «Griglia, a zigzag», «Griglia, per colonne» o «Griglia, a zigzag per colonne», con la **Spaziatura (cm)** e le **Colonne** accanto.
- **«Allinea»**: «Sinistra», «Centro», «Destra», «Fondo», «Mezzo», «Davanti», «Distribuisci in larghezza» e «Distribuisci in profondità».
- **15°** (clic destro: nell'altro senso) e **90°**: ruotano la selezione.
- **«Altezza e orientamento»**: dove sono appese le teste mobili selezionate, per [Follow](patch.md#altezza-e-orientamento-follow).

Le teste mobili hanno una piccola freccia sul bordo (una tacca, quando la pianta le mostra piccole): la direzione verso cui guardano. I nomi delle fixture molto vicine si nascondono per non sovrapporsi: aumenta lo zoom per leggerli. Uno **scudo** in alto a sinistra indica un effetto speciale protetto (tenue; rosso con gli effetti armati; con un anello che pulsa mentre si attiva) e un **lucchetto** in alto a destra una fixture parcheggiata.

La **rotella** fa lo zoom, il **tasto destro** o quello centrale trascinano la vista e un doppio clic con essi la centra. Anche i tasti nell'angolo fanno lo zoom. Un clic destro senza muovere il mouse apre il [menu della fixture](patch.md#selezionare).

> **Consiglio:** posiziona le fixture nell'ordine in cui vuoi che scorrano gli effetti: selezionale da sinistra a destra e usa «Disponi › Linea».

### Icone, colori e legenda

Ogni fixture mostra sulla pianta l'icona di ciò che è: testa mobile, scanner, wash o PAR, barra LED, strobo, blinder, dimmer o tradizionale, laser, fumo, hazer, getto di CO2, fiamma, scintille fredde, coriandoli, bolle, neve, ventilatore, effetto o altro. Lo dice il suo tipo di fixture, a meno che il nome del suo modello non dica di più: un «Effect» chiamato «CO2 Jet» è un getto di CO2 e un dimmer chiamato «Blinder» è un blinder, ma una testa mobile chiamata «Beam» o «Wash» resta una testa mobile. Un dimmer generico, che può stare al posto di qualsiasi fixture, prende l'icona del suo stesso nome («CO2 1», «FLAME 3», «BLINDER1»). Le barre LED e le matrici mostrano le loro celle; le altre fixture con più celle (un blinder a più lampade, uno strobo a più segmenti) hanno l'icona accanto alle loro celle.

Per sceglierla a mano, seleziona le fixture e usa **«Aspetto» › «Icona»**; **«Automatica»** torna alla sua. L'icona scelta si salva con lo show.

Ogni tipo di fixture dello show ha anche il suo colore, sull'icona e sul bordo, lo stesso ogni volta che apri lo show: due tipi con la stessa icona si distinguono per il colore. Un'icona scelta a mano su una fixture della libreria non ne cambia il colore. Dal vivo, le fixture accese mostrano la loro luce e quelle spente, la loro icona.

La **legenda**, in alto a sinistra della pianta, elenca i tipi di fixture con la loro icona, il loro colore e quante sono (i dimmer generici, per ciò che sostituiscono). Un clic su un tipo seleziona tutte le sue fixture, anche sulla pianta di Programma; **Ctrl+clic** o **Maiusc+clic** le aggiunge alla selezione. Un clic sul suo titolo la chiude o la apre, e viene ricordato. Il palco si colloca accanto o sotto la legenda, perché non copra nessuna fixture.

### Isolare un gruppo nella pianta

Nella vista «Pianta 2D», un clic su un gruppo del pannello «Gruppi di fixture» lo **isola**: le altre fixture restano bloccate e sbiadite nella pianta, e niente le seleziona né le sposta: né un clic, né il rettangolo, né il lazo, né la legenda, né «Sposta» (un clic su una di esse non fa nulla, e il rettangolo può iniziare sopra di essa). Così è facile selezionare parte di un gruppo in una pianta piena di fixture e farne gruppi più piccoli. Della selezione restano solo le fixture del gruppo che erano già selezionate. Il gruppo isolato ha un bordo spesso e un occhio, e accanto alla legenda della pianta un'etichetta dice quale gruppo vedi e quante fixture ha.

- **«Tutte»** seleziona tutte le fixture del gruppo, come **Ctrl+clic** sul gruppo. **Ctrl+clic** su un altro gruppo aggiunge solo le sue fixture che sono anche in quello isolato.
- Nella legenda, un clic su un tipo seleziona solo le sue fixture del gruppo; i tipi che non ne hanno nessuna appaiono sbiaditi.
- Un clic su un altro gruppo isola quello al suo posto; uno schema di selezione del suo menu isola prima quel gruppo. Un clic su una selezione seleziona sempre tutte le sue fixture: se alcune sono bloccate, prima isola il gruppo della selezione o, se contiene fixture di altri gruppi, termina l'isolamento.
- Dopo **«Crea gruppo»**, il gruppo resta isolato: puoi continuare a dividerlo in altri gruppi.
- Per terminare: un altro clic sul gruppo, la **×** dell'etichetta o **Esc**. Termina anche passando a «Elenco» o «Canali», aprendo un altro show o eliminando il gruppo. Non si salva con lo show.

Nelle viste «Elenco» e «Canali», un clic su un gruppo continua a selezionare le sue fixture, come sempre.

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
- **«Inverti pan»**, **«Inverti tilt»** e **«Scambia pan e tilt»** (per teste appese al contrario o ruotate). È una correzione fisica: fa muovere quelle teste come le altre, e lo specchio di un FX non ne tiene conto (vedi [Lo specchio](programar.md#lo-specchio)). Con più fixture di cui solo alcune sono invertite, appare spento con «(solo alcune)», come il tasto della barra, e attivarlo le inverte tutte.
- **«Ripristina»**: nessun limite, nessun asse invertito o scambiato.
- **«Proteggi dal grand master»**: il grand master non abbassa nessuno dei suoi canali, che agisca solo sull'intensità o su tutti: la luce di sala, un media server, un hazer. I master di gruppo, i dimmer di scena, il Blackout e la [dissolvenza al nero](directo.md#dissolvenza-al-nero) la abbassano ancora. Con più fixture di cui solo alcune sono protette, appare spento con «(solo alcune)», e attivarlo le protegge tutte. È anche nel [menu della fixture](patch.md#selezionare), in **Impostazioni**, come interruttore: acceso quando tutte le selezionate sono protette e a metà, con quante (3/4), quando lo sono solo alcune; premendolo a metà, si proteggono tutte. Nell'elenco, un'**ancora** dopo il nome («Protetta dal grand master: non si abbassa con esso»); nella pianta 2D, del Patch e di Programma, un'ancora in basso a sinistra (su una barra, alla sua destra). «Ripristina» non toglie la protezione. Si salva con lo show, in ogni fixture, e [importare il patch da un altro show](patch.md#importare-da-un-altro-show) la porta. QLC+ non la legge: lì il grand master abbassa la fixture.

Con una sola fixture, l'elenco **Canali** permette di decidere per ogni canale:

- **Dissolvenze**: se segue i tempi di dissolvenza. Se è spento, salta direttamente al suo valore (utile per ruote colori e gobo).
- **Dimmer**: se lo scalano il grand master, i master di gruppo e i dimmer di scena. Il grand master, inoltre, scala solo la luce (vedi [Master](directo.md#master)). Se la fixture è protetta dal grand master, la scalano solo i master di gruppo e i dimmer di scena.

I limiti agiscono sull'uscita: scene, effetti e modifiche dal vivo conservano i loro valori, ma ciò che esce li rispetta. Su un pan o un tilt a 16 bit, il limite va sui due canali insieme, quindi un movimento lento resta fluido. Con più fixture si vedono i valori della prima che ha ciascun controllo, e ciò che cambi si applica a tutte quelle che lo hanno.

## Effetti speciali protetti

Un **effetto speciale protetto** si attiva solo con gli effetti armati, da scene, cuelist e chaser, e ogni attivazione dura al massimo il suo tempo massimo. Come si armano e cosa li attiva è spiegato in [Live](directo.md#effetti-speciali-protetti).

**Quali lo sono.** Le macchine di **CO2**, **fiamme**, **scintille fredde** e **coriandoli** si proteggono da sole; quelle di **bolle** e **neve**, no (proteggile a mano se vuoi). La classe viene dal tipo di fixture, dalle parole della sua definizione, dal nome di un dimmer generico («CO2 1», «FLAME 3», «FIAMMA 2», «PIRO 1») o dall'icona scelta nella pianta. Le regole automatiche proteggono soltanto, non tolgono mai la protezione: cambiare il nome o l'icona non toglie mai la protezione a una fixture; una fixture che diventa di una di queste classi viene protetta, e un avviso lo dice («CO2 4 ora è un effetto speciale protetto»); un CO2 a cui hai tolto la protezione a mano resta senza anche se lo chiami «CO2 9».

> **Limiti:** «C02» scritto con uno zero non si riconosce, e «FOCO2» non è un CO2. Una macchina non riconosciuta non è protetta: controlla lo scudo nell'elenco e proteggila a mano.

**«Effetto speciale»** (il tasto della fiamma, accanto a «Limiti», o «Effetto speciale…» nel menu della fixture) apre le sue impostazioni per la selezione:

- **«Effetto speciale protetto»**: toglierlo chiede prima («Togliere la protezione?»). Senza di essa, la fixture esce come qualsiasi altro canale: senza armare, senza tempo massimo e anche dal programmatore, da Identifica, dal mixer e dagli FX.
- **«Tempo massimo di attivazione»**: da 0,1 a 30 s (per impostazione predefinita: CO2 3 s, fiamme 2 s, scintille 5 s, coriandoli 2 s, gli altri 3 s).
- **«Attesa minima tra due attivazioni»**: da 0 a 60 s (1 s per impostazione predefinita), dal rilascio.
- **«Canale di sicurezza»**: **«Nessuno»** o il canale della fixture di cui la macchina ha bisogno per attivarsi, con i suoi **«Valori da / a»** (da 0 a 255; «da» non può superare «a»). Quel canale esce in quei valori solo mentre l'effetto è armato e si attiva («da», se la scena non gli dà un valore); altrimenti 0. Si sceglie solo quando le fixture selezionate hanno gli stessi canali, e una fixture di un solo canale non può averne uno («… ha bisogno di un altro canale per attivarsi»). Se la fixture cambia modo o definizione e al suo posto resta un altro canale, il canale di sicurezza viene tolto, e un avviso lo dice («Il canale di sicurezza di … è stato tolto»).
- **«Secondo la sua classe (…)»**: torna alla protezione e ai tempi della sua classe, senza canale di sicurezza.

**Quali canali si attivano.** Tutti tranne il pan, il tilt e quello di sicurezza, qualunque sia il loro tipo (intensità, effetto, altezza…): a loro si applica il tempo massimo. Il pan e il tilt escono solo con gli effetti armati e dalle scene, senza limite di tempo, e a 0 disarmati: una scena che si limita a puntare la macchina non la attiva. Una macchina che ha solo pan e tilt si attiva su tutti i suoi canali tranne quello di sicurezza. Una fixture a cavallo di due universi (di uno show di QLC+) non si attiva mai: la finestra lo avvisa; spostala in un solo universo.

Nell'elenco, uno **scudo** dopo il nome; nella pianta, uno scudo in alto a sinistra: tenue normalmente, rosso con gli effetti armati e con un anello che pulsa mentre si attiva. **Identifica** non li accende mai. [Livelli DMX](salidas.md#livelli-dmx) e la pianta dal vivo mostrano ciò che esce davvero: un CO2 disarmato si vede a 0 anche se la sua scena suona.

La protezione si salva con lo show, nella sua sezione di DMX Total Control, e [importare il patch di un altro show](patch.md#importare-da-un-altro-show) la porta con sé. Quando apri uno show più vecchio o di QLC+, un avviso dice una volta quali fixture ora sono effetti protetti per la loro classe («2 fixture di questo show ora sono effetti speciali protetti: …»); anche una macchina del fumo il cui modello dice «CO2» o un dimmer generico chiamato «Fuego…» o «Fire…». Per cambiarlo, usa «Effetto speciale». QLC+ non legge questa protezione: aperto in QLC+, lo show attiva queste fixture da qualsiasi parte, senza armare e senza tempo massimo. Nemmeno le versioni precedenti di DMX Total Control la leggono: attivano queste fixture senza armare e, quando salvano lo show, tolgono la protezione messa a mano, i tempi e il canale di sicurezza. Usa questa versione su tutti i computer dello show.

## Parcheggiare i canali

Un canale **parcheggiato** esce sempre con un valore fisso, sopra le scene, le cuelist, gli FX, il programmatore, il mixer (Full, Flash…), il grand master e Ferma tutto: l'hazer al 30 % per tutta la notte, una testa guasta spenta e ferma, la luce di sala. Esc e Blackout non lo tolgono mai dal parcheggio.

Nel [menu della fixture](patch.md#selezionare) (clic destro nell'elenco o nella pianta), in **Parcheggio**, che a destra dice quante della selezione sono parcheggiate:

- **«Parcheggia com'è ora»**: tutti i suoi canali a ciò che esce ora, quindi il grand master, il mixer e i limiti di quel momento restano dentro. Non si può durante l'arresto dello show né durante una dissolvenza al nero (la loro luce resterebbe parcheggiata per sempre).
- **«Parcheggia spento»**: i canali di intensità (dimmer e colori) a 0 e gli altri come sono ora; una fixture senza canali di intensità (una macchina del fumo o un hazer della libreria), tutto a 0. Durante l'arresto dello show o una dissolvenza al nero, gli altri canali si parcheggiano a ciò che lo show invia sotto.
- **«Anche in blackout»** (un interruttore, con qualche fixture parcheggiata nella selezione): acceso, con il Blackout le sue intensità parcheggiate mantengono il loro valore. Spento, si spengono con il Blackout; il pan, il tilt, il gobo e gli altri canali che non sono luce restano sempre parcheggiati.
- **«Togli dal parcheggio»** (con qualche fixture parcheggiata nella selezione) e, in **Tutto lo show**, **«Togli tutto dal parcheggio»**: tutte le parcheggiate dello show, non solo le selezionate (come il pulsante dell'intestazione del Patch).

In [Programma](programar.md#parcheggiare), **«Parcheggia»** parcheggia i valori che il programmatore ha sulle fixture selezionate.

Con fixture parcheggiate, l'intestazione del Patch mostra **«N parcheggiate · Togli tutto dal parcheggio»** (in una finestra stretta, il lucchetto, il numero e il tasto con la sola icona). Nell'elenco, un **lucchetto** dopo il nome («Parcheggiata: N canali con un valore fisso in uscita»); nella pianta 2D, del Patch e di Programma, un lucchetto in alto a destra. In [Livelli DMX](salidas.md#livelli-dmx), un lucchetto su ogni canale parcheggiato.

- Un effetto speciale protetto si può parcheggiare solo **spento**: «Parcheggia com'è ora» o «Parcheggia» di Programma lo parcheggiano a 0 e lo dicono («Un effetto speciale protetto si può parcheggiare solo spento»). Parcheggiato, non si attiva mai, nemmeno armato; un'attivazione interrotta parcheggiandolo compare nel registro come «parcheggiato». Una fixture parcheggiata accesa che diventa protetta viene parcheggiata spenta; se poi non è più protetta, resta parcheggiata a 0 finché non la togli dal parcheggio. Tolta dal parcheggio, si attiva solo dopo aver rilasciato la sua scena e averla premuta di nuovo.
- I canali parcheggiati seguono la fixture se la reindirizzi. Cambiare la modalità di una fixture toglie i suoi canali parcheggiati («I canali parcheggiati di Spot 3 sono stati rimossi: è cambiata la modalità»), tranne in un effetto protetto, che resta parcheggiato spento. Eliminare una fixture toglie i suoi. Un pan parcheggiato di una fixture con pan e tilt scambiati esce dal canale del tilt, come il pan.
- L'[arresto dello show](directo.md#arresto-dello-show) vince sui canali parcheggiati: una fixture parcheggiata spenta si illumina anch'essa, e un hazer parcheggiato resta parcheggiato. Una [dissolvenza al nero](directo.md#dissolvenza-al-nero) abbassa i parcheggiati senza «Anche in blackout».

I canali parcheggiati si salvano con lo show, nella sua sezione di DMX Total Control (importare il patch di un altro show non li porta). Quando si apre uno show con canali parcheggiati, si applicano subito, e un avviso dice quali («2 fixture parcheggiate: HAZER, Spot 3 (Patch)»). Aprire un altro show o uno nuovo li toglie. Alla chiusura del programma vengono rilasciati (il palco va a riposo), ma restano nello show. QLC+ non parcheggia i canali: lì suonano come dicono le loro scene, e la copia per QLC+ lo avvisa («2 fixture parcheggiate: QLC+ non parcheggia i canali»). Le versioni precedenti di DMX Total Control non li applicano e, quando salvano lo show, li tolgono, come le impostazioni dell'arresto dello show.

> **Non provato con apparecchi reali:** il parcheggio non è stato provato con interfacce, nodi né hazer reali.

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

- **«Patch di un altro show…»**: le sue fixture. Vanno agli stessi indirizzi se sono liberi; altrimenti, ai successivi. Portano la loro protezione da effetto speciale, i loro tempi, il loro canale di sicurezza e la loro icona della pianta (ciò che dice l'altro show non toglie mai la protezione a un CO2), ma non i loro canali parcheggiati.
- **«Scene di un altro show…»**: le sue scene, assegnate alle fixture di questo show che coincidono (per indirizzo e modello, per nome o per modello). Entrano in gruppi con lo stesso nome, con le loro opzioni nel gruppo (dimensione, fase e velocità live, rilascio, flash, dimmer…). Un gruppo nuovo porta anche il suo crossfade e il suo master; uno che esiste già in questo show mantiene i suoi.
- **«Patch e scene di un altro show…»**: entrambe le cose.
- **«Patch da un programma di visualizzazione (CSV)…»**: la tabella di fixture che esporta il tuo programma di visualizzazione; vedi [Importare un patch da un programma di visualizzazione](patch.md#importare-un-patch-da-un-programma-di-visualizzazione).

## Importare un patch da un programma di visualizzazione

Se progetti l’impianto in un programma di visualizzazione, esporta la sua tabella di fixture (CSV) e importala: DMX Total Control patcha le stesse fixture, negli stessi universi e indirizzi, con i loro nomi, i loro numeri e la loro posizione sulla pianta.

1. Premi **«Importa» › «Patch da un programma di visualizzazione (CSV)…»** e scegli il file.
2. La finestra **«Importa un patch»** indica quante fixture ha la tabella, di quanti tipi e in quali universi. Non cambia nulla finché non premi **«Importa»**.
3. In **«Tipi di fixture»**, ogni tipo della tabella (fixture, modalità e canali) indica cosa diventa e come è stato trovato nella libreria: **«Esatto»** (lo stesso modello e la stessa modalità), **«Per numero di canali»** (la modalità con quei canali), **«Nome simile»**, **«Ricordato»** o **«Scelto a mano»**. Un **«Nome simile»** appare in arancione perché tu lo controlli: è un modello con un nome simile, non lo stesso. Se non è nella libreria, viene patchato come **«Dimmer generico»** con i suoi canali, in arancione: tutto resta comunque ai suoi indirizzi. Ogni tipo mostra anche l'icona che avranno le sue fixture sulla pianta; se viene patchato come dimmer generico, prende quella che dice il suo nome nella tabella (un «CO2 Jet» appare come getto di CO2; un «Flame», come fiamma).
4. **«Cambia»** sceglie un’altra fixture e modalità (**«Scegli dalla libreria…»**), un dimmer generico o **«Non importare»**. Quello che scegli a mano viene ricordato per la prossima volta che importi quel tipo.
5. In **«Opzioni»**:
   - **«Posizionarle sulla pianta del palco»**, con la **Vista**: **«Dall’alto»** (il pubblico in fondo alla pianta) o **«Frontale»** (come il pubblico vede l’impianto: le fixture più alte in cima). La vista frontale viene proposta quando l’impianto è appeso ad altezze diverse, e l’ultima che hai usato viene ricordata. La pianta si allarga per contenere l’impianto (non si restringe mai). Ogni importazione che le posiziona centra di nuovo l’impianto sulla pianta, anche dopo aver cambiato i suoi lati con «−» e «+».
   - **Gruppi**: uno per ogni tipo di fixture, uno per ogni livello (se la tabella ne ha più di uno) e quelli della colonna «Groups» della tabella.
6. Sotto i tipi ci sono i problemi, prima di importare: indirizzi già usati da un’altra fixture o da un’altra riga (quelle righe non vengono importate), righe senza universo o indirizzo, universi oltre il 128, indirizzi oltre il 512, un secondo DMX break (non importato), numeri di fixture che ha già un’altra fixture o troppo alti (ne ricevono un altro), un pan o tilt invertito che non è né sì né no, e gli universi aggiunti allo show.
7. Premi **«Importa N fixture»**. Un messaggio indica quante sono nuove e quante sono state aggiornate, e se resta qualche problema.

**Da ogni riga** vengono importati la fixture e la sua modalità, l’universo e l’indirizzo, il nome (la colonna «Unit») e il numero di fixture: quello della colonna «Channel», altrimenti quello dell’altra colonna del numero di fixture, altrimenti uno automatico. L’altezza di ogni testa mobile, se è in piedi (capovolta nella tabella) o appesa, e il suo pan o tilt invertito vanno alla sua [altezza e orientamento](patch.md#altezza-e-orientamento-follow), che tu le posizioni sulla pianta o no.

**Importare di nuovo** la tabella (dopo aver cambiato l’impianto nel programma di visualizzazione) aggiorna le stesse fixture: le riconosce dall’identificatore di ogni riga della tabella, o dallo stesso nome e fixture, e ne cambia la modalità, l’indirizzo, il nome, il numero e la posizione sulla pianta. Quelle che non sono più nella tabella restano, a meno che tu non spunti **«Rimuovi le N fixture importate prima che non sono più nella tabella»** (sotto si vedono i loro nomi). Un'icona che scegli a mano dopo viene mantenuta, a meno che non cambi la fixture con cui viene patchato il suo tipo.

> **Limiti:** di ogni fixture si importa solo il primo DMX break, e delle rotazioni si usa solo se la testa è capovolta (l’orientamento non viene importato). Viste dall’alto, le fixture appese una sopra l’altra finiscono insieme sulla pianta: per vederle separate, usa la vista frontale.

## Eliminare fixture

Selezionale e premi **«Elimina»**. Viene chiesto prima: i loro valori vengono rimossi da tutte le scene.

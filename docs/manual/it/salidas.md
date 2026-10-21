# Uscite

![La pagina Uscite](../img/salidas.png)

**Uscite** (F7) collega ogni **universo** dello show (512 canali DMX) a un'**interfaccia**: un'interfaccia DMX USB, un nodo Art-Net o un'uscita sACN. Contiene anche il monitor dei [livelli DMX](salidas.md#livelli-dmx), che mostra dal vivo ciò che esce.

Nell'intestazione:

- **«Universi»** / **«Livelli DMX»**: le due viste della pagina.
- **«Cerca di nuovo»**: cerca di nuovo le interfacce collegate.
- **«Rimuovi universo»** e **«Aggiungi universo»**.

## Collegare un universo

1. Collega l'interfaccia al computer (o alla rete, per Art-Net e sACN).
2. Se non compare in **«Uscite rilevate»**, premi **«Cerca di nuovo»**.
3. Nella riga dell'universo, apri il menu a tendina (indica «Non collegato») e scegli l'uscita.

Ogni uscita compare con il nome del suo tipo e quello dell'interfaccia o della rete: «DMX USB — …», «ArtNet — …», «E1.31 — …» (E1.31 è sACN). L'universo collegato viene segnato in verde. Per scollegarlo, scegli «Non collegato».

Il collegamento viene ricordato per la prossima volta che apri il programma.

> **Consiglio:** le interfacce DMX USB basate su chip FTDI hanno bisogno dei loro driver. Se non li hai, l'installer del programma offre alla fine la pagina per scaricarli.

## Universi

- **«Aggiungi universo»** ne aggiunge uno alla fine. Quando patchi scegli in quale universo va ogni fixture.
- Fai clic sul **nome** di un universo per cambiarlo (il nome si salva con lo show).
- **«Rimuovi universo»** rimuove l'**ultimo** (ne resta sempre uno). Se ha fixture patchate, non si può: spostale in un altro universo o eliminale prima nel Patch. Se è l'ingresso di un controller, viene chiesto prima, perché quel controller smetterà di funzionare finché non lo riattivi in Mappature.

## Livelli DMX

**«Livelli DMX»** mostra i 512 canali di un universo così come escono dal programma, con tutto applicato (scene, programmatore, mixer, grand master, limiti…):

- Scegli l'**Universo** nel menu a tendina.
- Ogni casella ha il numero del canale e il suo valore (0–255). **«Mostra in %»** passa alla percentuale.
- Le caselle con il bordo appartengono a una fixture. Passa il mouse su un canale per vedere di quale fixture è («Nome — canale N»).

> **Consiglio:** se una fixture non risponde, guarda qui se il suo canale si muove. Se si muove, il problema è nel cavo, nell'indirizzo dell'apparecchio o nell'interfaccia; se no, nel patch o in ciò che stai avviando.

Lo stesso monitor può essere una sezione dello [SHOW](show.md) («Livelli DMX»).

## Ingresso DMX

Un'interfaccia con ingresso DMX (o una console collegata a essa) serve per controllare il programma: si attiva come dispositivo in [Mappature](mapeos.md#dispositivi).

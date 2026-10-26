# Mappages

![La page Mappages](../img/mapeos.png)

**Mappages** (F8) sert à piloter le show depuis l’extérieur : contrôleurs **MIDI**, **OSC** depuis une autre application, une console par **entrée DMX** ou le **clavier**. Un mappage relie un contrôle de votre appareil (un pad, un fader, une touche…) à un élément du programme (une scène, un master, un GO…). Les mappages sont enregistrés avec le show.

La page a trois onglets : **« Mappages »**, **« Appareils »** et **« Moniteur »**, et le bouton **« Apprendre »**.

## Appareils

Dans **« Appareils »** apparaissent les appareils d’entrée que le programme trouve, avec leur type (MIDI, OSC ou DMX), leur nom et l’univers qu’ils utilisent. Les contrôleurs (MIDI et OSC) et les appareils que vous utilisez déjà viennent d’abord, sous **« Contrôleurs et appareils utilisés »** ; ensuite, sous **« Entrées DMX : interfaces et réseau »**, les autres entrées DMX, avec une ligne par carte réseau.

1. Branchez le contrôleur et, s’il n’apparaît pas, appuyez sur **« Rechercher à nouveau »**.
2. Activez son **« Entrée »**.
3. Si l’appareil a une sortie, activez **« Feedback (LED) »** pour que le programme allume ses LED ou bouge ses faders motorisés.

Le mappage d’un appareil non connecté est conservé : il fonctionne à nouveau quand vous le connectez.

## Mode Mapper

Le moyen le plus rapide de créer des mappages est le **mode Mapper** (« Apprendre ») :

1. Activez l’entrée de votre appareil dans « Appareils ».
2. Appuyez sur la **cible** de la barre du haut (ou sur **« Apprendre »** dans cette page). Les contrôles qui peuvent être mappés sont marqués en **violet** ; un nombre dans le coin indique combien de mappages ils ont déjà.
3. Cliquez sur un contrôle du programme : une scène de Direct, un master, un bouton du mixeur, un playback…
4. La barre violette indique « Bougez un contrôle de votre appareil ou appuyez sur une touche pour : … ». Bougez le fader, appuyez sur le pad ou sur une touche.
5. Le programme confirme « Mappé : … ». Passez au contrôle suivant.
6. Appuyez sur **« Terminé »** (ou Échap) pour sortir.

En mode Mapper, **Maj+clic** utilise le contrôle au lieu de le mapper. **Échap** annule le contrôle en attente ; un autre Échap quitte le mode. Le bouton « Mappages » de la barre violette vous amène sur cette page.

Un contrôle du programme peut avoir plusieurs mappages (par exemple, un pad d’un contrôleur et une touche).

> **Conseil :** quand vous apprenez un bouton d’une Akai APC, le mappage arrive déjà avec les LED en vert pendant que l’élément joue.

### Ce qui peut être mappé

- **Barre du haut** : GO, Stop, BeatGO, grand master, Blackout et Tout arrêter.
- **Direct** : chaque scène, ◀ ▶, Pause et Libérer les groupes de l’en-tête, et dans chaque groupe sa libération, précédent, suivant et master.
- **Mixeur** : sur chaque bande, le dimmer, la teinte, la saturation, le stroboscope et sa vitesse, et les boutons Full, Blackout, Flash, Flash excl. et Full excl.
- **Programmer** : les palettes.
- **Playbacks** : les noms de page, ◀ ▶ de page, « Tout libérer », « Retour », le grand GO, et pour chaque playback sa ×, son nom (sélectionner), ses trois boutons et son fader.
- **SHOW** : les boutons et les faders (le fader entier, avec tous ses éléments), et ◀ ▶ des sections de playbacks.

## La liste des mappages

Dans **« Mappages »**, chaque ligne affiche le contrôle de l’appareil, ce qu’il pilote et son comportement. Un point violet indique un bouton et un point bleu un fader. Cliquez sur un mappage pour le modifier à droite, ou appuyez sur **« Ajouter »** pour en créer un vide.

## Modifier un mappage

### Contrôle

- **Le type** : **MIDI**, **OSC**, **« Entrée DMX »** ou **« Clavier »**.
- **Appareil** : un appareil précis ou **« Tout appareil »**.
- **MIDI** : **CC** ou **Note**, son numéro (0–127) et le **Canal** MIDI (1–16).
- **OSC** : le chemin, par exemple `/scene/1`.
- **Entrée DMX** : le **Canal** (1–512).
- **Clavier** : la touche, par exemple `Q` ou `F9`.
- **« Apprendre le contrôle »** : appuyez dessus puis bougez le contrôle de l’appareil pour remplir tout ce qui précède.

### Pilote

Ce que fait le mappage. Les **boutons** :

| Destination | Ce qu’il fait |
|---|---|
| Lancer une scène | La scène choisie, dans son groupe. |
| Scène N d’un groupe | La scène qui occupe cette place dans le groupe (**Numéro de scène**). |
| Libérer un groupe / Scène suivante d’un groupe / Scène précédente d’un groupe | Dans un groupe ou dans « Tous les groupes ». |
| Blackout, Tout arrêter, Tout mettre en pause, Tap tempo, BeatGO, GO, Stop | Comme leurs boutons (GO et Stop choisissent le mode, comme dans la barre du haut : l’un des deux est toujours allumé). |
| Palette | La palette sur ses propres fixtures ou sur un groupe de fixtures. |
| Mixeur : full, blackout, flash, flash exclusif, full exclusif, stroboscope | Sur la bande choisie du mixeur. |
| Playback : GO, retour, pause / retour, flash, libérer, sélectionner | Sur un playback. |
| Playback : aller à une cue | Saute à la **Cue** indiquée (1 est la première de la liste). |
| GO du playback sélectionné / Retour du playback sélectionné | Comme le grand GO et « Retour » de Playbacks. |
| Page de playbacks N / suivante / précédente | Change la page de playbacks. |
| Libérer tous les playbacks | Comme « Tout libérer ». |

Les **faders** :

| Destination | Ce qu’il bouge |
|---|---|
| Grand master | Le GM. |
| Master d’un groupe | Le master de ce groupe. |
| Dimmer en direct d’une scène / Vitesse en direct d’une scène | Ses curseurs en direct. |
| BPM | Le tempo. |
| Mixeur : dimmer, teinte, saturation, vitesse du stroboscope | Sur la bande choisie. |
| Fader du SHOW | Un fader du SHOW entier. |
| Playback : fader | Le fader d’un playback. |

Pour les playbacks, vous choisissez la **page** (« Page actuelle (suit) », qui suit la page affichée, ou une page fixe) et le numéro de **Playback**. Avec **« Ou une cuelist ou une scène, où qu’elle soit »**, le mappage agit sur cette cuelist ou cette scène sur n’importe quel playback où elle se trouve. Dans « aller à une cue », vous voyez à quelle cue il mène maintenant (« Maintenant : cue 3 de Chanson 1 »).

### Comportement

Pour les **boutons** :

- **« Bascule : chaque appui on / off »** : chaque appui allume ou éteint. Sur une scène, il s’appelle **« Appuyer : selon GO / Stop »** : il la lance ou l’arrête selon GO et Stop de la barre du haut.
- **« Flash : tant qu’on appuie »**.
- **« Allumer seulement »** / **« Éteindre seulement »**.

Les boutons de playback agissent à l’appui, sauf le flash, et n’ont pas cette option.

Pour les **faders** :

- **« Absolu : suit le fader »**.
- **« Incrémental : un pas par appui »** (pour des boutons qui montent ou descendent).
- **« Relatif : encodeur sans fin »** et **« Relatif : encodeur, 1 monte et 127 descend »** (encodeurs sans fin).
- **« De »** … **« à »** : la plage, de 0 à 100 %. **« Inverser »** la retourne.
- **« Pas % »** et **« En boucle »** (incrémental et relatif).
- **« Reprendre le fader (sans moteur) »** (absolu) : le fader du contrôleur ne pilote le niveau qu’après l’avoir rejoint, sans sauts.

### Feedback

En MIDI ou OSC, **« Feedback »** fait que le programme **allume le bouton de l’appareil** ou **bouge son fader motorisé**. Seul ce qui change est envoyé.

Pour les LED des boutons MIDI :

1. Dans **« LED de »**, choisissez **« Tout contrôleur (numéros) »** ou votre contrôleur : **Akai APC mini**, **Akai APC mini mk2** ou **Akai APC40 mkII**.
2. Pour **« Activé »**, **« Désactivé »** et **« En pause »**, choisissez la couleur dans les échantillons (sur une APC) ou tapez la valeur (0–127) pour tout autre contrôleur.
3. Sur les pads RGB de l’APC mini mk2, choisissez aussi comment ils s’allument : **« Atténué »**, **« Plein »**, **« Pulsé »** ou **« Clignotant »** (sur l’APC40 mkII, **« Fixe »**). Sans rien choisir, « Activé » s’allume en **« Plein »** et « Désactivé » en **« Atténué »**.

Les LED du contrôleur s’éteignent quand vous fermez le programme, quand vous effacez un mappage ou le déplacez vers un autre contrôle, quand vous ouvrez un autre show ou quand vous désactivez l’appareil ou son feedback.

### Dupliquer et supprimer

**« Dupliquer »** crée une copie du mappage (utile pour un autre appareil ou un autre canal) et **« Supprimer »** l’efface.

## Clavier

Les mappages de clavier fonctionnent sur n’importe quelle page et ont la priorité sur les raccourcis du programme qui utilisent la même touche. Ils ne se déclenchent pas pendant que vous tapez dans un champ de texte (dans Programmer, tant que la ligne de commande a le clavier) ni avec Ctrl ou Alt enfoncé. Les boutons du SHOW ont en plus leur propre touche, qui ne fonctionne que sur la page Show (voir [Show](show.md#options-dun-bouton)).

## Modèles pour Akai APC

**« Tout mapper »** assigne d’un coup un contrôleur entier. Ce bouton se trouve sur chaque appareil MIDI de l’onglet « Appareils » :

1. Appuyez sur **« Tout mapper »** sur votre contrôleur.
2. Choisissez le modèle :
   - **« Akai APC mini → Groupes »** / **« Akai APC mini mk2 → Groupes »**
   - **« Akai APC mini → Playbacks »** / **« Akai APC mini mk2 → Playbacks »**
   - **« Akai APC40 mkII → Playbacks »**
3. Lisez ce que fait chaque contrôle et appuyez sur **« Mapper »**. Les mappages que cet appareil avait sont remplacés ; si son entrée était désactivée, elle est activée.

**→ Groupes** (APC mini et mk2) : chaque colonne de pads lance les scènes d’un groupe (vert : en cours ; jaune : prête ; éteint : pas de scène). Les faders sont les masters des groupes et le dernier le grand master. Les boutons sous les pads libèrent les groupes ; ceux de droite sont blackout, tout arrêter, pause, tap, BeatGO, précédent, suivant et flash.

**→ Playbacks** (APC mini et mk2) : chaque colonne est un playback 1-8 de la page de playbacks actuelle : son fader (il reprend le niveau) et, depuis le bas, GO, Pause/Retour, Flash et ses 5 premières cues (la cue 1 en haut). Le bouton sous les pads le libère. Le dernier fader est le grand master ; les boutons de droite sont page précédente et suivante, GO et Retour du playback sélectionné, blackout, tap, BeatGO et tout arrêter.

**→ Playbacks** (APC40 mkII) : chaque piste est un playback 1-8 de la page actuelle : son fader, GO sur le bouton de piste, Pause/Retour sur S, Flash sur l’armement d’enregistrement, sélectionner et libérer, et ses 5 premières cues sur les clips. Les boutons de scène vont aux pages 1-5, les boutons de banque changent la page et le groupe, et Play et Record sont GO et Retour du playback sélectionné. Les potentiomètres device sont les faders des playbacks 9-16 et les potentiomètres de piste, les masters des premiers groupes.

> **Conseil :** après « Tout mapper », chaque contrôle apparaît comme un mappage normal dans la liste : vous pouvez changer n’importe lequel, ou l’effacer, sans toucher aux autres.

## Moniteur

**« Moniteur »** affiche les **messages entrants et sortants** : l’heure, **IN** ou **OUT**, l’appareil (ou « Clavier »), le message et sa valeur. Bougez un contrôle d’un appareil activé pour voir ce qu’il envoie. **« Effacer »** vide la liste.

Le voyant vert de la cible, dans la barre du haut, clignote à chaque message reçu.

## Alt+clic

Hors du mode Mapper, **Alt+clic** sur un contrôle mappable l’envoie au SHOW : voir [Show](show.md#envoyer-nimporte-quel-contrôle-au-show).
